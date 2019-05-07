//
// SPITEST.cpp : VC++ console application.
// this example project use port A of FT2232H to access SPI EEPROM 93C56
// we send 16 word data to 93C56 and read them back, user can see the test
// result in command mode.
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "ftdi/ftdi_usb2spi.h"
#include "ftdi/ftd2xx.h"
#include "common.h"
#include <QDebug>
#include <QMutex>
#include <QThread>

//declare parameters for 93C56
const BYTE SPIDATALENGTH = 11;//3 digit command + 8 digit address
const BYTE READ = '\xC0';//110xxxxx
const BYTE WRITE = '\xA0';//101xxxxx
const BYTE WREN = '\x98';//10011xxx
const BYTE ERAL = '\x90';//10010xxx

//declare for BAD command
const BYTE AA_ECHO_CMD_1 = '\xAA';
const BYTE AB_ECHO_CMD_2 = '\xAB';
const BYTE BAD_COMMAND_RESPONSE = '\xFA';

//declare for MPSSE command
const BYTE MSB_RISING_EDGE_CLOCK_BYTE_OUT = '\x10';
const BYTE MSB_FALLING_EDGE_CLOCK_BYTE_OUT = '\x11';
const BYTE MSB_RISING_EDGE_CLOCK_BIT_OUT = '\x12';
const BYTE MSB_FALLING_EDGE_CLOCK_BIT_OUT = '\x13';
const BYTE MSB_RISING_EDGE_CLOCK_BYTE_IN = '\x20';
const BYTE MSB_RISING_EDGE_CLOCK_BIT_IN = '\x22';
const BYTE MSB_FALLING_EDGE_CLOCK_BYTE_IN = '\x24';
const BYTE MSB_FALLING_EDGE_CLOCK_BIT_IN = '\x26';

const BYTE SEND_IMMEDIATE = '\x87';
const BYTE WAIT_ON_IO_HIGH = '\x88';
const BYTE WAIT_ON_IO_LOW = '\x89';

const BYTE READ_DATA_BITS_LS = '\x81';

#define GPIO_MODE_CYCLE     1

FT_STATUS ftStatus; //Status defined in D2XX to indicate operation result
BYTE OutputBuffer0[196608]; //Buffer to hold MPSSE commands and data to be sent to FT2232H
BYTE OutputBuffer1[196608]; //Buffer to hold MPSSE commands and data to be sent to FT2232H
BYTE InputBuffer[32768];
DWORD dwClockDivisor = 39; //Value of clock divisor, SCL Frequency = 60/((1+19)*2) (MHz) = 1.5 Mhz
DWORD dwNumBytesToSend = 0; //Index of output buffer
DWORD dwNumBytesSent = 0, dwNumBytesRead = 0, dwNumInputBuffer = 0;
BYTE ByteDataRead;

FT_HANDLE ftdiHandle = NULL;
FT_HANDLE hdlRead = NULL;

bool gathering = false;
bool handle_using = false;
bool buftog = false;
DWORD grlen = 0;
BYTE* obuf = OutputBuffer0;
DWORD FixedNumBytesToSend = 0;

bool write_prog(uint8_t reset)
{
  static bool write = false;

  if (reset == 0)
    write = true;
  else if (reset == 1)
    write = false;

  return write;
}

void setMutex(bool lock)
{
  static QMutex spiMutex(QMutex::NonRecursive);
  
  if (!lock) {
    // qDebug() << "spi Unlocked !!";
    spiMutex.unlock();
  }
  else {
    // qDebug() << "spi locked !!";
    while(!spiMutex.try_lock());
  }
}

OVERLAPPED* get_ovWrite(bool reset)
{
  static OVERLAPPED osWrite = { 0 };
  
  if (reset) {
    memset(&osWrite, 0, sizeof(OVERLAPPED));
    write_prog(0);
  }
  
  return &osWrite;
}

OVERLAPPED* get_ovRead(bool reset)
{
  static OVERLAPPED osRead = { 0 };
  
  if (reset){
    memset(&osRead, 0, sizeof(OVERLAPPED));
    osRead.hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
  }
  
  return &osRead;
}

void outbufsel()
{
  dwNumBytesToSend = 0;
  obuf = OutputBuffer1;
}

void cpybufcnt()
{
  obuf[dwNumBytesToSend++] = SEND_IMMEDIATE;
  FixedNumBytesToSend = dwNumBytesToSend;
  dwNumBytesToSend = 0;
  obuf = OutputBuffer0;
}

FT_STATUS ft_uwrite(FT_HANDLE a, BYTE* b, DWORD c, DWORD* d)
{
  OVERLAPPED osWrite = { 0 };
  
  setMutex(true);
  // hdlWait();
  if (!FT_W32_WriteFile(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent, &osWrite)) {
    if (FT_W32_GetLastError(ftdiHandle) == ERROR_IO_PENDING) {
      // write is delayed so do some other stuff until ... 
      if (!FT_W32_GetOverlappedResult(ftdiHandle, &osWrite, &dwNumBytesSent, true)){
        // qDebug() << "Overlaped Write Error";
      } else { 
        if (dwNumBytesToSend == dwNumBytesSent){
          // qDebug() << "Overlapped Write Ok";
        } else { 
          qDebug() << "Overlapped Write Timeout";
        }
      }
    }
  }
  else {
    // qDebug() << "Overlapped Ok";
  }
  // hdlUnlock();
  setMutex(false);
  
  return FT_OK;
}

FT_STATUS ft_uread(FT_HANDLE a, BYTE* ibuf, DWORD NumBytetoRead, DWORD* d)
{
  OVERLAPPED osRead = { 0 };
  osRead.hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
  
  setMutex(true);
  // hdlWait();
  if (!FT_W32_ReadFile(ftdiHandle, ibuf, NumBytetoRead, &dwNumBytesRead, &osRead)) 
  {
    if (!gathering) {
      if (FT_W32_GetLastError(ftdiHandle) == ERROR_IO_PENDING) 
      { // write is delayed so do some other stuff until ... 
        if (!FT_W32_GetOverlappedResult(ftdiHandle, &osRead, &dwNumBytesRead, true)){
          // qDebug() << "Overlaped Read Error";
        } else { 
          if (NumBytetoRead == dwNumBytesRead)
          { 
            // qDebug() << "Overlaped Read Ok";
          } else{ 
            qDebug() << "Overlaped Read TimeOut";
          } 
        } 
      } 
    }
  } else {
    qDebug() << "Overlaped Read Ok";
  }
  setMutex(false);
  // hdlUnlock();
  
  return FT_OK;
}

//this routine is used to enable SPI device
void SPI_CSEnable()
{
	for(int loop=0;loop<10;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {	
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x01';//set CS high, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}

//this routine is used to disable SPI device
void SPI_CSDisable()
{
  for(int loop=0;loop<10;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x09';//set CS, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}

//this routine is used to enable SPI device
void SPI_SCK_WHIGH(uint8_t lsb)
{
	for(int loop=0;loop<GPIO_MODE_CYCLE;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {	
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x01' | (lsb<<1);//set CS high, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}

void SPI_SCK_WLOW(uint8_t lsb)
{
	for(int loop=0;loop<GPIO_MODE_CYCLE;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {	
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x00' | (lsb<<1);//set CS high, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}


//this routine is used to enable SPI device
void SPI_SCK_RHIGH()
{
	for(int loop=0;loop<GPIO_MODE_CYCLE;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {	
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x01';//set CS high, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}

void SPI_SCK_RLOW()
{
	for(int loop=0;loop<GPIO_MODE_CYCLE;loop++) //one 0x80 command can keep 0.2us, do 5 times to stay in this situation for 1us
  {	
    obuf[dwNumBytesToSend++] = '\x80';//GPIO command for ADBUS
    obuf[dwNumBytesToSend++] = '\x00';//set CS high, MOSI and SCL low
    obuf[dwNumBytesToSend++] = '\x0b';//bit3:CS, bit2:MISO, bit1:MOSI, bit0:SCK
  }
}

void SPI_SCK_READ_BIT()
{
  obuf[dwNumBytesToSend++] = '\x81';//GPIO command for ADBUS
}


//this routine is used to initial SPI interface
BOOL SPI_Initial(FT_HANDLE ftHandle)
{
    DWORD dwCount;
    ftStatus = FT_ResetDevice(ftHandle); //Reset USB device
    BOOL w32status = FALSE;
    FTTIMEOUTS ftTS; 
    
    ftTS.ReadIntervalTimeout = 0;
    ftTS.ReadTotalTimeoutMultiplier = 0;
    ftTS.ReadTotalTimeoutConstant = 1000;
    ftTS.WriteTotalTimeoutMultiplier = 0;
    ftTS.WriteTotalTimeoutConstant = 2000;
    
    //Purge USB receive buffer first by reading out all old data from FT2232H receive buffer
    // ftStatus |= FT_GetQueueStatus(ftHandle, &dwNumInputBuffer); // Get the number of bytes in the FT2232H receive buffer
    // if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
    //     ftStatus |= ft_uread(ftHandle, InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out the data from FT2232H receive buffer

    ftStatus |= FT_SetUSBParameters(ftHandle, 196608, 196608); //Set USB request transfer size
    w32status |= FT_W32_SetupComm(ftHandle, 196608, 196608); //Set USB request transfer size
    ftStatus |= FT_SetChars(ftHandle, false, 0, false, 0); //Disable event and error characters
    ftStatus |= FT_SetTimeouts(ftHandle, 3000, 3000); //Sets the read and write timeouts in 3 sec for the FT2232H
    w32status |= FT_W32_SetCommTimeouts(ftHandle, &ftTS);
    ftStatus |= FT_SetLatencyTimer(ftHandle, 0); //Set the latency timer
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x00); //Reset controller
    ftStatus |= FT_SetBitMode(ftHandle, 0x0, 0x02); //Enable MPSSE mode
    
    if (ftStatus != FT_OK || !w32status)
    {
        qDebug() << "fail on initialize FT4232H device !";
        return false;
    }
    Sleep(50); // Wait 50ms for all the USB stuff to complete and work
    
    //////////////////////////////////////////////////////////////////
    // Synchronize the MPSSE interface by sending bad command ＆xAA＊
    //////////////////////////////////////////////////////////////////
    dwNumBytesToSend = 0;
    obuf[dwNumBytesToSend++] = '\xAA'; //Add BAD command ＆xAA＊
    ftStatus = ft_uwrite(ftHandle, obuf, dwNumBytesToSend, &dwNumBytesSent); // Send off the BAD commands
    dwNumBytesToSend = 0; //Clear output buffer
    
    do{
        ftStatus = FT_GetQueueStatus(ftHandle, &dwNumInputBuffer); // Get the number of bytes in the device input buffer
    }while ((dwNumInputBuffer == 0) && (ftStatus == FT_OK)); //or Timeout
    
    bool bCommandEchod = false;
    ftStatus = ft_uread(ftHandle, InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out the data from input buffer
    
    for (dwCount = 0; dwCount < (dwNumBytesRead - 1); dwCount++) //Check if Bad command and echo command received
    {
        if ((InputBuffer[dwCount] == BYTE('\xFA')) && (InputBuffer[dwCount+1] == BYTE('\xAA')))
        {
            bCommandEchod = true;
            break;
        }
    }
    
    if (bCommandEchod == false)
    {
        printf("fail to synchronize MPSSE with command '0xAA' \n");
        return false; /*Error, can＊t receive echo command , fail to synchronize MPSSE interface;*/
    }
    
    //////////////////////////////////////////////////////////////////
    // Synchronize the MPSSE interface by sending bad command ＆xAB＊
    //////////////////////////////////////////////////////////////////
    //dwNumBytesToSend = 0; //Clear output buffer
    obuf[dwNumBytesToSend++] = '\xAB'; //Send BAD command ＆xAB＊
    ftStatus = ft_uwrite(ftHandle, obuf, dwNumBytesToSend, &dwNumBytesSent); // Send off the BAD commands
    dwNumBytesToSend = 0; //Clear output buffer
    
    do{
        ftStatus = FT_GetQueueStatus(ftHandle, &dwNumInputBuffer); //Get the number of bytes in the device input buffer
    }while ((dwNumInputBuffer == 0) && (ftStatus == FT_OK)); //or Timeout
    
    bCommandEchod = false;
    ftStatus = ft_uread(ftHandle, InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out the data from input buffer
    for (dwCount = 0;dwCount < (dwNumBytesRead - 1); dwCount++) //Check if Bad command and echo command received
    {
        if ((InputBuffer[dwCount] == BYTE('\xFA')) && (InputBuffer[dwCount+1] == BYTE( '\xAB')))
        {
            bCommandEchod = true;
            break;
        }
    }
    
    if (bCommandEchod == false)
    {
        printf("fail to synchronize MPSSE with command '0xAB' \n");
        return false;
        /*Error, can't receive echo command , fail to synchronize MPSSE interface;*/
    }
    
    ////////////////////////////////////////////////////////////////////
    //Configure the MPSSE for SPI communication with EEPROM
    //////////////////////////////////////////////////////////////////
    obuf[dwNumBytesToSend++] = '\x8A'; //Ensure disable clock divide by 5 for 60Mhz master clock
    obuf[dwNumBytesToSend++] = '\x97'; //Ensure turn off adaptive clocking
    obuf[dwNumBytesToSend++] = '\x8D'; //disable 3 phase data clock
    ftStatus = ft_uwrite(ftHandle, obuf, dwNumBytesToSend, &dwNumBytesSent); // Send out the commands
    
    dwNumBytesToSend = 0; //Clear output buffer
    obuf[dwNumBytesToSend++] = '\x80'; //Command to set directions of lower 8 pins and force value on bits set as output
    obuf[dwNumBytesToSend++] = '\x09'; //Set SDA, SCL high, WP disabled by SK, DO at bit ＆＊, GPIOL0 at bit ＆＊
    obuf[dwNumBytesToSend++] = '\x0b'; //Set SK,DO,GPIOL0 pins as output with bit ＊＊, other pins as input with bit ＆＊
    
    // The SK clock frequency can be worked out by below algorithm with divide by 5 set as off
    // SK frequency = 60MHz /((1 + [(1 +0xValueH*256) OR 0xValueL])*2)
    obuf[dwNumBytesToSend++] = '\x86'; //Command to set clock divisor
    obuf[dwNumBytesToSend++] = BYTE(dwClockDivisor & '\xFF'); //Set 0xValueL of clock divisor
    obuf[dwNumBytesToSend++] = BYTE(dwClockDivisor >> 8); //Set 0xValueH of clock divisor
    
    ftStatus = ft_uwrite(ftHandle, obuf, dwNumBytesToSend, &dwNumBytesSent); // Send out the commands
    dwNumBytesToSend = 0; //Clear output buffer
    Sleep(20); //Delay for a while
    
    //Turn off loop back in case
    obuf[dwNumBytesToSend++] = '\x85'; //Command to turn off loop back of TDI/TDO connection
    
    ftStatus = ft_uwrite(ftHandle, obuf, dwNumBytesToSend, &dwNumBytesSent); // Send out the commands
    dwNumBytesToSend = 0; //Clear output buffer
    Sleep(30); //Delay for a while
    printf("SPI initial successful\n");
    
    return true;
}

//this routine is used to write one word data to a random address
bool spi_write_word(WORD address, WORD* bdata, WORD wlen)
{
	WORD bcnt = wlen*2-1;
    dwNumBytesSent=0;
    
    SPI_CSEnable();
    
    // set WRITE flag && send address
    obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
    obuf[dwNumBytesToSend++] = 1;
    obuf[dwNumBytesToSend++] = 0;
    obuf[dwNumBytesToSend++] = ((address >> 8) & 0x7f) & WRFLAG;
    obuf[dwNumBytesToSend++] = address & 0xff;
    
    //send data
    obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
    obuf[dwNumBytesToSend++] = (BYTE) (bcnt & 0xff);
    obuf[dwNumBytesToSend++] = (BYTE) (bcnt >> 8);
	
	for(int i=0; i<wlen; i++) {
		obuf[dwNumBytesToSend++] = (BYTE) (bdata[i] >> 8);    // output high byte
		obuf[dwNumBytesToSend++] = (BYTE) (bdata[i] & 0xff);  // output low byte
	}
    
	SPI_CSDisable();
	
    ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
	dwNumBytesToSend = 0; //Clear output buffer
    
    return ftStatus;
}

//this routine is used to read one word data from a random address
bool spi_read_word(WORD address, WORD* bdata, WORD wlen)
{
	WORD bcnt = wlen*2-1;
    dwNumBytesSent=0;
    SPI_CSEnable();
    
    // set READ flag && send address
    obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
    obuf[dwNumBytesToSend++] = 1;
    obuf[dwNumBytesToSend++] = 0;
    obuf[dwNumBytesToSend++] = ((address >> 8) & 0x7f) | RDFLAG;
    obuf[dwNumBytesToSend++] = address & 0xff;
    
    //read data
    obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_IN;
    obuf[dwNumBytesToSend++] = (BYTE) (bcnt & 0xff);
    obuf[dwNumBytesToSend++] = (BYTE) (bcnt >> 8);
    
    SPI_CSDisable();
    ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
    dwNumBytesToSend = 0; //Clear output buffer
	
    ftStatus = ft_uread(ftdiHandle, InputBuffer, (wlen<<1), &dwNumBytesRead);//Read 2 bytes from device receive buffer
	for(int i=0; i<wlen; i++) {
		int idx = i<<1;
		bdata[i] = (InputBuffer[idx] << 8) + InputBuffer[idx+1];
	}
    
    return ftStatus;
}

//this routine is used to write one word data to a random address
bool spi_write_byte(BYTE address, BYTE* bdata, WORD wlen)
{
  WORD bcnt = wlen-1;
  dwNumBytesSent=0;
  
  SPI_CSEnable();
  
  // set WRITE flag && send address
  obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
  obuf[dwNumBytesToSend++] = 0;
  obuf[dwNumBytesToSend++] = 0;
  obuf[dwNumBytesToSend++] = (address & 0x7f) & WRFLAG;
  
  //send data
  obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
  obuf[dwNumBytesToSend++] = (BYTE) (bcnt & 0xff);
  obuf[dwNumBytesToSend++] = (BYTE) (bcnt >> 8);
	
	for(int i=0; i<wlen; i++) {
		obuf[dwNumBytesToSend++] = (BYTE) (bdata[i] & 0xff);
	}
    
	SPI_CSDisable();
	
  ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
	dwNumBytesToSend = 0; //Clear output buffer
    
  return ftStatus;
}

//this routine is used to read one word data from a random address
bool spi_read_byte(BYTE address, BYTE* bdata, WORD wlen)
{
	WORD bcnt = wlen-1;
  dwNumBytesSent=0;
  SPI_CSEnable();
  
  // set READ flag && send address
  obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_OUT;
  obuf[dwNumBytesToSend++] = 0;
  obuf[dwNumBytesToSend++] = 0;
  obuf[dwNumBytesToSend++] = (address & 0x7f) | RDFLAG;
  
  //read data
  obuf[dwNumBytesToSend++] = MSB_FALLING_EDGE_CLOCK_BYTE_IN;
  obuf[dwNumBytesToSend++] = (BYTE) (bcnt & 0xff);
  obuf[dwNumBytesToSend++] = (BYTE) (bcnt >> 8);
  
  SPI_CSDisable();
  ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
  dwNumBytesToSend = 0; //Clear output buffer
	
  ftStatus = ft_uread(ftdiHandle, InputBuffer, wlen, &dwNumBytesRead);//Read 2 bytes from device receive buffer
  
	for(int i=0; i<wlen; i++) {
		int idx = i;
		bdata[i] = InputBuffer[idx];
	}
    
    return ftStatus;
}

//this routine is used to write one word data to a random address
bool gpio_write_byte(BYTE address, BYTE* bdata, WORD wlen)
{
	BYTE bcnt = wlen-1;
  dwNumBytesSent=0;
  BYTE cmd = (address & 0x7f) & WRFLAG;
  SPI_CSEnable();
  
  for (int i=0; i<8; i++) {
    SPI_SCK_WLOW((cmd >> (7-i)) & 0x01);
    SPI_SCK_WHIGH((cmd >> (7-i)) & 0x01);
  }
  
  for (int j=0; j<wlen; j++){
    BYTE wdat = bdata[j];
    for (int i=0; i<8; i++) {
      SPI_SCK_WLOW((wdat >> (7-i)) & 0x01);
      SPI_SCK_WHIGH((wdat >> (7-i)) & 0x01);
    }
  }
    
	SPI_CSDisable();
	
  if (!gathering) {
    ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
    dwNumBytesToSend = 0; //Clear output buffer
  }
    
  return ftStatus;
}

//this routine is used to read one word data from a random address
bool gpio_read_byte(BYTE address, BYTE* bdata, WORD wlen)
{
	BYTE bcnt = wlen-1;
  dwNumBytesSent = 0;
  BYTE cmd = (address & 0x7f) | RDFLAG;
  WORD rcnt = 0;
  
  SPI_CSEnable();
  
  for (int i=0; i<8; i++) {
    SPI_SCK_WLOW((cmd >> (7-i)) & 0x01);
    SPI_SCK_WHIGH((cmd >> (7-i)) & 0x01);
  }
  
  for (int j=0; j<wlen; j++){
    for (int i=0; i<8; i++) {
      SPI_SCK_RLOW();
      SPI_SCK_READ_BIT();
      SPI_SCK_RHIGH();
    }
  }
  
  SPI_CSDisable();
  
  // grlen += wlen;
  
  if (!gathering) {
    ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
    dwNumBytesToSend = 0; //Clear output buffer
    ftStatus = ft_uread(ftdiHandle, InputBuffer, wlen*8, &dwNumBytesRead);//Read 2 bytes from device receive buffer
  
    for(int i=0; i<wlen; i++) {
      BYTE rddat = 0;
      for (int k=0; k<8; k++) {
        rddat |= (((InputBuffer[i*8+k] & 0x04) >> 2) << (7-k));
      }
      bdata[i] = rddat;
    }
  }
    
  return ftStatus;
}

void spi_queue_ctrl(bool onoff)
{
  gathering = onoff;
}

void spi_queue_write()
{
  static int wcnt = 0;
  
  int loop = 0;
  setMutex(true);
  OVERLAPPED* lposWrite = get_ovWrite(false);
  if (write_prog(2)) {
    do {
      FT_W32_GetOverlappedResult(ftdiHandle, lposWrite, &dwNumBytesSent, false);
      qDebug() << "[WR] check dwNumByteSent " << dwNumBytesSent << " Fixed length : " << FixedNumBytesToSend;
    } while (dwNumBytesSent < FixedNumBytesToSend);
  }
  
  write_prog(1);
  qDebug() << "out !!";
  
  // qDebug() << "[WR] dwNumByteSent " << dwNumBytesSent << "  ,    " << wcnt++;
  lposWrite = get_ovWrite(true);
  
  FT_W32_WriteFile(ftdiHandle, OutputBuffer1, FixedNumBytesToSend, &dwNumBytesSent, lposWrite);
  setMutex(false);
}

BYTE* spi_queue_read(BYTE* bdata)
{
  
  OVERLAPPED* lposRead = get_ovRead(false); 
  static int rcnt = 0;
  static BYTE* prevbuf;
  BYTE* returnval;
  
  setMutex(true);
  FT_W32_GetOverlappedResult(ftdiHandle, lposRead, &dwNumBytesRead, true);
  setMutex(false);
  
  if (dwNumBytesRead >= SPI_QUEUE_BUFF_LEN) {
    returnval = prevbuf;
  }
  else {
    returnval = NULL;
  }
  
  qDebug() << "[RD] dwNumByteRead " << dwNumBytesRead << "  ,      " << rcnt++ << " pt = " << returnval;

  setMutex(true);
  lposRead = get_ovRead(true);
  FT_W32_ReadFile(ftdiHandle, bdata, SPI_QUEUE_BUFF_LEN, &dwNumBytesRead, lposRead);
  setMutex(false);
  prevbuf = bdata;
  
  return returnval;
}

WORD spi_queue_rcnt()
{
  return grlen;
}

void gpio_set_high_byte()
{
  obuf[dwNumBytesToSend++] = '\x82';
  obuf[dwNumBytesToSend++] = 0;
  obuf[dwNumBytesToSend++] = 0;
}

void gpio_b5_wait_low()
{
  obuf[dwNumBytesToSend++] = WAIT_ON_IO_LOW;
}

void gpio_b5_wait_high()
{
  obuf[dwNumBytesToSend++] = WAIT_ON_IO_HIGH;
}

bool gpio_read_bit(BYTE bitpos)
{
  obuf[dwNumBytesToSend++] = SEND_IMMEDIATE;
  
  ftStatus = ft_uwrite(ftdiHandle, obuf, dwNumBytesToSend, &dwNumBytesSent);//send out MPSSE command to MPSSE engine
  dwNumBytesToSend = 0; //Clear output buffer
  ftStatus = ft_uread(ftdiHandle, InputBuffer, 1, &dwNumBytesRead);//Read 2 bytes from device receive buffer
  
  return ((InputBuffer[0] >> bitpos) & 0x01)? true : false;
}

bool ftdi_connect()
{
  DWORD numDevs;
  FT_DEVICE_LIST_INFO_NODE *devInfo;
  WCHAR Buf[64];

  ftdi_disconnect();

  ftStatus = FT_CreateDeviceInfoList(&numDevs);
  
  if (ftStatus == FT_OK)
    qDebug() << "Number of devices is " << numDevs;
  else
    return FALSE;
  
  if (numDevs > 0) {
    // allocate storage for list based on numDevs
    devInfo = (FT_DEVICE_LIST_INFO_NODE*)malloc(sizeof(FT_DEVICE_LIST_INFO_NODE)*numDevs);
    // get the device information list
    ftStatus = FT_GetDeviceInfoList(devInfo,&numDevs);
    if (ftStatus == FT_OK) {
      for (int i = 0; i < numDevs; i++) {
        // qDebug() << "Dev :" << i;
        // qDebug() << "Flags=0x" << devInfo[i].Flags;
        // qDebug() << "Type=0x" << devInfo[i].Type;
        // qDebug() << "ID=0x" << devInfo[i].ID;
        // qDebug() << "LocId=0x" << devInfo[i].LocId;
        // qDebug() << "SerialNumber = " << devInfo[i].SerialNumber;
        // qDebug() << "Description = " << devInfo[i].Description;
        // qDebug() << "ftHandle=0x" << (int)devInfo[i].ftHandle;
      }
    }
  }
  else {
    qDebug() << "Device number is under 0 ";
    return FALSE;
  }

  // ftStatus = FT_Open(0, &ftdiHandle);
  ftStatus = FT_ListDevices(0, Buf, FT_LIST_BY_INDEX | FT_OPEN_BY_SERIAL_NUMBER);
  ftdiHandle = FT_W32_CreateFile(Buf, GENERIC_READ | GENERIC_WRITE, 0, 0,
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FT_OPEN_BY_SERIAL_NUMBER, 0);
                
  if (ftdiHandle == INVALID_HANDLE_VALUE)
  {
      qDebug() << "Can't open FT4232H device";
      return FALSE;
  }
  else // Port opened successfully
    printf("Successfully open FT4232H device! ");
      
  if(SPI_Initial(ftdiHandle) == TRUE)
  {
      byte ReadByte = 0;
      
      //Purge USB received buffer first before read operation
      ftStatus = FT_GetQueueStatus(ftdiHandle, &dwNumInputBuffer); // Get the number of bytes in the device receive buffer
      if ((ftStatus == FT_OK) && (dwNumInputBuffer > 0))
          ft_uread(ftdiHandle, InputBuffer, dwNumInputBuffer, &dwNumBytesRead); //Read out all the data from receive buffer
      
  }
  
  return TRUE;
}

bool ftdi_disconnect()
{
  bool ft_ok = false;
	ft_ok =  FT_Close(ftdiHandle);
  ftdiHandle = NULL;
  // qDebug() << " ft close error code : " << ft_ok;
  return ft_ok;
}
