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
// File           : mthandle.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : declaration of functions for multi thread handle
// Author         : noah
//                                                                   
// Update History                                                    
//        2018-05-28 (V0.1)   : Initial creation
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "mth/mtdqueue.h"
#include "mth/mtfile.h"
#include "mth/mthandle.h"
#define NUM_QUEUE_THREAD  1

mtdqueue* get_mtdqueue_handle(uint8_t mod, uint8_t num)
{
  static mtdqueue* lp_mtdqueue[NUM_QUEUE_THREAD] = { NULL };
  
  // if (num > (NUM_QUEUE_THREAD-1))
  //   return NULL;
  
  if (mod == 0) {
    for (int i=0; i<NUM_QUEUE_THREAD; i++) {
      lp_mtdqueue[i] = new mtdqueue("gathering thread");
      lp_mtdqueue[i]->CreateThread();
    }
  }
  else if (mod == 1) {
    for (int i=0; i<NUM_QUEUE_THREAD; i++) {
      if (lp_mtdqueue[i] != NULL){
        delete lp_mtdqueue[i];
      }
      lp_mtdqueue[i] = NULL;
    }
  }
  
  return lp_mtdqueue[num];
}

mtfile* get_mtfile_handle(uint8_t mod)
{
  static mtfile* lp_mtfile = NULL;
  
  if (mod == 0) {
    lp_mtfile = new mtfile("file thread");
    lp_mtfile->CreateThread();
  }
  else if (mod == 1) {
    if (lp_mtfile != NULL){
      delete lp_mtfile;
    }
    lp_mtfile = NULL;
  }
  
  return lp_mtfile;
}

void exit_mtdqueue_thread()
{
  for (int i=0; i<NUM_QUEUE_THREAD; i++){
    mtdqueue* lp_mtdqueue = get_mtdqueue_handle(3, i);
    if (lp_mtdqueue != NULL) {
      lp_mtdqueue->ExitThread();
      get_mtdqueue_handle(1, i);
    }
  }
}

void exit_mtfile_thread()
{
  mtfile* lp_mtfile = get_mtfile_handle(3);
  
  if (lp_mtfile != NULL) {
    lp_mtfile->ExitThread();
    get_mtfile_handle(1);
  }
}

void set_lock_thread()
{
  static std::mutex lv_rawd_mutex;
  
  std::lock_guard<std::mutex> guard(lv_rawd_mutex);
}



