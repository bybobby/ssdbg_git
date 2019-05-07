///////////////////////////////////////////////////////////////////////////////
//                                                                          
//  PC serial port connection object for  event-driven programs                     
//                                        
//  Copyright @ 2001-2002
//                                                                          
// ----------------------------------------------------------------------------- 
//                                                                          
//  Filename    : SerialPort.cpp
//  Author      : Thierry Schneider(thierry@tetraedre.com)
//  Created     : 2000/04/04
//  Modified    : 2002/06/22
//  Plateform   : Windows 95, 98, NT, 2000, XP (Win32)                         
//
// ----------------------------------------------------------------------------- 
//                                                                          
//  This software is given without any warranty. It can be distributed      
//  free of charge as long as this header remains, unchanged.               
//                                                                          
// ----------------------------------------------------------------------------- 
//                                                                          
// 01.04.24      Comments added                                             
// 01.04.28      Bug 010427 corrected. OnDisconnectedManager was not        
//                initialized                                               
// 01.04.28      connect() function prototype modified to handle 7-bit      
//                communication                                             
// 01.04.29      "ready" field added to remove a bug that occured during    
//                 reconnect (event fp_SerialCallback pointers cleared)               
//                 I removed the "delete" in runThreadStart     
//                 because it was destroying the object even if we would    
//                 use it again                                             
//                                                                          
// 02.01.30      Version 2.0 of the serial event object                     
//                                                                          
//                                                                          
// 02.06.22      - wait for the thread termination before                   
//                 quiting or restarting                                    
//               - "owner" field added to be able to call C++ object from   
//                  the event fp_SerialCallback routine                               
//               - Correction of a bug that occured when receiving data     
//                 (setting twice the SIG_READ_DONE event)                  
//                                                                          
//                                                                          
// ----------------------------------------------------------------------------- 
//                                                                          
//    Note to Visual C++ users:  Don't forget to compile with the           
//     "Multithreaded" option in your project settings                      
//                                                                          
//         See   Project settings                                           
//                   |                                                      
//                   *--- C/C++                                             
//                          |                                               
//                          *--- Code generation                            
//                                       |                                  
//                                       *---- Use run-time library         
//                                                     |                    
//                                                     *---- Multithreaded  
//                                                                          
////////////////////////////////////////////////////////////////////////////////

#define STRICT
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>
#include <conio.h>
#include <windows.h>
#include <atltrace.h>

#include "serial/serialport.h"

#define SIG_POWER_DOWN     0
#define SIG_READER         1
#define SIG_READ_DONE      2    // data received has been read
#define SIG_WRITER         3
#define SIG_DATA_TO_TX     4    // data waiting to be sent
#define SIG_MODEM_EVENTS   5
#define SIG_MODEM_CHECKED  6

// #define DEBUG_EVENTS

// #define DTRACE(...) ATL::CTraceFileAndLineInfo("", 0)(0x80000, 0, __VA_ARGS__)

void runThreadStart(void *arg);

typedef unsigned (WINAPI *PBEGINTHREADEX_THREADFUNC) (LPVOID lpThreadParameter);
typedef unsigned *PBEGINTHREADEX_THREADID;

using std::string;
using std::wstring;
using namespace coms;

inline wstring
_prefix_port_if_needed(const wstring &input)
{
  static wstring windows_com_port_prefix = L"\\\\.\\";
  if (input.compare(windows_com_port_prefix) != 0)
  {
    return windows_com_port_prefix + input;
  }
  return input;
}

/**
    This function is not part of the CSerialEvt object. It is simply used
    to start the thread from an external point of the object.
*/
void runThreadStart(void *arg)
{
  class CSerialEvt *p_CSerialEvt = (CSerialEvt *) arg;
    
  if (p_CSerialEvt!=0)
    p_CSerialEvt->run();
}

// Constructor
CSerialEvt::CSerialEvt()
{
  ready         = false;
  parityMode    = SERIAL_PARITY_NONE;
  m_COMport     = "";
  rate          = 0;
  threadid      = 0;
  h_Serial      = INVALID_HANDLE_VALUE;
  h_Thread      = 0;
  owner         = 0;
  b_TX_inprog   = 0;
  b_RX_inprog   = 0;
  max_rx_size   = 1;
  tx_size       = 0;
  received_size = 0;
  check_modem   = false;

  fp_SerialCallback = 0;

  // creating Events for the different sources
  for (int i=0; i<SERIAL_SIGNAL_NBR; i++)
  {
    if ((i==SIG_READER) || (i==SIG_WRITER) || (i==SIG_MODEM_EVENTS))
      h_SerialEvt[i] = CreateEvent(NULL, TRUE, FALSE, NULL);  // Manual Reset
    else
      h_SerialEvt[i] = CreateEvent(NULL, FALSE, FALSE, NULL); // Auto reset
  }
}

// Distructor
CSerialEvt::~CSerialEvt()
{
  if (h_Thread!=0)
    WaitForSingleObject(h_Thread, 2000);
  
  h_Thread = 0;

  for (int i=0; i<SERIAL_SIGNAL_NBR; i++)         // deleting the events
  {
    if (h_SerialEvt[i]!=INVALID_HANDLE_VALUE)
      CloseHandle(h_SerialEvt[i]);
    
      h_SerialEvt[i] = INVALID_HANDLE_VALUE;
  }

  if (h_Serial!=INVALID_HANDLE_VALUE)
    CloseHandle(h_Serial);
  
  h_Serial = INVALID_HANDLE_VALUE;
}


void CSerialEvt::disconnect(void)
{
  ready = false;
  SetEvent(h_SerialEvt[SIG_POWER_DOWN]);

  if (h_Thread!=0)
    WaitForSingleObject(h_Thread, 2000);
  
  h_Thread = 0;
}


// Serial port, file and overlapped structures initialization
int CSerialEvt::connect (const string &port, int  rate_arg,  int parity_arg,
                             char ByteSize , bool modem_events)
{
  int  erreur;
  DCB  dcb;
  int  i;
  bool hard_handshake;
  COMMTIMEOUTS cto = { 0, 0, 0, 0, 0 };

  // check the serial handle
  if (h_Serial!=INVALID_HANDLE_VALUE)
    CloseHandle(h_Serial);
  
  h_Serial = INVALID_HANDLE_VALUE;
  
  if (!port.empty())
  {
    rate        = rate_arg;
    parityMode  = parity_arg;
    check_modem = modem_events;
    
    erreur      = 0;
    ZeroMemory(&ovReader   ,sizeof(ovReader)   );  // clearing the overlapped
    ZeroMemory(&ovWriter   ,sizeof(ovWriter)   );
    ZeroMemory(&ovWaitEvent,sizeof(ovWaitEvent));
    memset(&dcb,0,sizeof(dcb));
    
    // set DCB to configure the serial port
    dcb.DCBlength       = sizeof(dcb);                   
    
    /* ---------- Serial Port Config ------- */
    dcb.BaudRate        = rate;
    
    switch(parityMode)
    {
      case SERIAL_PARITY_NONE:
        dcb.Parity      = NOPARITY;
        dcb.fParity     = 0;
      break;
      case SERIAL_PARITY_EVEN:
        dcb.Parity      = EVENPARITY;
        dcb.fParity     = 1;
      break;
      case SERIAL_PARITY_ODD:
        dcb.Parity      = ODDPARITY;
        dcb.fParity     = 1;
      break;
    }
  
    dcb.StopBits        = ONESTOPBIT;
    dcb.ByteSize        = (BYTE) ByteSize;
  
    // -------------- modified 2005-05-16 ---------------
    dcb.fInX            = FALSE;
    dcb.fOutX           = FALSE;
  
    hard_handshake = false;
    
    if (hard_handshake)
    {
        dcb.fOutxDsrFlow    = TRUE;
        dcb.fOutxCtsFlow    = TRUE;
        dcb.fRtsControl     = RTS_CONTROL_HANDSHAKE;
        dcb.fDtrControl     = DTR_CONTROL_HANDSHAKE;
    }
    else
    {
      dcb.fOutxDsrFlow    = FALSE;
      dcb.fOutxCtsFlow    = FALSE;
        
      // dcb.fRtsControl     = RTS_CONTROL_ENABLE;
      // dcb.fDtrControl     = DTR_CONTROL_ENABLE;
      dcb.fRtsControl     = RTS_CONTROL_DISABLE;
      dcb.fDtrControl     = DTR_CONTROL_DISABLE;
    }
  
    /* ----------------- misc parameters ----- */
    dcb.fErrorChar      = 0;
    dcb.fBinary         = 1;
    dcb.fNull           = 0;
    dcb.fAbortOnError   = 0;
    dcb.wReserved       = 0;
    dcb.XonLim          = 2;
    dcb.XoffLim         = 4;
    dcb.XonChar         = 0x13;
    dcb.XoffChar        = 0x19;
    dcb.EvtChar         = 0;
    
    wstring port_w = wstring(port.begin(), port.end());
    wstring port_with_prefix = _prefix_port_if_needed(port_w);
    LPCWSTR lp_port = port_with_prefix.c_str();

    h_Serial    = CreateFile(lp_port, GENERIC_READ | GENERIC_WRITE,
              0, NULL, OPEN_EXISTING, FILE_FLAG_OVERLAPPED ,NULL);
              // opening serial port
  
    ovReader.hEvent    = h_SerialEvt[SIG_READER];
    ovWriter.hEvent    = h_SerialEvt[SIG_WRITER];
    ovWaitEvent.hEvent = h_SerialEvt[SIG_MODEM_EVENTS];
  
    if (h_Serial != INVALID_HANDLE_VALUE) {
      if (check_modem) {
        if(!SetCommMask(h_Serial, EV_RING | EV_RLSD))
          erreur = 1;
      }
      else {
        if(!SetCommMask(h_Serial, 0))
          erreur = 1;
      }
  
      // set timeouts
      if(!SetCommTimeouts(h_Serial,&cto))
        erreur = 2;
  
      // set DCB
      if(!SetCommState(h_Serial,&dcb))
        erreur = 4;
    }
    else
      erreur = 8;
  }
  else
    erreur = 16;
  
  
  /* --------------------------------------------- */
  for (i=0; i<SERIAL_SIGNAL_NBR; i++)
  {
    if (h_SerialEvt[i]==INVALID_HANDLE_VALUE)
      erreur = 32;
  }
  
  /* --------------------------------------------- */
  if (erreur!=0)
  {
    CloseHandle(h_Serial);
    h_Serial = INVALID_HANDLE_VALUE;
  }
  else
  {
    // start thread
    h_Thread = (HANDLE) _beginthreadex(NULL,0,
              (PBEGINTHREADEX_THREADFUNC) runThreadStart,
              this, 0, &threadid);
  }
  
  return(erreur);
}

void CSerialEvt::setManager(utdefCallback marg)
{
  fp_SerialCallback = marg;
}

void CSerialEvt::setRxSize(int size)
{
  max_rx_size = size;
  if (max_rx_size>SERIAL_MAX_RX)
    max_rx_size = SERIAL_MAX_RX;
}

char* CSerialEvt::getDataInBuffer(void)
{
  return(rxBuffer);
}

int CSerialEvt::getDataInSize(void)
{
  return(received_size);
}

void CSerialEvt::dataHasBeenRead(void)
{
  SetEvent(h_SerialEvt[SIG_READ_DONE]);
}

int CSerialEvt::getNbrOfBytes(void)
{
  struct _COMSTAT status;
  int n;
  unsigned long   etat;

  n = 0;

  if (h_Serial!=INVALID_HANDLE_VALUE)
  {
    ClearCommError(h_Serial, &etat, &status);
    n = status.cbInQue;
  }
  return(n);
}

void CSerialEvt::sendData (char *buffer, int size)
{
  if ((!b_TX_inprog) && (size<SERIAL_MAX_TX) && (buffer!=0))
  {
    b_TX_inprog = 1;
    memcpy(txBuffer, buffer, size);
    tx_size = size;
    SetEvent(h_SerialEvt[SIG_DATA_TO_TX]);
  }
}

void CSerialEvt::OnEvent (unsigned long events)
{
  unsigned long ModemStat;

  GetCommModemStatus(h_Serial, &ModemStat);

  if ((events & EV_RING)!=0)
  {
    if ((ModemStat &  MS_RING_ON)!= 0)
    {
      if (fp_SerialCallback!=0)
        fp_SerialCallback((uint64_t) this, SERIAL_RING);
    }
  }

  if ((events & EV_RLSD)!=0)
  {
    if ((ModemStat &  MS_RLSD_ON)!= 0)
    {
      if (fp_SerialCallback!=0)
        fp_SerialCallback((uint64_t) this, SERIAL_CD_ON);
    }
    else
    {
      if (fp_SerialCallback!=0)
        fp_SerialCallback((uint64_t) this, SERIAL_CD_OFF);
    }
  }
}

/**
 this function is the main loop of the CSerialEvt object. There is a
 do while() loop executed until either an error or a PowerDown is 
 received.
 this is not a busy wait since we use the WaitForMultipleObject function
*/

/* * /

/* */

void CSerialEvt::run(void)
{
  bool          done;
  long          status;
  unsigned long read_nbr, result_nbr;
  char          success;

  ready                   = true;
  done                    = false;
  b_TX_inprog          = 0;
  b_RX_inprog          = 0;
  WaitCommEventInProgress = 0;

  if (fp_SerialCallback!=0)
    fp_SerialCallback((uint64_t) this, SERIAL_CONNECTED);

  GetLastError();               // just to clear any pending error
  SetEvent(h_SerialEvt[SIG_READ_DONE]);
  if (check_modem)
    SetEvent(h_SerialEvt[SIG_MODEM_CHECKED]);

  /* ----------------------------------------------------------- */
  while(!done)
  {
    /* ------------------------------------------------------------------ */
    /*                                                                    */
    /*                                                                    */
    /*                                                                    */
    /*                          Waiting  for signals                      */
    /*                                                                    */
    /*                                                                    */
    /*                                                                    */
    /* ------------------------------------------------------------------ */
    // Main wait function. Waiting for something to happen.
    // This may be either the completion of a Read or a Write or
    // the reception of modem events, Power Down, new Tx
    //
    status = WaitForMultipleObjects(SERIAL_SIGNAL_NBR, h_SerialEvt,
                                    FALSE, INFINITE);
  
    // processing answer to filter other failures
    status = status - WAIT_OBJECT_0;
    
    if ((status<0) || (status>=SERIAL_SIGNAL_NBR))
      done=true;   // error
    
    else
    {
      /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
      /* ++++++++++++++++++++ EVENT DISPATCHER ++++++++++++++++++ */
      /* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */
      switch(status)
      {
        /* ######################################################## */
        case SIG_POWER_DOWN:
          // receiving a POWER down signal. Stopping the thread
          done = true;
        break;
        /* ######################################################## */
        /* #                                                      # */
        /* #                                                      # */
        /* #                       RX                             # */
        /* #                                                      # */
        /* #                                                      # */
        /* ######################################################## */
        case SIG_READ_DONE:
          // previous reading is finished
          // I start a new one here
          if (!b_RX_inprog)
          {
            // locking reading
            b_RX_inprog = 1;
            // starting a new read
            memset(rxBuffer, 0x00, sizeof(char)*SERIAL_MAX_RX);
            success = (char) ReadFile(h_Serial,&rxBuffer,
                                      max_rx_size,&read_nbr,&ovReader);
            if (!success)
            {
              // failure
              if(GetLastError() != ERROR_IO_PENDING )
              {
                // real failure => quiting
                done = true;
#ifdef DEBUG_EVENTS
                // DTRACE("Readfile error (not pending)\n");
#endif DEBUG_EVENTS
              }
#ifdef DEBUG_EVENTS
              else
                // DTRACE("ReadFile pending\n");
#endif DEBUG_EVENTS
            }
#ifdef DEBUG_EVENTS
            else
            {
              // I make nothing here since the overlapped
              // will be signaled anyway, so I'll make
              // the processing there
              // DTRACE("ReadFile immediate success\n");
            }
#endif
          }
        break;
        
        /* ######################################################## */
        case SIG_READER:
          // reading the result of the terminated read
          if (GetOverlappedResult(h_Serial, &ovReader, &result_nbr, FALSE))
          {
#ifdef DEBUG_EVENTS
            // DTRACE("ReadFile => GetOverlappedResult done\n");
#endif DEBUG_EVENTS
            // no error => OK
            // Read operation completed successfully
            ResetEvent(h_SerialEvt[SIG_READER]);
            // Write operation completed successfully
            received_size  = result_nbr;
            b_RX_inprog = 0; // read has ended
            // if incoming data, I process them
            if ((result_nbr!=0) &&(fp_SerialCallback!=0))
              fp_SerialCallback((uint64_t) this, SERIAL_DATA_ARRIVAL);
            // I automatically restart a new read once the
            // previous is completed.
            //SetEvent(h_SerialEvt[SIG_READ_DONE]);
            // BUG CORRECTION 02.06.22
          }
          else
          {
            // GetOverlapped didn't succeed !
            // What's the reason ?
            if(GetLastError()!= ERROR_IO_PENDING )
              done = 1;  // failure
          }
        break;
        
        /* ######################################################## */
        /* #                                                      # */
        /* #                                                      # */
        /* #                       TX                             # */
        /* #                                                      # */
        /* #                                                      # */
        /* ######################################################## */
        case SIG_DATA_TO_TX:
          // Signal asserted that there is a new valid message
          // in the "txBuffer" variable
          // sending data to the port
          success = (char) WriteFile(h_Serial, txBuffer, tx_size,
                                &result_nbr, &ovWriter);
          if (!success)
          {
            // ouups, failure
            if(GetLastError() != ERROR_IO_PENDING )
            {
              // real failure => quiting
              done = true;
#ifdef DEBUG_EVENTS
              // DTRACE("WriteFile error (not pending)\n");
#endif DEBUG_EVENTS
            }
#ifdef DEBUG_EVENTS
            else
              // DTRACE("WriteFile pending\n");
#endif DEBUG_EVENTS
          }
#ifdef DEBUG_EVENTS
          else
          {
            // I make nothing here since the overlapped
            // will be signaled anyway, so I'll make
            // the processing there
            // DTRACE("WriteFile immediate success\n");
          }
#endif
        break;
        /* ######################################################## */
        case SIG_WRITER:
          // WriteFile has terminated
          // checking the result of the operation
          if (GetOverlappedResult(h_Serial, &ovWriter,
                              &result_nbr, FALSE))
          {
            // Write operation completed successfully
            ResetEvent(h_SerialEvt[SIG_WRITER]);
            // further write are now allowed
            b_TX_inprog = 0;
            // telling it to the fp_SerialCallback
            if (fp_SerialCallback!=0)
              fp_SerialCallback((uint64_t) this, SERIAL_DATA_SENT);
          }
          else
          {
            // GetOverlapped didn't succeed !
            // What's the reason ?
            if(GetLastError() != ERROR_IO_PENDING )
              done = 1;  // failure
          }
        break;
        
        /* ######################################################## */
        /* #                                                      # */
        /* #                                                      # */
        /* #               MODEM_EVENTS EVENTS                    # */
        /* #                                                      # */
        /* #                                                      # */
        /* ######################################################## */
        case SIG_MODEM_CHECKED:
          if ((!WaitCommEventInProgress) && check_modem)
          // if no wait is in progress I start a new one
          {            
            WaitCommEventInProgress=1;
            success = (char) WaitCommEvent(h_Serial,&dwCommEvent,
                                              &ovWaitEvent);
            // reading one byte only to have immediate answer on each byte
            if (!success)
            {
              // ouups, failure
              if(GetLastError() != ERROR_IO_PENDING )
              {
                // real failure => quiting
                done = true;
#ifdef DEBUG_EVENTS
                // DTRACE("WaitCommEvent error (not pending)\n");
#endif DEBUG_EVENTS
              }
#ifdef DEBUG_EVENTS
              else
                // DTRACE("WaitCommEvent pending\n");
#endif DEBUG_EVENTS
            }
#ifdef DEBUG_EVENTS
            else
            {
              // I make nothing here since the overlapped
              // will be signaled anyway, so I'll make
              // the processing there
              // DTRACE("WaitCommEvent immediate success\n");
            }
#endif
          }
        break;
        
        /* ######################################################## */
        case SIG_MODEM_EVENTS:
          // reading the result of the terminated wait
          if (GetOverlappedResult(h_Serial, &ovWaitEvent,
                &result_nbr, FALSE))
          {
            // Wait operation completed successfully
            ResetEvent(h_SerialEvt[SIG_MODEM_EVENTS]);
            WaitCommEventInProgress = 0;
            // if incoming data, I process them
            OnEvent(dwCommEvent);
            // automatically starting a new check
            SetEvent(h_SerialEvt[SIG_MODEM_CHECKED]);
          }
          else
          {
            // GetOverlapped didn't succeed !
            // What's the reason ?
            if(GetLastError() != ERROR_IO_PENDING )
              done = 1;  // failure
          }
        break;
          /* ######################################################## */
      }
    }
  };

  // --------------------- Disconnecting ----------------
  ready = false;
  
  if (h_Serial!=INVALID_HANDLE_VALUE)
    CloseHandle(h_Serial);
  
  h_Serial = INVALID_HANDLE_VALUE;

  if (fp_SerialCallback!=0)
    fp_SerialCallback((uint64_t) this, SERIAL_DISCONNECTED);
}

