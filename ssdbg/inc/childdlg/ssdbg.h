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
// File           : ssdbg.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : shin-sung ic debugger main dialog header file
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __SSDBG_H__
#define __SSDBG_H__

#pragma once

#include <QtWidgets/QMainWindow>
#include <QToolButton>
#include "ui_ssdbg.h"

#define     BUTTON_NUM      10
#define     LABEL_NUM       10
#define     LEDIT_NUM       10

QT_FORWARD_DECLARE_CLASS(QTextEdit);
QT_FORWARD_DECLARE_CLASS(QPushButton);
QT_FORWARD_DECLARE_CLASS(QMouseEvent);
QT_FORWARD_DECLARE_CLASS(QComboBox);
QT_FORWARD_DECLARE_CLASS(QPixmap);
QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(QLineEdit);
QT_FORWARD_DECLARE_CLASS(QIcon);
QT_FORWARD_DECLARE_CLASS(dbgdlg);
QT_FORWARD_DECLARE_CLASS(gphdlg);
QT_FORWARD_DECLARE_CLASS(imgdlg);
QT_FORWARD_DECLARE_CLASS(tbldlg);
QT_FORWARD_DECLARE_CLASS(qsport);

class ssdbg : public QMainWindow
{
  Q_OBJECT

public:
  ssdbg(QWidget *parent = Q_NULLPTR);
  ~ssdbg();

  dbgdlg* m_pdbgdlg;
  gphdlg* m_pgphdlg;
  imgdlg* m_pimgdlg;
  tbldlg* m_ptbldlg;
  QPixmap* m_on_state;
  QPixmap* m_off_state;
  QLabel* m_connect;
  QLineEdit* m_qle[LEDIT_NUM];
  QComboBox* m_qComboBox;
  qsport* m_pqsport;

  bool m_conn_chk;
  void ftdi_try_conn();
  void ftdi_try_discon();
  bool uart_try_conn(bool set);
  void uart_try_discon();
  void set_conn_state(bool set);
  void intr_read(bool onoff);
  
private:
   Ui::ssdbgClass ui;
   bool m_mousePressed;
   QPoint m_mousePos;
   QPoint m_wndPos;
   QToolButton* m_minButton;
   QToolButton* m_quitButton;
   
   void styleWindow(bool bActive);

private slots:
  void OnAppStateChanged(Qt::ApplicationState state);
  void OnminimizeButtonClicked();
  void OncloseButtonClicked();
  void OnVersionCheck();

public slots:
  void btnclicked(int nID);
  
protected:
  void mouseMoveEvent(QMouseEvent* event);
	void mousePressEvent(QMouseEvent* event);
  bool eventFilter(QObject *obj, QEvent *evt) override;
};

#endif