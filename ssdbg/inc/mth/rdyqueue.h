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
// File           : rdyqueue.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : definition of prepare for using ftdi queue
// Author         : noah                                            
//                                                                   
// Update History                                                    
//        2018-05-28 (V0.1)   : Initial Creation
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __RDYQUEUE_H__
#define __RDYQUEUE_H__

#include "mthandle.h"
#include "mtdqueue.h"
#include <iostream>
#include <fstream>

// #define NORMAL_READ_WRITE 1

// void gen_rdbuf_seq();
void start_thread();
void stop_thread();
bool exit_queue(int getset);

void set_lock_rawd_var();
void push_rawd_queue(UserData* data);
UserData* pop_rawd_queue();
void flush_queue();

void push_reorder_queue(UserData* data);
UserData* pop_reorder_queue();
void set_lock_reorder_var();
uint64_t get_reorder_id(bool reset);

void push_file_queue(UserData* data);
UserData* pop_file_queue();
void set_lock_file_var();
uint64_t get_file_id(bool reset);

std::ofstream* get_dumpf_stream(uint8_t getnew, const char* filename);

#endif

