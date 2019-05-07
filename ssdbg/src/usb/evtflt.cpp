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
// File           : evtflt.cpp
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

#include "usb/evtflt.h"
#include "childdlg/gphdlg.h"
#include "childdlg/dlghandle.h"
#include "chart/mthread.h"

#include <stdio.h>
#include <string>
#include <QObject>
#include <QDebug>

#ifdef Q_OS_WIN32
#include <windows.h>
#include <Dbt.h>
#endif

using namespace std;

#define FTDI_D2XX_GUID    3
#define VCP_DEVICE_GUID   4

static const GUID GuidInterfaceList[] =
{
  // USB Raw Device Interface Class GUID
  { 0xa5dcbf10, 0x6530, 0x11d2,{ 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed } },
  // Disk Device Interface Class GUID
  { 0x53f56307, 0xb6bf, 0x11d0,{ 0x94, 0xf2, 0x00, 0xa0, 0xc9, 0x1e, 0xfb, 0x8b } },
  //Human Interface Device Class GUID
  { 0x4d1e55b2, 0xf16f, 0x11Cf,{ 0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30 } },
  // FTDI_D2XX_Device Class GUID
  { 0x219d0508, 0x57a8, 0x4ff5,{ 0x97, 0xa1, 0xbd, 0x86, 0x58, 0x7c, 0x6c, 0x7e } },
  // FTDI_VCP_Device Class GUID
  { 0x86e0d1e0L, 0x8089, 0x11d0,{ 0x9c, 0xe4, 0x08, 0x00, 0x3e, 0x30, 0x1f, 0x73 } },
};

QWinEventFilter::QWinEventFilter(QObject *parent)
  : QObject(parent)
  , m_dev_notify(0)
  , m_ftdi_acnt(0)
  , m_ftdi_dcnt(-1)
  , m_pMainDlg(Q_NULLPTR)
{
  connect(this, SIGNAL(usbattached()), this, SLOT(try_connect()));
  connect(this, SIGNAL(usbdeattached()), this, SLOT(try_disconnect()));
}

bool QWinEventFilter::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
  // Q_UNUSED(eventType);
  // Q_UNUSED(result);

#ifdef Q_OS_WIN32

  MSG *msg = static_cast<MSG*>(message);
  
  if (msg->message == WM_DEVICECHANGE) {
  
    PDEV_BROADCAST_HDR pHdr = (PDEV_BROADCAST_HDR)msg->lParam;

    if(msg->wParam == DBT_DEVICEARRIVAL)
    {
      
      if (pHdr->dbch_devicetype == DBT_DEVTYP_DEVICEINTERFACE) {
        m_dev_notify++;
      }
      PDEV_BROADCAST_DEVICEINTERFACE pdbch = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
      QString name = QString::fromWCharArray((wchar_t*)pdbch->dbcc_name);
      // qDebug() << name;
      if (name.contains(m_guidstr[FTDI_D2XX_GUID], Qt::CaseSensitive)) {
        if (m_ftdi_acnt < 4)
          m_ftdi_acnt++;

        if (m_ftdi_acnt == 4) {
          qDebug() << "FTDI attached !!";
          m_ftdi_dcnt = 4;
          emit usbattached();
        }
      }
      else if (name.contains(m_guidstr[VCP_DEVICE_GUID], Qt::CaseSensitive)) {
        // qDebug() << "Attached VCP Device ";
        emit usbattached();
      }
      
    }
    else if(msg->wParam == DBT_DEVICEREMOVECOMPLETE)
    {
      PDEV_BROADCAST_DEVICEINTERFACE pdbch = (PDEV_BROADCAST_DEVICEINTERFACE)pHdr;
      QString name = QString::fromWCharArray((wchar_t*)pdbch->dbcc_name);
      if (name.contains(m_guidstr[FTDI_D2XX_GUID], Qt::CaseSensitive)) {
        if (m_ftdi_dcnt > 0)
          m_ftdi_dcnt--;

        if (m_ftdi_dcnt == 0) {
          qDebug() << "FTDI De-attached !!";
          m_ftdi_acnt = 0;
          emit usbdeattached();
        }
      }
      else if (name.contains(m_guidstr[VCP_DEVICE_GUID], Qt::CaseSensitive)) {
        // qDebug() << "De-Attached VCP Device ";
        emit usbdeattached();
      }
    }
  }
#endif

  // Return false so that the event is propagated
  return false;
}


void QWinEventFilter::registerEventWindow(QWindow *p_Wnd)
{
  if( p_Wnd != Q_NULLPTR )
  {
#ifdef Q_OS_WIN32
    for (int i = 0; i < 5; i++) {
      GUID guid = GuidInterfaceList[i];
      m_guidstr[i].sprintf("{%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x}", 
        guid.Data1, guid.Data2, guid.Data3,
        guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
        guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

      DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

      ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
      NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
      NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
      NotificationFilter.dbcc_classguid = guid;
      HDEVNOTIFY hDevNotify = RegisterDeviceNotification((HANDLE)p_Wnd->winId(), &NotificationFilter, DEVICE_NOTIFY_WINDOW_HANDLE);
      if (NULL == hDevNotify)
      {
        qDebug() << "device initialize failed !!";
      }
    }
#endif
  }
}

void QWinEventFilter::registerMainWidget(QWidget *p_Widget)
{
  if (p_Widget != Q_NULLPTR) {
    m_pMainDlg = (ssdbg*) p_Widget;
    m_ftdi_dcnt = (!m_pMainDlg->m_conn_chk)? 0 : 4;
  }
}

void QWinEventFilter::try_connect()
{
  qDebug() << __func__ << ",   line : " << __LINE__;
  set_lock_gphdlg(true);
  set_lock_ssdbg(true);
  if (m_pMainDlg->try_conn(false)){
    m_pMainDlg->intr_read(false);
    Sleep(5);
    m_pMainDlg->try_discon();
    m_pMainDlg->try_conn(false);
    m_pMainDlg->intr_read(false);
    Sleep(5);
    m_pMainDlg->try_discon();
    m_pMainDlg->try_conn(true);
  }
  // m_pMainDlg->m_pgphdlg->thread_resume();
  set_lock_ssdbg(false);
  set_lock_gphdlg(false);
}

void QWinEventFilter::try_disconnect()
{
  qDebug() << __func__ << ",   line : " << __LINE__;
  set_lock_gphdlg(true);
  set_lock_ssdbg(true);
  m_pMainDlg->try_discon();
  // m_pMainDlg->m_pgphdlg->thread_pause();
  set_lock_ssdbg(false);
  set_lock_gphdlg(false);
}
