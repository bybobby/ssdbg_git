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
// File           : common.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : definition for common macro
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////


#ifndef __COMMON_H__
#define __COMMON_H__

// #define QT_STATIC 1
// #define MS_NO_COREDLL 1
// #define Py_NO_ENABLE_SHARED 1

#define NUM_OF_REAREG       14
#define SPI_REP_NUM         64
#define SPI_NUM_OF_RDATA    SPI_REP_NUM
#define SPI_RDATA_LENGTH    SPI_NUM_OF_RDATA*NUM_OF_REAREG
#define SPI_QUEUE_LEGNTH    SPI_RDATA_LENGTH/4
#define SPI_QUEUE_BUFF_LEN  SPI_QUEUE_LEGNTH * 8
#define DUMP_NUM            SPI_REP_NUM/4
#define GRAPH_DATA_LENGTH   SPI_RDATA_LENGTH*2

#ifdef FTDI_INTERFACE
  #define try_conn          ftdi_try_conn
  #define try_discon        ftdi_try_discon
  #define brd_byte_rd       spi_rd_byte
  #define brd_byte_wr       spi_wr_byte
#else
  #define try_conn          uart_try_conn
  #define try_discon        uart_try_discon
  #define brd_byte_rd       uart_byte_rd
  #define brd_byte_wr       uart_byte_wr
#endif

#endif