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
// File           : dlghandle.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : declaration of child dialogs
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-05-20 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////
#include "childdlg/dlghandle.h"
#include "serial/uart_conn.h"
#include <QMutex>
#include <QThread>
#include <QDebug>

uint64_t time_increment(bool reset)
{
  static uint64_t currentTime = 0;
  const int interval = 1;
  
  if (reset) {
    currentTime = 0;
  }
  else {
    currentTime += interval;
  }
  
  return currentTime;
}

void set_lock_ssdbg(bool lock)
{
  static QMutex ssdbgMutex(QMutex::NonRecursive);
  static int lck = 0;
  static int ulck = 0;
  if (!lock) {
    ulck++;
    // qDebug() << "ssdbg Unlocked !!" << ulck << ",   " << lck;
    ssdbgMutex.unlock();
  }
  else {
    lck++;
    // qDebug() << "ssdbg locked !!" << ulck << ",   " << lck;
    while (!ssdbgMutex.tryLock(1));
  }
}

void set_lock_dbgdlg(bool lock)
{
  static QMutex dbgMutex(QMutex::NonRecursive);
  
  if (!lock) {
    // qDebug() << "dbgdlg Unlocked !!";
    dbgMutex.unlock();
  }
  else {
    // qDebug() << "dbgdlg locked !!";
    while(!dbgMutex.tryLock(1));
  }
}

void set_lock_imgdlg(bool lock)
{
  static QMutex imgMutex(QMutex::NonRecursive);

  if (!lock) {
    imgMutex.unlock();
  }
  else {
    while(!imgMutex.tryLock(1));
  }
}

void set_lock_gphdlg(bool lock)
{
  static QMutex gphMutex(QMutex::NonRecursive);
  static bool locking = false;
  static int lck = 0;
  static int ulck = 0;
  
  if (!lock) {
    // qDebug() << "gph unlock !!" << ++ulck << ",   " << lck;
    locking = false;
    gphMutex.unlock();
  }
  else {
    while (locking)
      delay(1);
    locking = true;
    // qDebug() << "gph locked !!" << ulck << ",   " << ++lck;
    while (!gphMutex.tryLock(1)) { delay(1); };
  }
}

void set_lock_tbldlg(bool lock)
{
  static QMutex tblMutex(QMutex::NonRecursive);
  
  if (!lock) {
    // qDebug() << "dbgdlg Unlocked !!";
    tblMutex.unlock();
  }
  else {
    // qDebug() << "dbgdlg locked !!";
    while(!tblMutex.tryLock(1));
  }
}
