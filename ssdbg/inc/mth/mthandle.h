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
// Description    : definition of functions for multi thread handle
// Author         : noah
//                                                                   
// Update History                                                    
//        2018-05-28 (V0.1)   : Initial creation
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __MTHREAD_HANDLE_H__
#define __MTHREAD_HANDLE_H__

#include <stdint.h>

class mtrd;
class mtwr;
class mtmsg;
class mtdqueue;
class mtfile;

mtrd* get_mtrd_handle(uint8_t mod);
mtwr* get_mtwr_handle(uint8_t mod);
mtmsg* get_mtmsg_handle(uint8_t mod);
mtdqueue* get_mtdqueue_handle(uint8_t mod, uint8_t num);
mtfile* get_mtfile_handle(uint8_t mod);

void exit_mtrd_thread();
void exit_mtwr_thread();
void exit_mtdqueue_thread();
void exit_mtfile_thread();

void set_lock_thread();

#endif


