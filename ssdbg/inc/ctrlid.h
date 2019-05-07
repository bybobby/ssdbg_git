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
// File           : ctrlid.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : definition of ui control id
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////


// buntton  control id offset     0
// static   control id offset     256
// combobox control id offset     768
// editbox  control id offset     1024
// spin     control id offset     1280
// checkbox control id offset     1536
// misc     control id offset     1792

#define     MAIN_BTNID(A)       ((A)-32)
#define     MAIN_EDITID(A)      ((A)-1024)

#define     MAIN_BTN_LOAD       32
#define     MAIN_BTN_SAVE       33
#define     MAIN_BTN_WRITE      34
#define     MAIN_BTN_READ       35
#define     MAIN_BTN_WRITEALL   36
#define     MAIN_BTN_READALL    37

#define     MAIN_EDIT_WADDR     1024
#define     MAIN_EDIT_WDATA     1025
#define     MAIN_EDIT_RDATA     1026


#define     GPH_BTNID(A)        ((A)-2048)
#define     GPH_CBID(A)         ((A)-2816)

#define     GPH_BTN_GRAPH       2048
#define     GPH_BTN_DUMP        2049
#define     GPH_BTN_BIN         2050

#define     GPH_CBBOX_PTA       2816
#define     GPH_CBBOX_PTB       2817
#define     GPH_CBBOX_PTC       2818
#define     GPH_CBBOX_PTD       2819
#define     GPH_CBBOX_PTE       2820
#define     GPH_CBBOX_PTF       2821
#define     GPH_MOUSE_USAGE     2822

