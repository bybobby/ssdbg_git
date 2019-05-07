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
// File           : mtfile.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : declaration of thread for read operation
// Author         : David Lafreniere (a member of www.codeproject.com)
// Modifier       : noah                                            
//                                                                   
// Update History                                                    
//        2017-02-07 (V0.1)   : Last Update by David
//        2018-05-28 (V0.2)   : first modification by noah            
//                                                                   
/////////////////////////////////////////////////////////////////////

#include <iostream>
#include <string>
#include <iomanip>
#include <chrono>
#include "mth/mtfile.h"
#include "mth/mtdqueue.h"
#include "mth/mthandle.h"
#include "mth/rdyqueue.h"
#include <sstream>
#include "common.h"
#include <QDebug>

using namespace std;

#define MSG_EXIT_THREAD			1
#define MSG_POST_USER_DATA		2
#define MSG_TIMER				3


//----------------------------------------------------------------------------
// mtfile
//----------------------------------------------------------------------------
mtfile::mtfile(const char* threadName) 
: m_thread(0), m_timerExit(false)
  , THREAD_NAME(threadName)
  , m_stopThread(true), m_pmtdqueue(NULL)
{
}

//----------------------------------------------------------------------------
// ~mtfile
//----------------------------------------------------------------------------
mtfile::~mtfile()
{
	ExitThread();
}

//----------------------------------------------------------------------------
// CreateThread
//----------------------------------------------------------------------------
bool mtfile::CreateThread()
{
  m_stopThread = false;
  
	if (!m_thread)
		m_thread = new thread(&mtfile::Process, this);
  
	return true;
}

//----------------------------------------------------------------------------
// GetThreadId
//----------------------------------------------------------------------------
std::thread::id mtfile::GetThreadId()
{
	return m_thread->get_id();
}

//----------------------------------------------------------------------------
// GetCurrentThreadId
//----------------------------------------------------------------------------
std::thread::id mtfile::GetCurrentThreadId()
{
	return this_thread::get_id();
}

//----------------------------------------------------------------------------
// ExitThread
//----------------------------------------------------------------------------
void mtfile::ExitThread()
{
	if (!m_thread)
		return;

  set_lock_file_var();
  UserData* exit_userdata = new UserData(1, 0, 0);
  push_file_queue(exit_userdata);
  
  m_stopThread = true;
  
	m_thread->join();
	delete m_thread;
	m_thread = 0;
}

//----------------------------------------------------------------------------
// Process
//----------------------------------------------------------------------------
void mtfile::Process()
{
  string str = "";
  const size_t MAX_STR_WIDTH = 10;
  chrono::high_resolution_clock::time_point start;
  chrono::high_resolution_clock::time_point stop;
  chrono::high_resolution_clock::time_point nowt;
  ofstream* lfs_dump = NULL;
  bool timetag = false;

	while (!m_stopThread)
	{

    set_lock_file_var();
    UserData* udata = pop_file_queue();
    
    uint64_t id = udata->id;
    short* lvp_buf = (short*)udata->buf_pt;
    uint16_t* lvp_ubuf = (uint16_t*)udata->buf_pt;
    
    lfs_dump = get_dumpf_stream(1, 0);

    if (udata->control > 0) {
      if (lfs_dump != NULL) {
        if (lfs_dump->is_open()) {
          stop = chrono::high_resolution_clock::now();
          chrono::duration<double> timecnt = chrono::duration_cast<chrono::duration<double>> (stop - start);
          str += "\nfinal time : " + to_string(timecnt.count()) + " sec";
          *lfs_dump << str;
          lfs_dump->close();
        }
      }
      return;
    }
    
    if (lfs_dump != NULL) {
      if (lfs_dump->is_open()) {
        ostringstream tmp;
        if (id == 1) {
          str += "start time : 0 sec";
          start = chrono::high_resolution_clock::now();
          timetag = true;
        }

        for (int i = 0; i < 12; i++) {
          if (!timetag)
            tmp << setw(30) << setfill(' ') << setiosflags(ios::right) << ' ' << ",   ";
          else
            tmp << setw(30-str.length()) << setfill(' ') << setiosflags(ios::right) << ' ' << ",   ";
          timetag = false;
          tmp << setw(MAX_STR_WIDTH) << setfill(' ') << setiosflags(ios::right) << (id-1) * 12 + i << ",   ";
          for (int j = 0; j < 3; j++) {
            if (!m_binary) {
              tmp << setw(MAX_STR_WIDTH) << setfill(' ') << setiosflags(ios::right) << lvp_buf[i * 3 + j] << ",   ";
            }
            else {
              // tmp << setw(4) << setfill('0') << setiosflags(ios::right) << std::hex << lvp_ubuf[i * 3 + j] << ",   ";
              tmp << setw(8) << setiosflags(ios::right) << std::dec << (int16_t)lvp_ubuf[i * 3 + j] << ",   ";
              tmp << std::dec;
            }
          }
          tmp << endl;
        }
        str += tmp.str();
        
        qDebug() << QString().fromStdString(str);

        if (str.length() > 1024) {
          *lfs_dump << str;
          str.clear();
          nowt = chrono::high_resolution_clock::now();
          chrono::duration<double> timecnt = chrono::duration_cast<chrono::duration<double>> (nowt - start);
          str += "end time : " + to_string(timecnt.count()) + " sec";
          timetag = true;
        }
      }
    }
    else {
      get_file_id(true);
    }
    delete[] lvp_buf;
	}
  
  if (lfs_dump != NULL) {
    if (lfs_dump->is_open()) {
      stop = chrono::high_resolution_clock::now();
      chrono::duration<double> timecnt = chrono::duration_cast<chrono::duration<double>> (stop - start);
      str += "\nfinal time : " + to_string(timecnt.count()) + " sec";
      *lfs_dump << str;
      lfs_dump->close();
    }
  }
}


