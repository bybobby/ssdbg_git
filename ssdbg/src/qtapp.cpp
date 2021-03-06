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
// File           : qtapp.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : run qt application
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//        2018-04-25 (V0.2)   : separate entry point for win32 dll load
//                                                                   
/////////////////////////////////////////////////////////////////////



#include "common.h"
#include "childdlg/ssdbg.h"
#include "usb/evtflt.h"
#include "serial/uart_conn.h"

#include <cstdio>
#include <iostream>
#include <QtWidgets/QApplication>
#include <QGuiApplication>
#include <QStyleFactory>
#include <QFontDataBase>
#include <QFile>
#include <QDebug>

using namespace std;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QWinEventFilter evtFilter;
  
  std::cout << "check step 1 " << endl;

  // loadstylesheet
  QFile qFile(QStringLiteral(":/res/stylesheet/ssdbg.qss"));

  if (qFile.open(QIODevice::ReadOnly | QIODevice::Text))
  {
    // set stylesheet
    QString strStyle = QString::fromUtf8(qFile.readAll());
    app.setStyleSheet(strStyle);
    qFile.close();
  }

  std::cout << "check step 2 " << endl;
  QFontDatabase::addApplicationFont(":/res/fonts/Everson Mono Bold.ttf");
  QFont uFont("Everson Mono", 11, QFont::Normal);
  app.setFont(uFont);
  
  std::cout << "check step 3 " << endl;
  ssdbg m_main_dlg;
  // getmainWidget(&m_main_dlg);
  m_main_dlg.show();

  std::cout << "check step 4 " << endl;

  // windows event handlers
  QWindow* p_Wnd = qApp->allWindows().first();
  evtFilter.registerEventWindow(p_Wnd);
  evtFilter.registerMainWidget(&m_main_dlg);
  app.installNativeEventFilter(&evtFilter);
  auto result = app.exec();
  std::cout << "retult :  " << result << endl;

  return result;
}
