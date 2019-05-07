/////////////////////////////////////////////////////////////////////
//                                                                   
//   r5MgMP:  .ubRgZU        :YPMQQu    rri7jY  :7iriiirir7i         
// 7BBBBBBBU BBBBBBBB      sBBBBBBBQ   BBBBBB.  gBBBBBBBBBBQ         
// BBBBBBBB27BBBBBBBB     ZBBBBBBBBB   BBBBB    jBBBQBBBBBBP         
// BBBBBBBKi:BBBBBBdM    .BBBBBBBQBB   PBBBB    vBQBBBBBBMBJ         
//  QBBBQY   YBBBBB      7BBBBBD       .BBBB       rBBBBs            
//    DBBB:    vBBBD     .BBBBB      iBQBQBBj Br   :BBBQv            
//     BQBB     7BBBs     qBBBBX     BBB. BBBBB7   :BBBBr            
//  vBBBBBB   BQBBBB:      vBBBBBBBg BBBj MBBBi    .BBBQi            
//   BBBg7    PBBB5          iSBBBB7  sBBBP.uQb     MDEQ.            
//                                                                   
// Copyright 2018 - ShinSung C&T 
//                                                                   
// Licensed under the Apache License, Version 2.0 (the 'License');   
// you may not use this file except in compliance with the License.  
// You may obtain a copy of the License at                           
//                                                                   
//      http://www.apache.org/licenses/LICENSE-2.0                   
//                                                                   
// Unless required by applicable law or agreed to in writing,        
// software distributed under the License is distributed on          
// an 'AS IS' BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,   
// either express or implied. See the License for the specific       
// language governing permissions and limitations under the License. 
//                                                                   
/////////////////////////////////////////////////////////////////////
                                                                     
/////////////////////////////////////////////////////////////////////
//                                                                   
// File           : ssdbg.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : shin-sung ic debugger main dialog
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "common.h"
#include "comm_macro.h"
#include "childdlg/ssdbg.h"
#include "childdlg/tbldlg.h"
#include "childdlg/dbgdlg.h"
#include "childdlg/gphdlg.h"
#include "childdlg/imgdlg.h"
#include "ftdi/ftdi_usb2spi.h"
#include "serial/qsport.h"
#include "serial/uart_acc.h"
#include "serial/uart_conn.h"
#include "ctrlid.h"
#include "ftdi/spi_access.h"
#include "mth/rdyqueue.h"
#include <QtWinExtras\qwinfunctions.h>
#include <QGraphicsDropShadowEffect>
#include <QGuiApplication>
#include <QLayout>
#include <QToolButton>
#include <QSignalMapper>
#include <QDebug>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QMouseEvent>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDir>
#include <QDirIterator>
#include <QPixmap>
#include <QThread>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <cstdio>
#include <iostream>

using std::string;
using std::exception;
using std::cout;
using std::cerr;
using std::endl;
using std::vector;
using namespace coms;
using namespace std;

QString btnstr[] = {
  "LOAD",
  "SAVE",
  "WRITE",
  "READ",
  "WRITE ALL",
  "READ ALL"
};

ssdbg::ssdbg(QWidget *parent)
    : QMainWindow(parent)
    , m_conn_chk(false)
    , m_mousePressed(false)
{
  ui.setupUi(this);
  resize(1200, 700);

#ifdef Q_OS_WIN
  Qt::WindowFlags flags = 0;

  if (QtWin::isCompositionEnabled()) {
    flags |= Qt::MSWindowsFixedSizeDialogHint;
    flags |= Qt::FramelessWindowHint;
    flags |= Qt::WindowSystemMenuHint;
    this->setWindowFlags(flags);
    QtWin::enableBlurBehindWindow(this);
    QtWin::extendFrameIntoClientArea(this, -1, -1, -1, -1);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, false);
    
    //window shadow
    // QGraphicsDropShadowEffect *winShadow = new QGraphicsDropShadowEffect;
    // winShadow->setBlurRadius(9.0);
    // winShadow->setColor(palette().color(QPalette::Highlight));
    // winShadow->setOffset(0.0);
    // ui.centralWidget->setGraphicsEffect(winShadow);
  }
  else {
    QtWin::resetExtendedFrame(this);
    setAttribute(Qt::WA_TranslucentBackground, false);
    // setStyleSheet(QStringLiteral("ssdbgClass { background: %1; }").arg(QtWin::realColorizationColor().name()));
  }
#endif

  QGridLayout* gridLayout = new QGridLayout(this);
  QHBoxLayout* sysBtnLayout = new QHBoxLayout;
  QTabWidget* tabwidget = new QTabWidget(this);
  m_minButton = new QToolButton(this);
  m_quitButton = new QToolButton(this);
  QSignalMapper *signalMapper = new QSignalMapper(this);
  m_connect = new QLabel();
  QPushButton* qButtons[BUTTON_NUM];
  QLabel* qLabel[LABEL_NUM];
  m_connect->setPixmap(QPixmap(":/res/img/off_state.png"));
  m_connect->setAlignment(Qt::AlignVCenter);
  m_connect->setStyleSheet(QStringLiteral("QLabel{ padding-left: 15px; }"));
  
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(btnclicked(int)));

  m_minButton->setObjectName("minimize");
  m_quitButton->setObjectName("close");
  
  m_pdbgdlg = new dbgdlg(this);
  m_pgphdlg = new gphdlg(this);
  m_pimgdlg = new imgdlg(this);
  m_ptbldlg = new tbldlg(this);
  
  int btncnt = MAIN_BTN_LOAD;

  for (int i=0; i<6; i++) {
    qButtons[i] = new QPushButton(btnstr[i], this);
    qButtons[i]->setStyleSheet("QPushButton { min-width : 110 px; \
                                               min-height: 25 px; }");
    connect(qButtons[i], SIGNAL(clicked()), signalMapper, SLOT(map()));
          signalMapper->setMapping(qButtons[i], btncnt++);
  }

  m_qComboBox = new QComboBox;
  qLabel[0] = new QLabel;
  qLabel[1] = new QLabel;
  qLabel[2] = new QLabel;
  qLabel[0]->setText(QString(tr("ADDR : 0x")));
  qLabel[0]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  qLabel[1]->setText(QString(tr("WVAL : 0x")));
  qLabel[1]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  qLabel[2]->setText(QString(tr("RVAL : 0x")));
  qLabel[2]->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  
  for (int i = 0; i < 3; i++) {
    m_qle[i] = new QLineEdit;
    m_qle[i]->setObjectName("medit");
    m_qle[i]->installEventFilter(this);
  }
  m_qle[MAIN_EDITID(MAIN_EDIT_RDATA)]->setReadOnly(true);
  
  tabwidget->addTab(m_pgphdlg, tr("Graph"));
  tabwidget->addTab(m_ptbldlg, tr("Contol"));
  tabwidget->addTab(m_pdbgdlg, tr("Debug"));
  tabwidget->addTab(m_pimgdlg, tr("Image"));
  
  sysBtnLayout->addWidget(m_minButton);
  sysBtnLayout->addWidget(m_quitButton);

  gridLayout->addWidget(tabwidget,    0,  0, 2, 12);
  gridLayout->addWidget(m_connect,    0,  0, 1, 1);
  gridLayout->addWidget(m_qComboBox,  0,  8, 1, 2);
  gridLayout->addLayout(sysBtnLayout, 0, 11);
  gridLayout->addWidget(qButtons[0],  2,  0);
  gridLayout->addWidget(qButtons[1],  2,  1);
  gridLayout->addWidget(qLabel[0],    2,  2);
  gridLayout->addWidget(m_qle[MAIN_EDITID(MAIN_EDIT_WADDR)], 2,  3);
  gridLayout->addWidget(qLabel[1],    2,  4);
  gridLayout->addWidget(m_qle[MAIN_EDITID(MAIN_EDIT_WDATA)], 2,  5);
  gridLayout->addWidget(qButtons[2],  2,  6);
  gridLayout->addWidget(qLabel[2],    2,  7);
  gridLayout->addWidget(m_qle[MAIN_EDITID(MAIN_EDIT_RDATA)], 2,  8);
  gridLayout->addWidget(qButtons[3],  2,  9);
  gridLayout->addWidget(qButtons[4],  2, 10);
  gridLayout->addWidget(qButtons[5],  2, 11);

  ui.centralWidget->setLayout(gridLayout);
  
  layout()->setMargin(15);

  // serching .tbl files and add to combobox list
  QString exePath = QCoreApplication::applicationDirPath();
  exePath += tr("/regset/");

  QStringList filter;
  QString fileName = tr("*.tbl");
  if (!fileName.isEmpty())
    filter << fileName;
  QDirIterator it(exePath, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
  while (it.hasNext()) {
    QFileInfo fileInfo(it.next());
    m_qComboBox->addItem(fileInfo.baseName());
  }
  m_qComboBox->setCurrentIndex(0);

  QObject::connect(qApp, &QGuiApplication::applicationStateChanged, this, &ssdbg::OnAppStateChanged);
  connect(m_quitButton, SIGNAL(clicked()), this, SLOT(OncloseButtonClicked()));
  connect(m_minButton, SIGNAL(clicked()), this, SLOT(OnminimizeButtonClicked()));

  m_connect->installEventFilter(this);
  
  m_ptbldlg->createTable();
  m_pgphdlg->addReadString(m_ptbldlg->m_readonly);
  spi_debug_alloc(m_pdbgdlg);
  getmainWidget(this);

  if (try_conn(false)) {
    intr_read(false);
    Sleep(5);
    try_discon();
    try_conn(false);
    intr_read(false);
    Sleep(5);
    try_discon();
    try_conn(true);
  }
  // get_whdl(true);
  // try_conn(true);

  start_thread();
}

ssdbg::~ssdbg()
{
  intr_read(false);
  Sleep(5);
  try_discon();
  try_conn(false);
  intr_read(false);
  Sleep(5);
  try_discon();
  stop_thread();
}

void ssdbg::ftdi_try_conn()
{
  bool con_ok = false;
  int try_cnt = 0;

  do {
    con_ok = ftdi_connect();
    // qDebug() << "ftdi_connect : " << con_ok;
    if (try_cnt++ > 3) {
      set_conn_state(false);
      return;
    }
    QThread::msleep(1);

  } while (!con_ok);

  set_conn_state(true);
}

void ssdbg::ftdi_try_discon()
{
  bool disconnect_ok = false;
  disconnect_ok = ftdi_disconnect();
  // qDebug() << "disconnect : " << disconnect_ok;
  set_conn_state(false);
}

void ssdbg::OnAppStateChanged(Qt::ApplicationState state)
{
  if (windowState().testFlag(Qt::WindowNoState)) {
    if (state == Qt::ApplicationActive) {
      styleWindow(true);
    }
    else {
      styleWindow(false);
    }
  }
}

bool ssdbg::uart_try_conn(bool set)
{
  // coms::CSerialEvt* lp_COM = NULL;
  QSerialPort* lp_COM = NULL;
  bool success = false;

  set_conn_state(false);
  if (chk_sport()) {
    // lp_COM = get_serial_hdl(true);
    // if (lp_COM != 0) {
    //   lp_COM->setManager(SerialEventManager);
    //   lp_COM->setRxSize(75);
    //   int erreur = lp_COM->connect(get_portnum(), 921600, SERIAL_PARITY_NONE, 8, true);
    //   if (!erreur) {
    //     set_conn_state(set);
    //     success = true;
    //     // qDebug() << QString("\n\n***********\n\nConnect Succeed\n\n**********n\n");
    //   }
    // }
    lp_COM = get_qsport_hdl(true); 
    lp_COM->setPort(get_portinfo()[0]);
    lp_COM->setBaudRate(921600);
    lp_COM->setReadBufferSize(0);
    bool err = lp_COM->open(QIODevice::ReadWrite);
    if (err) {
      set_conn_state(set);
      success = true;
      m_pqsport = new qsport(get_qsport_hdl(false), this);
      qDebug() << QString("\n\n***********\n\nConnect Succeed\n\n**********n\n");
    }
  }

  return success;
}

void ssdbg::uart_try_discon()
{
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  QSerialPort* lp_COM = get_qsport_hdl(false);

  lp_COM->close();
  // lp_COM->disconnect();
  // delete[] lp_COM;
  lp_COM = NULL;

  set_conn_state(false);
}

void ssdbg::set_conn_state(bool set)
{
  if (!set) {
    m_connect->setPixmap(QPixmap(":/res/img/off_state.png"));
    m_conn_chk = false;
  }
  else {
    m_connect->setPixmap(QPixmap(":/res/img/on_state.png"));
    m_conn_chk = true;
  }
}

void ssdbg::styleWindow(bool bActive)
{
  if (bActive) {
    QGraphicsEffect *oldShadow = ui.centralWidget->graphicsEffect();
    if (oldShadow)
      delete oldShadow;
    QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
    windowShadow->setBlurRadius(9.0);
    windowShadow->setColor(palette().color(QPalette::Highlight));
    windowShadow->setOffset(0.0);
    ui.centralWidget->setGraphicsEffect(windowShadow);
  }
  else {
    
    QGraphicsEffect *oldShadow = ui.centralWidget->graphicsEffect();
    if (oldShadow)
      delete oldShadow;
    QGraphicsDropShadowEffect *windowShadow = new QGraphicsDropShadowEffect;
    windowShadow->setBlurRadius(9.0);
    windowShadow->setColor(palette().color(QPalette::Shadow));
    windowShadow->setOffset(0.0);
    ui.centralWidget->setGraphicsEffect(windowShadow);
  } // if (bActive) { else no focus
}


void ssdbg::btnclicked(int nID)
{
  QString dbg = "";
  QString addr = "";
  QString data = "";
  BYTE num_addr = 0;
  BYTE num_data = 0;
  int ret = 0;
  uint8_t ddat[9] = {0};

  // qDebug() << tr("nId = %1 \n").arg(nID);
  switch (nID)
  {
    case MAIN_BTN_LOAD :
      qDebug() << QString("LOAD");
      // uart_read(0x02, ddat, 6);
      break;
    case MAIN_BTN_SAVE:
      qDebug() << QString("SAVE");
      break;
    case MAIN_BTN_WRITE:
      m_pgphdlg->graph_turnoff();
      addr += m_qle[MAIN_EDITID(MAIN_EDIT_WADDR)]->text();
      data += m_qle[MAIN_EDITID(MAIN_EDIT_WDATA)]->text();
      num_addr = addr.toInt(Q_NULLPTR, 16);
      num_data = data.toInt(Q_NULLPTR, 16);
      addr.sprintf("0x%02x", num_addr);
      data.sprintf("0x%02x", num_data);
      m_ptbldlg->UpdateAddrData(addr, data);
      brd_byte_wr(num_addr, num_data);
      break;
    case MAIN_BTN_READ:
      m_pgphdlg->graph_turnoff();
      addr += m_qle[MAIN_EDITID(MAIN_EDIT_WADDR)]->text();
      num_addr = addr.toInt(Q_NULLPTR, 16);
      num_data = brd_byte_rd(num_addr);
      addr.sprintf("0x%02x", num_addr);
      data.sprintf("%02x", num_data);
      m_ptbldlg->UpdateAddrData(addr, data);
      m_qle[MAIN_EDITID(MAIN_EDIT_RDATA)]->setText(data);
      break;
    case MAIN_BTN_WRITEALL:
      m_pgphdlg->graph_turnoff();
      m_ptbldlg->WriteAllData();
      break;
    case MAIN_BTN_READALL:
      m_pgphdlg->graph_turnoff();
      m_ptbldlg->ReadAllData();
      break;
  }
}

void ssdbg::mousePressEvent(QMouseEvent *event)
{
	m_mousePressed = (event->y() < 40)? true : false;
  m_mousePos = event->globalPos();
  m_wndPos = this->pos();
}

void ssdbg::mouseMoveEvent(QMouseEvent *event)
{
	if((event->buttons() == Qt::LeftButton) && m_mousePressed){
		move(m_wndPos + (event->globalPos()-m_mousePos));
	}
}

void ssdbg::OnminimizeButtonClicked()
{
  setWindowState(Qt::WindowMinimized);
}

void ssdbg::OncloseButtonClicked()
{
  this->close();
}

void ssdbg::OnVersionCheck()
{
  qDebug() << " ssdbg version check clicked()\n";
}

void ssdbg::intr_read(bool onoff)
{
  send_ufunc2_cmd((!onoff)? false : true);
  // coms::CSerialEvt* lp_COM = get_serial_hdl(false);
  // uart_flush();
}

// R : 253,   G : 173,    B : 32  --> yellow
// R : 33,    G : 42,     B : 60  --> dark blue

bool ssdbg::eventFilter(QObject *obj, QEvent *evt)
{
  QString str = "";
  
  if (evt->type() == QEvent::KeyPress) {
    QKeyEvent* key = static_cast<QKeyEvent*>(evt);
    int keyval = key->key();
    str = ((QLineEdit*)obj)->text();
    int len = str.length();
    int maxlen = 2;

    if ((keyval < '0' || keyval > '9') && (keyval < Qt::Key_A || keyval > Qt::Key_F) &&
      keyval != Qt::Key_Backspace && keyval != Qt::Key_Return &&
      keyval != Qt::Key_Enter)
      return true;
    else if (len >= maxlen && keyval != Qt::Key_Backspace) {
      return true;
    }
  }
  else if (evt->type() == QEvent::MouseButtonRelease) {
    if (obj == m_connect){
      uint8_t vinfo[2] = { 0 };
      send_get_verinfo(vinfo);
      QString vstr = tr("");
      vstr.sprintf("v%d.%d", vinfo[0], vinfo[1]);
      QMessageBox::information(this, tr("ssdbg"), tr("ssdbg v1.9 / fw : ")+vstr);
    }
  }
  
  return QObject::eventFilter(obj, evt);
}

