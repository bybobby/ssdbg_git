
#include "serial/qsport.h"
#include "serial/uart_acc.h"
#include "serial/uart_conn.h"
#include "comm_macro.h"
#include <QMessageBox>
#include <QDialog>
#include <QDebug>
#include <QWidget>
#include <QThread>

void uart_byte_wr(uint8_t adr, uint8_t dat)
{
  uint8_t lv_dat = dat;
  
  uart_write(adr, &lv_dat, 1);
}

uint8_t uart_byte_rd(uint8_t adr)
{
  uint8_t rdat;

  uart_read(adr, &rdat, 1);

  return rdat;
}

void uart_write(uint8_t adr, uint8_t* dat, uint16_t len)
{
  uint8_t cmd[8] = {0};
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);
  uint16_t llen = len + 4;
  
  cmd[0] = COMM_MULTI_WR;
  cmd[1] = adr;
  cmd[2] = (uint8_t)((len >> 8) & 0xff);
  cmd[3] = (uint8_t)(len & 0xff);
  
  memcpy(&cmd[4], dat, sizeof(uint8_t)*len);
  
  if ((llen % 8) != 0)
    llen = ((llen >> 3) + 1)*8;

  uart_wt_wait(WAIT_FLAG_SET);
  // lp_COM->sendData((char *)cmd, llen);
  p_hdl->write(cmd, llen);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);
}


void uart_read(uint8_t adr, uint8_t* dat, uint16_t len)
{
  uint8_t cmd[8] = { 0 };
  
  uint8_t* lp_rcvbuf = NULL;
  static uint32_t timer = 0;
  int ret = 0;

  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);

  cmd[0] = COMM_MULTI_RD;
  cmd[1] = adr;
  cmd[2] = ((len >> 8) & 0xff);
  cmd[3] = (len & 0xff);

  uart_rcv_cnt(RECV_CNT_RESET, 0);
  uart_rt_wait(WAIT_FLAG_SET);
  // lp_COM->sendData((char *)cmd, 8);
  uart_wt_wait(WAIT_FLAG_SET);
  p_hdl->write(cmd, 8);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);
  timer = 0;
  while (uart_rt_wait(WAIT_FLAG_GET)) {
    delay(1);
  }
  
  lp_rcvbuf = uart_rcv_buf(0);
  memcpy(dat, &lp_rcvbuf[3], sizeof(uint8_t)*len);
}

void eeprom_write(uint8_t adr, uint16_t dat)
{
  uint8_t cmd[8] = { 0 };
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);

  cmd[0] = COMM_EEPROM_WR;
  cmd[1] = adr;
  cmd[2] = ((dat >> 8) & 0xff);
  cmd[3] = (dat & 0xff);
  
  uart_rcv_cnt(RECV_CNT_RESET, 0);
  eeprom_wait(WAIT_FLAG_SET);
  // lp_COM->sendData((char *)cmd, 8);
  uart_wt_wait(WAIT_FLAG_SET);
  p_hdl->write(cmd, 8);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);
  while (eeprom_wait(WAIT_FLAG_GET))
    delay(1);
}

void send_ufunc2_cmd(bool onoff)
{
  uint8_t cmd[8] = { 0 };
  //coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);
  uint16_t cnt = 0;

  cmd[0] = COMM_PROC_UFUNC2;
  cmd[1] = (!onoff)? COMM_PROC_UF2OFF : COMM_PROC_UF2ON;
  uart_rcv_cnt(RECV_CNT_RESET, 0);

  if (!onoff)
    ufunc2_wait(WAIT_FLAG_SET);

  //lp_COM->sendData((char *)cmd, 8);
  uart_wt_wait(WAIT_FLAG_SET);
  p_hdl->write(cmd, 8);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);

  if (!onoff) {
    while (ufunc2_wait(WAIT_FLAG_GET)) {
      if (cnt++ > 128) {
        p_hdl->write(cmd, 8);
        while (uart_wt_wait(WAIT_FLAG_GET))
          delay(1);
        cnt = 0;
      }
      delay(1);
    }
  }
}

uint16_t send_ufunc3_cmd(uint16_t* rbuf)
{
  uint8_t* lp_rcvbuf = NULL;
  uint8_t cmd[8] = { 0 };
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);

  cmd[0] = COMM_PROC_UFUNC3;
  uart_rcv_cnt(RECV_CNT_RESET, 0);
  ufunc3_wait(WAIT_FLAG_SET);
  // lp_COM->sendData((char *)cmd, 8);
  uart_wt_wait(WAIT_FLAG_SET);
  p_hdl->write(cmd, 8);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);
  while (ufunc3_wait(WAIT_FLAG_GET))
    delay(1);

  lp_rcvbuf = uart_rcv_buf(0);
  memcpy(rbuf, (uint16_t*)&lp_rcvbuf[3], sizeof(uint16_t)*32);

  return 0;
}

uint8_t uart_wt_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if(ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}

uint8_t uart_rt_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if (ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}

uint8_t eeprom_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if (ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}

uint8_t ufunc2_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if (ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}

uint8_t* uart_rcv_buf(uint8_t reset)
{
  static uint8_t rcv_buff0[128];
  static uint8_t rcv_buff1[128];
  static bool sw_buf = false;

  if (reset) {
    sw_buf = (!sw_buf)? true : false;
    memset((!sw_buf)? rcv_buff0 : rcv_buff1, 0x00, sizeof(uint8_t) * 128);
  }

  return (!sw_buf)? rcv_buff0 : rcv_buff1;
}

uint16_t uart_rcv_cnt(uint8_t ctrl, uint16_t inc)
{
  static uint16_t cnt = 0;
  
  if (ctrl == RECV_CNT_RESET)
    cnt = 0;
  else if (ctrl == RECV_CNT_INC)
    cnt += inc;
  
  return cnt;
}

uint16_t send_get_verinfo(uint8_t* rbuf)
{
  uint8_t* lp_rcvbuf = NULL;
  uint8_t cmd[8] = { 0 };
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  qsport* p_hdl = get_whdl(false);

  cmd[0] = COMM_GET_VER;
  uart_rcv_cnt(RECV_CNT_RESET, 0);
  get_ver_wait(WAIT_FLAG_SET);
  // lp_COM->sendData((char *)cmd, 8);
  uart_wt_wait(WAIT_FLAG_SET);
  p_hdl->write(cmd, 8);
  while (uart_wt_wait(WAIT_FLAG_GET))
    delay(1);
  while (get_ver_wait(WAIT_FLAG_GET))
    delay(1);

  lp_rcvbuf = uart_rcv_buf(0);
  memcpy(rbuf, (uint8_t*)&lp_rcvbuf[1], sizeof(uint8_t) * 2);

  return 0;
}

uint8_t get_ver_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if (ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}

uint8_t ufunc3_wait(uint8_t ctrl)
{
  static uint8_t wait_flag = 0;

  if (ctrl == WAIT_FLAG_SET) {
    wait_flag = 1;
  }
  else if (ctrl == WAIT_FLAG_UNSET) {
    wait_flag = 0;
  }

  return wait_flag;
}