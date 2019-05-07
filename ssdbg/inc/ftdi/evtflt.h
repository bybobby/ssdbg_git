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
// File           : evtflt.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : windows event filter
// Author         : noah                
// Reference      : Benjamin T (stackoverflow.com QnA)
//                  https://stackoverflow.com/questions/38528684/\
//                  detected-new-usb-device-connected-disconnected-on-qt/\
//                  38531390#38531390
//                                                                   
// Update History                                                    
//        2018-04-20 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////


#ifndef __EVTFLT_H__
#define __EVTFLT_H__

#pragma once

#include <QWindow>
#include <QAbstractNativeEventFilter>
#include "childdlg/ssdbg.h"

class QWinEventFilter : public QObject, public QAbstractNativeEventFilter
{
  Q_OBJECT
  
public :
  explicit QWinEventFilter(QObject *parent = 0);
  bool nativeEventFilter(const QByteArray &eventType, 
                            void *message, long *result) override;
                            
signals :
  void usbattached();
  void usbdeattached();
  
public slots :
  void registerEventWindow(QWindow *p_Wnd);
  void registerMainWidget(QWidget *p_Widget);

public :
  int32_t m_ftdi_acnt;
  int32_t m_ftdi_dcnt;
  
private :
  uint16_t m_dev_notify;
  QString m_guidstr[5];
  ssdbg* m_pMainDlg;
  
private slots :
  void try_connect();
  void try_disconnect();
};

#endif
