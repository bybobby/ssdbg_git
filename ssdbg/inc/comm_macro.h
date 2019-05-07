

#ifndef __COMM_MACRO_H__
#define __COMM_MACRO_H__


#define     COMM_SINGLE_WR    0xa0
#define     COMM_SINGLE_RD    0xb0
#define     COMM_MULTI_WR     0xc0
#define     COMM_MULTI_RD     0xd0
#define     COMM_EEPROM_WR    0xe0
#define     COMM_GET_VER      0xf0

#define     COMM_PROC_UFUNC0  0xea
#define     COMM_PROC_UFUNC1  0xeb
#define     COMM_PROC_UFUNC2  0xec
#define     COMM_PROC_UFUNC3  0xed

#define     COMM_RESP_FIXB1   0xab
#define     COMM_RESP_FIXB2   0xd3

#define     COMM_PROC_UF2ON   0xc3
#define     COMM_PROC_UF2OFF  0xf2

#define     COMM_RESP_OKAY    0x23
#define     COMM_RESP_ERROR   0x59
#define     COMM_RESP_READ    0x7a
#define     COMM_RESP_BRD_ERR 0x3c
#define     COMM_RESP_UFUNC0  0xe2
#define     COMM_RESP_UFUNC1  0xe3
#define     COMM_RESP_UFUNC2  0xe4
#define     COMM_RESP_UFUNC3  0xe5
#define     COMM_RESP_GET_VER 0xe6

#define     COMM_RESP_FIXB1   0xab
#define     COMM_RESP_FIXB2   0xd3

#define     WAIT_FLAG_SET     0xa
#define     WAIT_FLAG_UNSET   0xb
#define     WAIT_FLAG_GET     0xc

#define     RECV_CNT_RESET    0xa
#define     RECV_CNT_INC      0xb
#define     RECV_CNT_GET      0xc


#endif
