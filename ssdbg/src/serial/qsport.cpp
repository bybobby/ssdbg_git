/****************************************************************************
**
** Copyright (C) 2013 Laszlo Papp <lpapp@kde.org>
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtSerialPort module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "serial/qsport.h"
#include "serial/uart_acc.h"
#include "serial/uart_conn.h"
#include "comm_macro.h"
#include "childdlg/ssdbg.h"
#include "childdlg/gphdlg.h"

#include <math.h>
#include <QWidget>
#include <QCoreApplication>
#include <QDebug>

qsport::qsport(QSerialPort *serialPort, QObject *parent) :
  QObject(parent),
  m_serialPort(serialPort)
{
  connect(m_serialPort, &QSerialPort::readyRead, this, &qsport::handleReadyRead);
  connect(m_serialPort, &QSerialPort::bytesWritten, this, &qsport::handleBytesWritten);
  connect(m_serialPort, &QSerialPort::errorOccurred, this, &qsport::handleError);
  connect(&m_wrtimer, &QTimer::timeout, this, &qsport::handleWRTimeout);
  connect(&m_rdtimer, &QTimer::timeout, this, &qsport::handleRDTimeout);
  m_wrtimer.setSingleShot(true);
  m_rdtimer.setSingleShot(true);
}

void qsport::handleReadyRead()
{
  int count = floor(m_serialPort->size() / 75.0);
  uint8_t* lp_rcvbuf = NULL;
  static int32_t len = 0;
  ssdbg* p_ssdbg = getmainWidget(Q_NULLPTR);

  lp_rcvbuf = uart_rcv_buf(1);
  qDebug() << m_serialPort->size();
  qDebug() << count;

  if (count >= 1) {
    for (int i = 0; i < count; i++) {
      m_serialPort->read((char*)lp_rcvbuf, 75);
      if ((lp_rcvbuf[0] == COMM_RESP_READ) ||
        (lp_rcvbuf[0] == COMM_RESP_UFUNC3) ||
        (lp_rcvbuf[0] == COMM_RESP_UFUNC2)) {
        len = (lp_rcvbuf[1] << 8) | lp_rcvbuf[2];
        if (lp_rcvbuf[0] == COMM_RESP_READ)
          uart_rt_wait(WAIT_FLAG_UNSET);
        if (lp_rcvbuf[0] == COMM_RESP_UFUNC3)
          ufunc3_wait(WAIT_FLAG_UNSET);
        if (lp_rcvbuf[0] == COMM_RESP_UFUNC2) {
          if (p_ssdbg != Q_NULLPTR)
            emit p_ssdbg->m_pgphdlg->takedata();
        }
      }
      else if (lp_rcvbuf[0] == COMM_RESP_OKAY) {
        if (lp_rcvbuf[1] == COMM_RESP_FIXB1) {
          if (lp_rcvbuf[2] == COMM_RESP_FIXB2) {
            eeprom_wait(WAIT_FLAG_UNSET);
            ufunc2_wait(WAIT_FLAG_UNSET);
            len = 5;
          }
        }
      }
      else if (lp_rcvbuf[0] == COMM_RESP_GET_VER) {
        get_ver_wait(WAIT_FLAG_UNSET);
      }
      else {
        m_serialPort->readAll();
        m_serialPort->reset();
      }
    }
  }

  // if (!m_rdtimer.isActive())
  //    m_rdtimer.start(5000);
}

void qsport::handleBytesWritten(qint64 bytes)
{
  m_bytesWritten += bytes;
  // if (m_bytesWritten == m_writeData.size()) {
  if (m_bytesWritten == m_wdata_size) {
    m_bytesWritten = 0;
    uart_wt_wait(WAIT_FLAG_UNSET);
    // qDebug() << QObject::tr("Data successfully sent to port %1")
    //                 .arg(m_serialPort->portName()) << endl;
  }
  m_wrtimer.stop();
}

void qsport::handleRDTimeout()
{
  if (m_readData.isEmpty()) {
    qDebug() << QObject::tr("No data was currently available "
      "for reading from port %1")
      .arg(m_serialPort->portName())
      << endl;
  }
  else {
    qDebug() << QObject::tr("Data successfully received from port %1")
      .arg(m_serialPort->portName())
      << endl;
    qDebug() << m_readData << endl;
  }
}

void qsport::handleWRTimeout()
{
  qDebug() << QObject::tr("Operation timed out for port %1, error: %2")
    .arg(m_serialPort->portName())
    .arg(m_serialPort->errorString())
    << endl;
}

void qsport::handleError(QSerialPort::SerialPortError serialPortError)
{
  if (serialPortError == QSerialPort::ReadError) {
    qDebug() << QObject::tr("An I/O error occurred while reading "
      "the data from port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }

  if (serialPortError == QSerialPort::WriteError) {
    qDebug() << QObject::tr("An I/O error occurred while writing"
      " the data to port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }
}

void qsport::write(const QByteArray &wdata)
{
  m_writeData = wdata;

  const qint64 bytesWritten = m_serialPort->write(wdata);

  if (bytesWritten == -1) {
    qDebug() << QObject::tr("Failed to write the data to port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }
  else if (bytesWritten != m_writeData.size()) {
    qDebug() << QObject::tr("Failed to write all the data to port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }

  m_wrtimer.start(5000);
}

void qsport::write(uint8_t* buf, int size)
{
  m_wdata_size = size;

  const qint64 bytesWritten = m_serialPort->write(reinterpret_cast<char*>(buf), size);

  if (bytesWritten == -1) {
    qDebug() << QObject::tr("Failed to write the data to port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }
  else if (bytesWritten != size) {
    qDebug() << QObject::tr("Failed to write all the data to port %1, error: %2")
      .arg(m_serialPort->portName())
      .arg(m_serialPort->errorString())
      << endl;
  }

  m_wrtimer.start(5000);
}
