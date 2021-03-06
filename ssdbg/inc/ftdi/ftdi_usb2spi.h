#ifndef __FTDI_USB2SPI_H__
#define __FTDI_USB2SPI_H__

#define MemSize 16 //define data quantity you want to send out
#define CSNUM   5
#define WRFLAG	~0x80
#define RDFLAG	0x80
#define GPIO_MODE 1
#define FTD2XX_STATIC 1

typedef unsigned char BYTE;
typedef unsigned short WORD;


#ifdef GPIO_MODE
  #define SPI_RBYTE gpio_read_byte
  #define SPI_WBYTE gpio_write_byte
#else
  #define SPI_RBYTE spi_read_byte
  #define SPI_WBYTE spi_write_byte
#endif

bool ftdi_connect();
bool ftdi_disconnect();

bool spi_read_word(WORD address, WORD* bdata, WORD wlen);
bool spi_write_word(WORD address, WORD* bdata, WORD wlen);
bool spi_read_byte(BYTE address, BYTE* bdata, WORD wlen);
bool spi_write_byte(BYTE address, BYTE* bdata, WORD wlen);
void spi_queue_ctrl(bool onoff);
void spi_queue_write();
BYTE* spi_queue_read(BYTE* bdata);
WORD spi_queue_rcnt();
bool gpio_read_byte(BYTE address, BYTE* bdata, WORD wlen);
bool gpio_write_byte(BYTE address, BYTE* bdata, WORD wlen);
bool gpio_read_bit(BYTE bitpos);
bool gpio_get_intr(BYTE inum);
void gpio_b5_wait_high();
void gpio_b5_wait_low();
void gpio_set_high_byte();
void outbufsel();
void cpybufcnt();

#endif