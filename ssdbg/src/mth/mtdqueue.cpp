
/////////////////////////////////////////////////////////////////////
//                                                                   
//   r5MgMP:  .ubRgZU        :YPMQQu    rri7jY  :7iriiirir7i         
// 7BBBBBBBU BBBBBBBB      sBBBBBBBQ   BBBBBB.  gBBBBBBBBBBQ         
// BBBBBBBB27BBBBBBBB     ZBBBBBBBBB   BBBBB    jBBBQBBBBBBP         
// BBBBBBBKi:BBBBBBdM    .BBBBBBBQBB   PBBBB    vBQBBBBBBMBJ         
//  QBBBQY   YBBBBB      7BBBBBD       .BBBB       rBBBBs            
//    DBBB:    vBBBD     .BBBBB      iBQBQBBj Br   :BBBQv            
//     BQBB     7BBBs     qBBBBX     BBB. BBBBB7   :BBBBr            
//  vBBBBBB   BQBBBB:      vBBBBBBBg BBBj MBBBi    .BBBQi            
//   BBBg7    PBBB5          iSBBBB7  sBBBP.uQb     MDEQ.            
//                                                                   
// Copyright 2018 - ShinSung C&T 
//                                                                   
// Licensed under the Apache License, Version 2.0 (the 'License');   
// you may not use this file except in compliance with the License.  
// You may obtain a copy of the License at                           
//                                                                   
//      http://www.apache.org/licenses/LICENSE-2.0                   
//                                                                   
// Unless required by applicable law or agreed to in writing,        
// software distributed under the License is distributed on          
// an 'AS IS' BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,   
// either express or implied. See the License for the specific       
// language governing permissions and limitations under the License. 
//                                                                   
/////////////////////////////////////////////////////////////////////
                                                                     
/////////////////////////////////////////////////////////////////////
//                                                                   
// File           : mtdqueue.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : declaration of thread for queue operation
// Author         : David Lafreniere (a member of www.codeproject.com)
// Modifier       : noah                                            
//                                                                   
// Update History                                                    
//        2017-02-07 (V0.1)   : Last Update by David
//        2018-05-28 (V0.2)   : first modification by noah            
//                                                                   
/////////////////////////////////////////////////////////////////////


#include "common.h"
#include "mth/mtdqueue.h"
#include "mth/rdyqueue.h"
#include <iostream>

using namespace std;

#define MSG_EXIT_THREAD			1
#define MSG_POST_USER_DATA		2
#define MSG_TIMER				3

struct ThreadMsg
{
	ThreadMsg(int i, const void* m) { id = i; msg = m; }
	int id;
	const void* msg;
};

//----------------------------------------------------------------------------
// mtdqueue
//----------------------------------------------------------------------------
mtdqueue::mtdqueue(const char* threadName) 
: m_thread(0), m_timerExit(false)
, THREAD_NAME(threadName)
, m_stopThread(true)
{
}

//----------------------------------------------------------------------------
// ~mtdqueue
//----------------------------------------------------------------------------
mtdqueue::~mtdqueue()
{
	ExitThread();
}

//----------------------------------------------------------------------------
// CreateThread
//----------------------------------------------------------------------------
bool mtdqueue::CreateThread()
{
  m_stopThread = false;
  exit_queue(1);

	if (!m_thread)
		m_thread = new thread(&mtdqueue::Process, this);
  
	return true;
}

//----------------------------------------------------------------------------
// GetThreadId
//----------------------------------------------------------------------------
std::thread::id mtdqueue::GetThreadId()
{
	return m_thread->get_id();
}

//----------------------------------------------------------------------------
// GetCurrentThreadId
//----------------------------------------------------------------------------
std::thread::id mtdqueue::GetCurrentThreadId()
{
	return this_thread::get_id();
}

//----------------------------------------------------------------------------
// ExitThread
//----------------------------------------------------------------------------
void mtdqueue::ExitThread()
{
	if (!m_thread)
		return;

  exit_queue(0);
  m_stopThread = true;

  UserData* exit_userdata = new UserData(1, 0, 0);
  push_rawd_queue(exit_userdata);
  
  set_lock_reorder_var();
  UserData* reorder_userdata = new UserData(1, 0, 0);
  push_reorder_queue(reorder_userdata);
  
	std::unique_lock<std::mutex> lk(m_mutex);

	m_thread->join();
	delete m_thread;
	m_thread = 0;
}

//----------------------------------------------------------------------------
// Process
//----------------------------------------------------------------------------
void mtdqueue::Process()
{
  unsigned char data_reoder[SPI_QUEUE_LEGNTH*2];
  short sdata[6] = { 0 };
  
	while (!m_stopThread)
	{
    set_lock_rawd_var();
    UserData* udata = pop_rawd_queue();
    
    unsigned char* lvp_buf = (unsigned char*)udata->buf_pt;
    
    if (udata->control > 0) {
      return;
    }
    
    for (int i = 0; i < SPI_QUEUE_LEGNTH; i++) {
      unsigned char rddat = 0;
      
      for (int k = 0; k < 8; k++) {
        rddat |= (((lvp_buf[i * 8 + k] & 0x04) >> 2) << (7 - k));
      }
      
      data_reoder[((i&0xfffffffe) + ((i+1) % 2))] = rddat;
    }
    
    delete[] lvp_buf;
    
    int seq_m = 0;
    short* sbuf = new short[DUMP_NUM*6];
    
    // DTRACE("data_reorder : %08x\n", *((short*)data_reoder));

    // for (int m=0; m<SPI_QUEUE_LEGNTH; m+=NUM_OF_READEG) {
    //   memcpy(&sbuf[seq_m], (short *)(data_reoder     + m), sizeof(short) * 3); seq_m += 3;
    //   memcpy(&sbuf[seq_m], (short *)(data_reoder + 8 + m), sizeof(short) * 3); seq_m += 3;
    // }
    // set_lock_reorder_var();
    // UserData* reorder_userdata = new UserData(0, get_reorder_id(false), sbuf);
    // push_reorder_queue(reorder_userdata);
      
    delete udata;
	}
}


