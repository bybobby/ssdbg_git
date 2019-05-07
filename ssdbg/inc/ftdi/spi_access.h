#ifndef __SPI_ACESS_H__
#define __SPI_ACESS_H__

#pragma once

#include <qglobal.h>
#include <stdint.h>

QT_FORWARD_DECLARE_CLASS(QWidget);

typedef unsigned char BYTE;
typedef unsigned short WORD;

WORD spi_rd_word(WORD adr);
bool spi_wr_word(WORD adr, WORD bdata);
WORD* spi_rdburst_word(WORD adr, WORD wlen);
bool spi_wrburst_word(WORD adr, WORD* bdata, WORD wlen);

BYTE spi_rd_byte(BYTE adr);
bool spi_wr_byte(BYTE adr, BYTE bdata);
BYTE* spi_rdburst_byte(BYTE adr, WORD wlen);
bool spi_wrburst_byte(BYTE adr, BYTE* bdata, WORD wlen);
void spi_debug_alloc(QWidget* p_Widget);

#endif