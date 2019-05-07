

#include "ftdi/ftdi_usb2spi.h"
#include "ftdi/spi_access.h"
#include "childdlg/dbgdlg.h"
#include "childdlg/dlghandle.h"
#include <QMutex>
#include <QString>

dbgdlg*    m_dbgDlg = NULL;

void spiMutex(bool lock)
{
  static QMutex vMutex(QMutex::NonRecursive);
  
  if (!lock) {
    // qDebug() << "access Unlocked !!";
    vMutex.unlock();
  }
  else {
    // qDebug() << "access locked !!";
    while(!vMutex.try_lock());
  }
}

void spi_debug_alloc(QWidget* p_Widget)
{
  if (p_Widget != NULL)
    m_dbgDlg = (dbgdlg*)p_Widget;
  else
    m_dbgDlg = NULL;
}

WORD spi_rd_word(WORD adr)
{
	WORD bdata;
	spiMutex(true);
	spi_read_word(adr, &bdata, 1);
	spiMutex(false);
	
	return bdata;
}

bool spi_wr_word(WORD adr, WORD bdata)
{
	WORD dat = bdata;
	
	spiMutex(true);
	spi_write_word(adr, &dat, 1);
	spiMutex(false);
	
	return true;
}

WORD* spi_rdburst_word(WORD adr, WORD wlen)
{
	WORD* buf;
	
	buf = new WORD[wlen];
	
	spiMutex(true);
	spi_read_word(adr, buf, wlen);
	spiMutex(false);
	
	return buf;
}

bool spi_wrburst_word(WORD adr, WORD* bdata, WORD wlen)
{
	spiMutex(true);
	spi_write_word(adr, bdata, wlen);
	spiMutex(false);
	
	return true;
}

BYTE spi_rd_byte(BYTE adr)
{
	BYTE bdata = 0;
  QString rstr;
  
	spiMutex(true);
	SPI_RBYTE(adr, &bdata, 1);
  set_lock_dbgdlg(true);
  if (m_dbgDlg != NULL){
    rstr.sprintf("read  -> address : 0x%02x,   data = 0x%02x", adr, bdata);
    m_dbgDlg->appendline(rstr);
  }
  set_lock_dbgdlg(false);
	spiMutex(false);
	
	return bdata;
}

bool spi_wr_byte(BYTE adr, BYTE bdata)
{
	BYTE dat = bdata;
  QString wstr;
	
	spiMutex(true);
	SPI_WBYTE(adr, &dat, 1);
  set_lock_dbgdlg(true);
  if (m_dbgDlg != NULL){
    wstr.sprintf("write  -> address : 0x%02x,   data = 0x%02x", adr, bdata);
    m_dbgDlg->appendline(wstr);
  }
  set_lock_dbgdlg(false);
	spiMutex(false);
	
	return true;
}

BYTE* spi_rdburst_byte(BYTE adr, WORD wlen)
{
	BYTE* buf;
	
	buf = new BYTE[wlen];
	
	spiMutex(true);
	SPI_RBYTE(adr, buf, wlen);
	spiMutex(false);
	
	return buf;
}

bool spi_wrburst_byte(BYTE adr, BYTE* bdata, WORD wlen)
{
	spiMutex(true);
	SPI_WBYTE(adr, bdata, wlen);
	spiMutex(false);
	
	return true;
}

