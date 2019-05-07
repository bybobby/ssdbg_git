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
// File           : ftdiComObj.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : python object for ftdi chip communication
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __FTDI_COM_OBJ_H__
#define __FTDI_COM_OBJ_H__

#pragma once

#include "pyQt/PythonQt.h"
#include "pyQt/PythonQtCppWrapperFactory.h"
#include "childdlg/ssdbg.h"
#include "childdlg/tbldlg.h"
#include <qglobal.h>

QT_FORWARD_DECLARE_CLASS(QObject);
QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(ssdbg);

class ftdiComObj {
public :
  ftdiComObj(QWidget * pWidget);
  ~ftdiComObj();
  
  int ftdi_spi_rbyte(unsigned char adr, int len);
  bool ftdi_spi_wbyte(unsigned char adr, unsigned char wdat, int len);
  int ftdi_spi_rname(const QString& name);
  bool ftdi_spi_wname(const QString& name, int val);
  void ftdi_spi_queue_ctrl(bool onoff);
  bool ftdi_spi_queue_write();
  QString ftdi_spi_queue_read();
  
private :
  ssdbg* m_pMainWidget;
};

class ftdiComObjWrapper : public QObject
{
  Q_OBJECT 

public Q_SLOTS :
  // add a constructor
  ftdiComObj * new_ftdiComObj();
  
  // add a destructor
  void delete_ftdiComObj(ftdiComObj* obj);
  
  // add access methods
  int py_spi_rbyte(ftdiComObj* obj, unsigned char adr, int len);
  bool py_spi_wbyte(ftdiComObj* obj, unsigned char adr, unsigned char wdat, int len);
  int py_spi_rname(ftdiComObj* obj, const QString& name);
  bool py_spi_wname(ftdiComObj* obj, const QString& name, int val);
  void py_spi_queue_ctrl(ftdiComObj* obj, bool onoff);
  bool py_spi_queue_write(ftdiComObj* obj);
  QString py_spi_queue_read(ftdiComObj* obj);

};


#endif
