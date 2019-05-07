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
// File           : uart_conn.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : uart connection control
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-08-06 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////


#include "comm_macro.h"
#include "serial/qsport.h"
#include "serial/uart_acc.h"
#include "serial/uart_conn.h"
#include "childdlg/ssdbg.h"
#include "childdlg/gphdlg.h"
#include <QDebug>
#include <QWidget>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <cstdio>
#include <iostream>

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using namespace coms;
using namespace std;

string portnum("");
QSerialPortInfo stmicro;

CSerialEvt* get_serial_hdl(bool get_set)
{
  static coms::CSerialEvt* p_COM = NULL;

  if (get_set) {
    p_COM = new coms::CSerialEvt();
  }

  return p_COM;
}

QSerialPort* get_qsport_hdl(bool get_set)
{
  static QSerialPort* p_COM = NULL;

  if (get_set) {
    p_COM = new QSerialPort();
  }

  return p_COM;
}

qsport* get_whdl(bool get_set)
{
  ssdbg* p_ssdbg = getmainWidget(Q_NULLPTR);
  
  return p_ssdbg->m_pqsport;

  // qsport* p_qsport = NULL;
  // 
  // if (get_set) {
  //   p_qsport = new qsport(get_qsport_hdl(false), getmainWidget(Q_NULLPTR));
  // }
  // 
  // return p_qsport;
}

void delay(int millisec)
{
  QTime dieTime = QTime::currentTime().addMSecs(millisec);

  while (QTime::currentTime() < dieTime)
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
}

void delay_allevt(int millisec)
{
  QTime dieTime = QTime::currentTime().addMSecs(millisec);

  while (QTime::currentTime() < dieTime)
    QApplication::processEvents(QEventLoop::AllEvents);
}

void SerialEventManager(uint64_t obj, uint32_t evt)
{
  char *buffer;
  int   size;
  CSerialEvt *p_ComPort;
  uint8_t* lp_rcvbuf = NULL;
  uint16_t idx = 0;
  static bool proc_header = false;
  static int32_t len = 0;
  int ret;
  p_ComPort = (CSerialEvt *)obj;

  if (p_ComPort != 0)
  {
    switch (evt)
    {
    case  SERIAL_CONNECTED:
      qDebug() << QString("Connected !! \n");
      break;
    case  SERIAL_DISCONNECTED:
      qDebug() << QString("Disconnected !! \n");
      break;
    case  SERIAL_DATA_SENT:
      uart_wt_wait(WAIT_FLAG_UNSET);
      break;
    case  SERIAL_RING:
      qDebug() << QString("DRING !! \n");
      break;
    case  SERIAL_CD_ON:
      qDebug() << QString("Carrier Detected !! \n");
      break;
    case  SERIAL_CD_OFF:
      qDebug() << QString("No more carrier !! \n");
      break;
    case  SERIAL_DATA_ARRIVAL:
      size = p_ComPort->getDataInSize();
      buffer = p_ComPort->getDataInBuffer();
      lp_rcvbuf = uart_rcv_buf(1);
      ssdbg* p_ssdbg = getmainWidget(Q_NULLPTR);

      idx = uart_rcv_cnt(RECV_CNT_RESET, 0);
      memcpy(&lp_rcvbuf[idx], buffer, size);
      
      idx = uart_rcv_cnt(RECV_CNT_INC, size);

      if ((lp_rcvbuf[0] == COMM_RESP_READ) || 
          (lp_rcvbuf[0] == COMM_RESP_UFUNC3) || 
          (lp_rcvbuf[0] == COMM_RESP_UFUNC2)) {
        len = (lp_rcvbuf[1] << 8) | lp_rcvbuf[2];
        eeprom_wait(WAIT_FLAG_UNSET);
        uart_rt_wait(WAIT_FLAG_UNSET);
        ufunc3_wait(WAIT_FLAG_UNSET);
        if (lp_rcvbuf[0] == COMM_RESP_UFUNC2){
          if (p_ssdbg != Q_NULLPTR)
            emit p_ssdbg->m_pgphdlg->takedata();
        }
      }
      else if (lp_rcvbuf[0] == COMM_RESP_OKAY) {
        if (lp_rcvbuf[1] == COMM_RESP_FIXB1) {
          if (lp_rcvbuf[2] == COMM_RESP_FIXB2) {
            eeprom_wait(WAIT_FLAG_UNSET);
            uart_rt_wait(WAIT_FLAG_UNSET);
            ufunc3_wait(WAIT_FLAG_UNSET);
            len = 5;
          }
        }
      }
      else if (lp_rcvbuf[0] == COMM_RESP_GET_VER) {
        get_ver_wait(WAIT_FLAG_UNSET);
      }
      else {
        qDebug() << "len = " << size << ",   idx = " << idx << ",    dat = " << (uint8_t)buffer[0]; // << ",  cpy = " << (uint8_t)lp_rcvbuf[idx - 1];
        // ret = QMessageBox::warning(this, tr("SSDBG"),
            //   tr("The received UART ERROR"),
            //   QMessageBox::Ok);
      }

      p_ComPort->dataHasBeenRead();
      break;
    }
  }
}

int EnumerateCOM()
{
  vector<coms::PortInfo> devices_found = coms::list_ports();
  vector<coms::PortInfo>::iterator iter = devices_found.begin();

  string st_port_name("STMicroelectronics STLink Virtual COM Port");
  //string st_port_name("USB Serial Port(");

  uint8_t pcnt = 0;

  while (iter != devices_found.end())
  {
    coms::PortInfo device = *iter++;

    qDebug() << QString("(") << device.port.c_str()
      << QString(", ") << device.description.c_str()
      << QString(", ") << device.hardware_id.c_str()
      << QString(")");

    portnum = device.port.c_str();
    string devicename(device.description.c_str());
    int r = devicename.find(st_port_name);

    if (r >= 0) {
      // atoi(&portnum.at(portnum.length() - 1));
      return 1;
    }
  }

  return 0;
}

bool chk_sport()
{
  const auto infos = QSerialPortInfo::availablePorts();

  for (const QSerialPortInfo &info : infos) {
    qDebug() << info.portName();
    qDebug() << info.description();
    qDebug() << info.manufacturer();
    if (!info.description().compare("STMicroelectronics STLink Virtual COM Port")) {
      stmicro = info;
      return true;
    }
  }

  return false;
}

std::string get_portnum()
{
  return portnum;
}

QSerialPortInfo* get_portinfo()
{
  return &stmicro;
}

ssdbg* getmainWidget(QWidget *p_Widget)
{
  static ssdbg* p_widget = NULL;

  if (p_Widget != Q_NULLPTR) {
    p_widget = (ssdbg*)p_Widget;
  }

  return p_widget;
}

void uart_flush()
{
  coms::CSerialEvt* lp_COM = get_serial_hdl(false);

  // lp_COM->dataHasBeenRead();
  PurgeComm(lp_COM, PURGE_RXABORT);
  PurgeComm(lp_COM, PURGE_RXCLEAR);
  PurgeComm(lp_COM, PURGE_TXABORT);
  PurgeComm(lp_COM, PURGE_TXCLEAR);
  FlushFileBuffers(lp_COM);
}