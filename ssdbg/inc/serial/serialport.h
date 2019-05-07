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
//                 I removed the "delete" in CSerialEvt_thread_start     
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
//                                                                          
//                                                                          
///////////////////////////////////////////////////////////////////////////////


#ifndef __SERIAL_EVT_H__
#define __SERIAL_EVT_H__

#include <stdio.h>
#include <windows.h>
#include <limits>
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include <exception>
#include <stdexcept>

#define THROW(exceptionClass, message) throw exceptionClass(__FILE__, \
__LINE__, (message) )

#define SERIAL_PARITY_NONE 0
#define SERIAL_PARITY_ODD  1
#define SERIAL_PARITY_EVEN 2

#define SERIAL_CONNECTED         0
#define SERIAL_DISCONNECTED      1
#define SERIAL_DATA_SENT         2
#define SERIAL_DATA_ARRIVAL      3
#define SERIAL_RING              4
#define SERIAL_CD_ON             5
#define SERIAL_CD_OFF            6

typedef void (*utdefCallback) (uint64_t object, uint32_t event);

// #ifndef __BORLANDC__
// #define bool  BOOL 
// #define true  TRUE
// #define false FALSE
// #endif

#define SERIAL_SIGNAL_NBR   7           // number of events in the thread
#define SERIAL_MAX_RX       65536       // Input buffer max size
#define SERIAL_MAX_TX       65536       // output buffer max size

namespace coms {
/* -------------------------------------------------------------------- */
/* -----------------------------  CSerialEvt  ------------------------- */
/* -------------------------------------------------------------------- */
class CSerialEvt
{
protected:
    bool          ready;
    bool          check_modem;
    std::string   m_COMport;                           // port name "com1",...
    int           rate;                               // baudrate
    int           parityMode;

    HANDLE        h_SerialEvt[SERIAL_SIGNAL_NBR];     // events to wait on
    unsigned int  threadid;                           // ...
    HANDLE        h_Serial;                           // ...
    HANDLE        h_Thread;                           // ...
    OVERLAPPED    ovReader;                           // Overlapped structure for ReadFile
    OVERLAPPED    ovWriter;                           // Overlapped structure for WriteFile
    OVERLAPPED    ovWaitEvent;                        // Overlapped structure for WaitCommEvent
    char          b_TX_inprog;                        // BOOL indicating if a WriteFile is in progress
    char          b_RX_inprog;                        // BOOL indicating if a ReadFile is in progress
    char          WaitCommEventInProgress;
    int           max_rx_size;
    int           received_size;
    char          rxBuffer[SERIAL_MAX_RX];
    char          txBuffer[SERIAL_MAX_TX];
    int           tx_size;
    DWORD         dwCommEvent;                       // to store the result of the wait

    utdefCallback fp_SerialCallback;

    void          OnCharArrival    (char c);
    void          OnEvent          (unsigned long events);

    // ++++++++++++++++++++++++++++++++++++++++++++++
    // .................. EXTERNAL VIEW .............
    // ++++++++++++++++++++++++++++++++++++++++++++++
public:
    void         *owner;                // do what you want with this
    void          run          (void);
                  CSerialEvt();
                 ~CSerialEvt();
    int           connect          (const std::string &port, int rate, int parity,
                                    char ByteSize, bool modem_events);

    void          setManager       (utdefCallback fp_SerialCallback);
    void          setRxSize        (int size);
    void          sendData         (char *buffer, int size);
    int           getNbrOfBytes    (void);
    int           getDataInSize    (void);
    char*         getDataInBuffer  (void);
    void          dataHasBeenRead  (void);
    void          disconnect       (void);
};


/*!
* Structure that describes a serial device.
*/
struct PortInfo {

  /*! Address of the serial port (this can be passed to the constructor of Serial). */
  std::string port;

  /*! Human readable description of serial device if available. */
  std::string description;

  /*! Hardware ID (e.g. VID:PID of USB serial devices) or "n/a" if not available. */
  std::string hardware_id;

};

/* Lists the serial ports available on the system
*
* Returns a vector of available serial ports, each represented
* by a serial::PortInfo data structure:
*
* \return vector of serial::PortInfo.
*/
std::vector<PortInfo>
  list_ports();

/*!
* Enumeration defines the possible bytesizes for the serial port.
*/
typedef enum {
  fivebits = 5,
  sixbits = 6,
  sevenbits = 7,
  eightbits = 8
} bytesize_t;

/*!
* Enumeration defines the possible parity types for the serial port.
*/
typedef enum {
  parity_none = 0,
  parity_odd = 1,
  parity_even = 2,
  parity_mark = 3,
  parity_space = 4
} parity_t;

/*!
* Enumeration defines the possible stopbit types for the serial port.
*/
typedef enum {
  stopbits_one = 1,
  stopbits_two = 2,
  stopbits_one_point_five
} stopbits_t;

/*!
* Enumeration defines the possible flowcontrol types for the serial port.
*/
typedef enum {
  flowcontrol_none = 0,
  flowcontrol_software,
  flowcontrol_hardware
} flowcontrol_t;

} // end of namespace serial

#endif __SERIAL_EVT_H__


