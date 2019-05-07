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
// File           : dbgdlg.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : command debugger dialog
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "common.h"
#include "childdlg/dbgdlg.h"
#include "pyQt/udobj/ftdiComObj.h"
#include "ftdi/ftdi_usb2spi.h"
#include "ftdi/spi_access.h"

#include <QListWidget>
#include <QGridLayout>
#include <QTextEdit>
#include <QDebug>
#include <QLabel>
#include <QPushButton>
#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QListWidgetItem>
#include <QFileDialog>
#include <QTextStream>

dbgdlg::dbgdlg(QWidget *parent)
	: QDialog(parent)
{
  QStringList py_pre_defined;
  QGridLayout* dlgLayout = new QGridLayout(this);
  QLabel* qLabel[4];
  
  py_pre_defined
    << "from PythonQt.ftdi import *"
    << "ftdiobj = ftdiComObj()"
    << "class gfunc:"
    << "  global chkform"
    << "  def chkform(str):"
    << "    pos = str.find(\"0x\")"
    << "    slen = len(str)"
    << "    if pos == -1:"
    << "      if slen > 2 :"
    << "        print (\"Error : data length is over 2\")"
    << "        return -1"
    << "    elif pos != 0: "
    << "      print (\"Error : not valid Format\")"
    << "      return -1"
    << "    elif pos == 0 and slen > 4:"
    << "      print (\"Error : data length is over 2\")"
    << "      return -1"
    << "    return 1"
    << "  global chkrange"
    << "  def chkrange(num):"
    << "    if num < 0 or num > 255:"
    << "      print (\"Error : number range is over 0~255\")"
    << "      return -1"
    << "    return 1"
    << "  global spi_wadr"
    << "  def spi_wadr(adr, wdat):"
    << "    if type(adr) is int:"
    << "      ladr = adr"
    << "    else:"
    << "      if chkform(adr)<0 :"
    << "        return"
    << "      ladr = int(adr, 16)"
    << "    if type(wdat) is int:"
    << "      lwdat = wdat"
    << "    else:"
    << "      if chkform(wdat) < 0:"
    << "        return"
    << "      lwdat = int(wdat, 16)"
    << "    if chkrange(ladr) < 0 or chkrange (lwdat) < 0:"
    << "      return"
    << "    ftdiobj.py_spi_wbyte(ladr, lwdat, 1)"
    << "  global spi_radr"
    << "  def spi_radr(adr):"
    << "    if type(adr) is int:"
    << "      ladr = adr"
    << "    else :"
    << "      if chkform(adr) < 0:"
    << "        return -2"
    << "      ladr = int(adr, 16)"
    << "    if chkrange(ladr) < 0:"
    << "      return -1"
    << "    return ftdiobj.py_spi_rbyte(ladr, 1)"
    << "  global spi_rburst"
    << "  def spi_rburst(adr, len):"
    << "    if type(adr) is int:"
    << "      ladr = adr"
    << "    else :"
    << "      if chkform(adr) < 0:"
    << "        return -2"
    << "      ladr = int(adr, 16)"
    << "    if chkrange(ladr) < 0:"
    << "      return -1"
    << "    return ftdiobj.py_spi_rbyte(ladr, len*-1)"
    << "  global spi_rname"
    << "  def spi_rname(regname):"
    << "    if (not type(regname) is str):"
    << "      print (\"Error : parameter is not string\")"
    << "      return -1"
    << "    return ftdiobj.py_spi_rname(regname)"
    << "  global spi_wname"
    << "  def spi_wname(regname, val):"
    << "    if (not type(regname) is str):"
    << "      print (\"Error : 1st param is not string\")"
    << "      return -1"
    << "    lval = 0"
    << "    if (not type(val) is int):"
    << "      lval = int(val, 16)"
    << "    else:"
    << "      lval = val"
    << "    return ftdiobj.py_spi_wname(regname, lval)"
    << "  global spi_queue_ctrl"
    << "  def spi_queue_ctrl(onoff):"
    << "    return ftdiobj.py_spi_queue_ctrl(onoff)"
    << "  global spi_queue_write"
    << "  def spi_queue_write():"
    << "    return ftdiobj.py_spi_queue_write()"
    << "  global spi_queue_read"
    << "  def spi_queue_read():"
    << "    str = ftdiobj.py_spi_queue_read()"
    << "    return str";
  
  
  for (int i=0; i<2; i++) {
    m_qButton[i] = new QPushButton(this);
    m_qButton[i]->setObjectName(QString(tr("")).sprintf("btn_%02d", i));
  }
  
  m_qButton[0]->setText("log on");
  m_qButton[1]->setText("log save");

  for (int i=0; i<3; i++) {
    qLabel[i] = new QLabel(this);
  }
  
  m_logon = false;
  m_saveon = false;
  m_fp = NULL;
  m_fp_out = NULL;
  m_qListWidget = new QListWidget(this);
  m_qListWidget->setObjectName("flist");
  
  m_logconsole = new QTextEdit(this);
  m_logconsole->setObjectName("log");
    
  qLabel[0]->setText(QString(tr("scripts files")));
  qLabel[0]->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  qLabel[1]->setText(QString(tr("python console")));
  qLabel[1]->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  qLabel[2]->setText(QString(tr("log window")));
  qLabel[2]->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
  
  PythonQt::init(PythonQt::IgnoreSiteModule | PythonQt::RedirectStdOut);

  m_mainContext = PythonQt::self()->getMainModule();
  m_pyQtConsole = new PythonQtScriptingConsole(this, m_mainContext);

  // serching .py phthon scripts in scripts folder
  m_sspath  = QCoreApplication::applicationDirPath();
  m_sspath += tr("/scripts/");
  
  // register the new object as a known classname and add it's wrapper object
  PythonQt::self()->registerCPPClass("ftdiComObj", "", "ftdi", PythonQtCreateObject<ftdiComObjWrapper>);

  QStringList filter;
  QString fileName = tr("*.py");
  if (!fileName.isEmpty())
    filter << fileName;
  QDirIterator it(m_sspath, filter, QDir::AllEntries | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
  while (it.hasNext()) {
    QFileInfo fileInfo(it.next());
    m_qListWidget->addItem(fileInfo.fileName());
  }

  m_mainContext.evalScript(py_pre_defined.join("\n"));
  
  // setting grid layout
  dlgLayout->addWidget(qLabel[0],       0, 0, 1, 1);
  dlgLayout->addWidget(m_qListWidget,   1, 0, 8, 1);
  dlgLayout->addWidget(qLabel[1],       0, 1, 1, 1);
  dlgLayout->addWidget(m_pyQtConsole,   1, 1, 5, 3);
  dlgLayout->addWidget(qLabel[2],       6, 1, 1, 1);
  dlgLayout->addWidget(m_logconsole,    7, 1, 2, 2);
  dlgLayout->addWidget(m_qButton[0],    7, 3, 1, 1);
  dlgLayout->addWidget(m_qButton[1],    8, 3, 1, 1);
  
  this->setLayout(dlgLayout);

  connect(m_qListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(qListitemDoubleClicked(QListWidgetItem*)));
  connect(m_pyQtConsole, SIGNAL(runScriptsFile(const QString&)), this, SLOT(pyQtrunScript(const QString&)));
  connect(m_qButton[0], SIGNAL(clicked()), this, SLOT(log_onoff_btnclicked()));
  connect(m_qButton[1], SIGNAL(clicked()), this, SLOT(log_save_btnclicked()));
}

void dbgdlg::qListitemDoubleClicked(QListWidgetItem* pItem)
{
  pyQtrunScript(pItem->text());
}

void dbgdlg::pyQtrunScript(const QString& filename)
{
  QString fullpath = m_sspath + filename;
  QFile scr(fullpath);
  QString err = "print(\"Error: does not exist filename - ";

  if (!scr.exists()) {
    err += filename + "\")";
    m_mainContext.evalScript(err);
  }
  else {
    m_mainContext.evalFile(fullpath);
  }
  
  m_pyQtConsole->appendCommandPrompt(false);
}

void dbgdlg::appendline(const QString& str)
{
  if (!m_logon)
    return;
  
  m_logconsole->moveCursor(QTextCursor::End);
  m_logconsole->insertPlainText(str+"\n");
  
  if (!m_saveon)
    return;
  
  *m_fp_out << str << "\n";
}

void dbgdlg::log_onoff_btnclicked()
{
  if (!m_logon) {
    m_qButton[0]->setText("log off");
    m_logon = true;
  }
  else {
    m_qButton[0]->setText("log on");
    m_logon = false;
  }
}

void dbgdlg::log_save_btnclicked()
{
  unsigned char  rarr[200] = {0};
  
  spi_queue_ctrl(true);
  int i = 400;
  
  // if (!m_saveon) {
    
    // QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
                           // "./",
                           // tr("log files (*.log)"));
    
    // m_fp = new QFile(fileName);
    // if (!m_fp->open(QIODevice::WriteOnly | QIODevice::Text))
      // return;
    
    // m_fp_out = new QTextStream(m_fp);
    
    // m_qButton[1]->setText("log close");
    // m_saveon = true;
  // }
  // else {
    // if (m_fp_out != NULL)
      // delete m_fp_out;
    // m_fp_out = NULL;
    
    // m_fp->close();
    // if (m_fp != NULL)
      // delete m_fp;
    // m_fp = NULL;
    
    // m_qButton[1]->setText("log save");
    // m_saveon = false;
  // }
}

