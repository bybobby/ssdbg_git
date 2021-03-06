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
// File           : mtdqueue.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : definition of thread for data queueing
// Author         : David Lafreniere (a member of www.codeproject.com)
// Modifier       : noah                                            
//                                                                   
// Update History                                                    
//        2017-02-07 (V0.1)   : Last Update by David
//        2018-05-28 (V0.2)   : first modification by noah            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __MT_DEQUEUE_H__
#define __MT_DEQUEUE_H__

#include <thread>
#include <queue>
#include <mutex>
#include <atomic>
#include <condition_variable>

struct UserData
{
  UserData(uint8_t c, uint64_t i, void* b) { control = c, id = i, buf_pt = b; }
  uint8_t control;
  uint64_t  id;
  void* buf_pt;
};

struct ThreadMsg;

class mtdqueue 
{
public:
	/// Constructor
	mtdqueue(const char* threadName);

	/// Destructor
	~mtdqueue();

	/// Called once to create the worker thread
	/// @return TRUE if thread is created. FALSE otherwise. 
	bool CreateThread();

	/// Called once a program exit to exit the worker thread
	void ExitThread();

	/// Get the ID of this thread instance
	/// @return The worker thread ID
	std::thread::id GetThreadId();

	/// Get the ID of the currently executing thread
	/// @return The current thread ID
	static std::thread::id GetCurrentThreadId();

	/// Add a message to thread queue. 
	/// @param[in] data - thread specific information created on the heap using operator new.
	void PostMsg(const UserData* data);
  
private:
	mtdqueue(const mtdqueue&);
	mtdqueue& operator=(const mtdqueue&);

	/// Entry point for the worker thread
	void Process();

	/// Entry point for timer thread
	void TimerThread();

	std::thread* m_thread;
	std::queue<ThreadMsg*> m_queue;
	std::mutex m_mutex;
	std::condition_variable m_cv;
	std::atomic<bool> m_timerExit;
	const char* THREAD_NAME;
  bool m_stopThread;
};

#endif 

