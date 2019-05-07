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
// File           : dbgdlg.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : command debugger header file
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __DBGDBG_H__
#define __DBGDBG_H__

#pragma once

#include <QDialog>
#include "pyQt/PythonQt.h"
#include "pyQt/PythonQtScriptingConsole.h"

QT_FORWARD_DECLARE_CLASS(QTextEdit);
QT_FORWARD_DECLARE_CLASS(QListWidget);
QT_FORWARD_DECLARE_CLASS(QListWidgetItem);
QT_FORWARD_DECLARE_CLASS(QPushButton);
QT_FORWARD_DECLARE_CLASS(QFile);
QT_FORWARD_DECLARE_CLASS(QTextStream);

class dbgdlg : public QDialog
{
  Q_OBJECT

public :
  dbgdlg(QWidget *parent = Q_NULLPTR);
  
public :
  PythonQtObjectPtr m_mainContext;
  PythonQtScriptingConsole* m_pyQtConsole;
  QPushButton* m_qButton[3];
  void appendline(const QString& str);
  
private :
  QTextEdit* m_logconsole;
  QListWidget* m_qListWidget;
  QString m_sspath;
  QFile* m_fp;
  QTextStream* m_fp_out;
  bool m_logon;
  bool m_saveon;
  
private slots :
  void qListitemDoubleClicked(QListWidgetItem* pItem);
  void pyQtrunScript(const QString& filename);
  void log_onoff_btnclicked();
  void log_save_btnclicked();
};

#endif