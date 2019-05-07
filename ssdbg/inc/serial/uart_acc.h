

#ifndef __UART_ACC_H__

#define __UART_ACC_H__

#include <stdint.h>

void uart_byte_wr(uint8_t adr, uint8_t dat);
uint8_t uart_byte_rd(uint8_t adr);
void uart_write(uint8_t adr, uint8_t* dat, uint16_t len);
void uart_read(uint8_t adr, uint8_t* dat, uint16_t len);
void eeprom_write(uint8_t adr, uint16_t dat);
void send_ufunc2_cmd(bool onoff);
uint16_t send_ufunc3_cmd(uint16_t* rbuf);
uint8_t* uart_rcv_buf(uint8_t reset);
uint16_t uart_rcv_cnt(uint8_t ctrl, uint16_t inc);
uint8_t uart_wt_wait(uint8_t ctrl);
uint8_t uart_rt_wait(uint8_t ctrl);
uint8_t eeprom_wait(uint8_t ctrl);
uint8_t ufunc2_wait(uint8_t ctrl);
uint8_t ufunc3_wait(uint8_t ctrl);
uint16_t send_get_verinfo(uint8_t* rbuf);
uint8_t get_ver_wait(uint8_t ctrl);

#endif

