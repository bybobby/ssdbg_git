

#ifndef __UART_CONN_H__
#define __UART_CONN_H__

#include "serial/serialport.h"
#include <QDialog>

QT_FORWARD_DECLARE_CLASS(ssdbg);
QT_FORWARD_DECLARE_CLASS(qsport);
QT_FORWARD_DECLARE_CLASS(QSerialPort);
QT_FORWARD_DECLARE_CLASS(QSerialPortInfo);

coms::CSerialEvt* get_serial_hdl(bool get_set);
QSerialPort* get_qsport_hdl(bool get_set);
void SerialEventManager(uint64_t obj, uint32_t evt);
int EnumerateCOM();
std::string get_portnum();
ssdbg* getmainWidget(QWidget *p_Widget);
void uart_flush();
bool chk_sport();
QSerialPortInfo* get_portinfo();
qsport* get_whdl(bool get_set);
void delay(int millisec);

#endif