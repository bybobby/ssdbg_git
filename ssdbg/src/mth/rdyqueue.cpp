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
// File           : rdyqueue.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : declaration of prepare for using ftdi queue
// Author         : noah                                            
//                                                                   
// Update History                                                    
//        2018-05-28 (V0.1)   : Initial Creation
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "mth/rdyqueue.h"
#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

std::queue<UserData*> lv_rawd_queue;
std::condition_variable lv_rawd_cv;
std::mutex lv_rawd_mutex;
  
std::queue<UserData*> lv_reorder_queue;
std::condition_variable lv_reorder_cv;
std::mutex lv_reorder_mutex;

std::queue<UserData*> lv_file_queue;
std::condition_variable lv_file_cv;
std::mutex lv_file_mutex;

void clear(std::queue<UserData*> &q)
{
  std::queue<UserData*> empty;
  std::swap(q, empty);
}

void initialize ()
{
  // get_mtdqueue_handle(1, 0);
  get_mtfile_handle(1);
  exit_queue(1);
  // get_reorder_id(true);
  get_file_id(true);
  // get_dumpf_stream(0, 0);
  flush_queue();
}

void start_thread()
{  
  get_file_id(true);
  // get_reorder_id(true);
  get_mtfile_handle(0);
  // get_mtdqueue_handle(0, 0);
  // this_thread::sleep_for(milliseconds(200));
  // this_thread::sleep_for(milliseconds(200));
  // this_thread::sleep_for(milliseconds(200));
}

void stop_thread()
{
  exit_mtfile_thread();
  // exit_mtdqueue_thread();
}

void set_lock_rawd_var()
{
  static std::mutex lv_mutex;

  std::lock_guard<std::mutex> guard(lv_mutex);
}

void push_rawd_queue(UserData* data)
{
  std::unique_lock<std::mutex> lk(lv_rawd_mutex);
  lv_rawd_queue.push(data);
  lv_rawd_cv.notify_one();
}

bool exit_queue(int getset)
{
  static bool exitvar = false;

  if (getset == 0)
    exitvar = true;
  else if (getset == 1)
    exitvar = false;
    
  return exitvar;
}

UserData* pop_rawd_queue()
{
  UserData* udata = NULL;
  
  std::unique_lock<std::mutex> lk(lv_rawd_mutex);
  
  if (exit_queue(3))
    return NULL;
    
  while (lv_rawd_queue.empty())
    lv_rawd_cv.wait(lk);
  
  if (!lv_rawd_queue.empty()){
    udata = lv_rawd_queue.front();
    lv_rawd_queue.pop();
  }
  
  return udata;
}

void flush_queue()
{
  UserData* udata = NULL;
  
  while (!lv_file_queue.empty())
  {
    udata = lv_file_queue.front();
    lv_file_queue.pop();
    if (udata->buf_pt != NULL){
      delete udata->buf_pt;
      udata->buf_pt = NULL;
    }
    delete udata;
  }

  while (!lv_reorder_queue.empty())
  {
    udata = lv_reorder_queue.front();
    lv_reorder_queue.pop();
    if (udata->buf_pt != NULL){
      delete udata->buf_pt;
      udata->buf_pt = NULL;
    }
    delete udata;
  }

  while (!lv_rawd_queue.empty())
  {
    udata = lv_rawd_queue.front();
    lv_rawd_queue.pop();
    if (udata->buf_pt != NULL){
      delete udata->buf_pt;
      udata->buf_pt = NULL;
    }
    delete udata;
  }
  
}

void push_reorder_queue(UserData* data)
{
  UserData* udata = NULL;
  std::unique_lock<std::mutex> lk(lv_reorder_mutex);
  if (!lv_reorder_queue.empty() && lv_reorder_queue.size() > 100) {
    udata = lv_reorder_queue.front();
    lv_reorder_queue.pop();
    delete[] udata->buf_pt;
  }
	lv_reorder_queue.push(data);
	lv_reorder_cv.notify_one();
}

UserData* pop_reorder_queue()
{
  UserData* udata = NULL;
  
  std::unique_lock<std::mutex> lk(lv_reorder_mutex);
  
  if (!lv_reorder_queue.empty()){
    udata = lv_reorder_queue.front();
    lv_reorder_queue.pop();
  }
  
  return udata;
}

void set_lock_reorder_var()
{
  static std::mutex lv_mutex;
  
  std::lock_guard<std::mutex> guard(lv_mutex);
}

uint64_t get_reorder_id(bool reset)
{
  static uint64_t id = 0;
  
  if (reset)
    id = 0;
  else
    id++;
  
  return id;
}


void push_file_queue(UserData* data)
{
  std::unique_lock<std::mutex> lk(lv_file_mutex);
	lv_file_queue.push(data);
	lv_file_cv.notify_one();
}

UserData* pop_file_queue()
{
  UserData* udata = NULL;
  
  std::unique_lock<std::mutex> lk(lv_file_mutex);
  
  while (lv_file_queue.empty())
    lv_file_cv.wait(lk);
  
  if (!lv_file_queue.empty()){
    udata = lv_file_queue.front();
    lv_file_queue.pop();
  }
  
  return udata;
}

void set_lock_file_var()
{
  static std::mutex lv_mutex;
  
  std::lock_guard<std::mutex> guard(lv_mutex);
}

ofstream* get_dumpf_stream(uint8_t getnew, const char* filename)
{
  static ofstream* fs_dump = NULL;
  
  if (getnew > 1){
    fs_dump = new ofstream(filename, ofstream::out);
  }
  else if (getnew == 0){
    if (fs_dump != NULL){
      delete fs_dump;
      fs_dump = NULL;
    }
  }
  
  return fs_dump;
}

uint64_t get_file_id(bool reset)
{
  static uint64_t id = 0;
  
  if (reset)
    id = 0;
  else
    id++;
  
  return id;
}


