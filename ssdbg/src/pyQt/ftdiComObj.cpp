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
// File           : ftdiComObj.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : python object declaration
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "pyQt/udobj/ftdiComObj.h"
#include "ftdi/spi_access.h"
#include "ftdi/ftdi_usb2spi.h"
#include "childdlg/dlghandle.h"
#include <math.h>
#include <QDebug>
#include <QWidget>
#include <QApplication>

ftdiComObj::ftdiComObj(QWidget* pWidget)
{
  m_pMainWidget = (ssdbg *)pWidget;
}

ftdiComObj::~ftdiComObj()
{}

int ftdiComObj::ftdi_spi_rbyte(unsigned char adr, int len)
{
  if (len < 0) {
    len *= -1;
    spi_rdburst_byte(adr, len);
  }
  else {
    QString stradr = QString("").sprintf("0x%02x", adr);
    int rdat = 0;
    // qDebug() << __func__ << ",   line : " << __LINE__;
    set_lock_ssdbg(true);
    rdat = m_pMainWidget->m_ptbldlg->UpdateAddrData(stradr, "RDOP");
    set_lock_ssdbg(false);
    return rdat;
  }
  return 0;
}

bool ftdiComObj::ftdi_spi_wbyte(unsigned char adr, unsigned char wdat, int len)
{
  if (len < 0){
    len *= -1;
    spi_wrburst_byte(adr, &wdat, len);
    return true;
  }
  else {
    QString stradr = QString("").sprintf("0x%02x", adr);
    QString strdat = QString("").sprintf("0x%02x", wdat);
    
    // qDebug() << "wbyte addr = " << stradr << "wdata = " << strdat;
    // qDebug() << __func__ << ",   line : " << __LINE__;
    set_lock_ssdbg(true);
    m_pMainWidget->m_ptbldlg->UpdateAddrData(stradr, strdat);
    set_lock_ssdbg(false);
    return true;
  }
}

int ftdiComObj::ftdi_spi_rname(const QString& name)
{
  int rdat = 0;
  qDebug() << "rname = " << name;
  // qDebug() << __func__ << ",   line : " << __LINE__;
  set_lock_ssdbg(true);
  rdat = m_pMainWidget->m_ptbldlg->ReadNGetValue(name);
  set_lock_ssdbg(false);
  return rdat;
}

bool ftdiComObj::ftdi_spi_wname(const QString& name, int val)
{
  bool status = false;
  // qDebug() << "wname = " << name;
  // qDebug() << __func__ << ",   line : " << __LINE__;
  set_lock_ssdbg(true);
  status = m_pMainWidget->m_ptbldlg->SetValuenWrite(name, val, true);
  set_lock_ssdbg(false);
  return status;
}

void ftdiComObj::ftdi_spi_queue_ctrl(bool onoff)
{
  spi_queue_ctrl(onoff);
}

bool ftdiComObj::ftdi_spi_queue_write()
{
  spi_queue_write();
  return true;
}

QString ftdiComObj::ftdi_spi_queue_read()
{
  int rcnt = spi_queue_rcnt();
  uint8_t* rddat = NULL;
  QString rtnstr;
  
  rddat = new uint8_t[rcnt];
  memset(rddat, 0x00, sizeof(uint8_t)*rcnt);
  
  // spi_queue_read((BYTE*)rddat);
  
  for (int i=0; i<rcnt; i++)
    rtnstr += QString("").sprintf("0x%02x@", rddat[i]);
  
  delete[] rddat;
  rddat = NULL;
  
  return rtnstr;
}

ftdiComObj* ftdiComObjWrapper::new_ftdiComObj()
{
  QWidgetList tmp = QApplication::topLevelWidgets();

  for (int i = 0; i < tmp.count(); i++)
  {
    if (!tmp.at(i)->objectName().compare("ssdbgClass"))
    {
      return new ftdiComObj(tmp.at(i));
    }
  }
  
  return Q_NULLPTR;
}

void ftdiComObjWrapper::delete_ftdiComObj(ftdiComObj* obj)
{
  delete obj;
}

int ftdiComObjWrapper::py_spi_rbyte(ftdiComObj* obj, unsigned char adr, int len) 
{
  return obj->ftdi_spi_rbyte(adr, len);
}

bool ftdiComObjWrapper::py_spi_wbyte(ftdiComObj* obj, unsigned char adr, unsigned char wdat, int len)
{
  return obj->ftdi_spi_wbyte(adr, wdat, len);
}

int ftdiComObjWrapper::py_spi_rname(ftdiComObj* obj, const QString& name)
{
  return obj->ftdi_spi_rname(name);
}

bool ftdiComObjWrapper::py_spi_wname(ftdiComObj* obj, const QString& name, int val)
{
  return obj->ftdi_spi_wname(name, val);
}

void ftdiComObjWrapper::py_spi_queue_ctrl(ftdiComObj* obj, bool onoff)
{
  return obj->ftdi_spi_queue_ctrl(onoff);
}

bool ftdiComObjWrapper::py_spi_queue_write(ftdiComObj* obj)
{
  return obj->ftdi_spi_queue_write();
}

QString ftdiComObjWrapper::py_spi_queue_read(ftdiComObj* obj)
{
  return obj->ftdi_spi_queue_read();
}

