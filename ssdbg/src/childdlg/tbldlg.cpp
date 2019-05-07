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
// File           : tbldlg.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : tableview dialog
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////
#include "common.h"
#include "childdlg/tbldlg.h"
#include "childdlg/ssdbg.h"
#include "childdlg/gphdlg.h"
#include "childdlg/dlghandle.h"
#include "ftdi/spi_access.h"
#include "serial/uart_acc.h"
#include "umacros.h"
#include <string>
#include <stdlib.h>
#include <iostream>
#include <functional>
#include <tchar.h>

#include <QStandardItemModel>
#include <QHeaderView>
#include <QComboBox>
#include <QDebug>
#include <QWidget>
#include <QFile>
#include <QMessageBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLayout>

tbldlg::tbldlg(QWidget *parent)
  : QTableView(parent)
{
  QStandardItemModel* p_mdl = geth_mdl(true);
  setModel(p_mdl);

  m_mask_bits = new uint16_t[1];
  *m_mask_bits = 0;
      
  m_qLineEdit = Q_NULLPTR;
  m_qComboBox = Q_NULLPTR;
  m_qSpinBox = Q_NULLPTR;
  m_qSlider = Q_NULLPTR;

  change_from_spin = false;
  change_from_slider = false;

  setEditTriggers(QAbstractItemView::NoEditTriggers);
  
  p_mdl->setHorizontalHeaderLabels(QStringList() << tr("Addr") << tr("Val") << tr("Name")
    << tr("Val(HEX)") << tr("Val(DEC)"));

  m_readonly = tr("");
  
  horizontalHeader()->setMinimumSectionSize(160);
  horizontalHeader()->setSectionResizeMode(CIDX_ADDR   , QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(CIDX_AVAL   , QHeaderView::Fixed);
  horizontalHeader()->setSectionResizeMode(CIDX_NAME   , QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(CIDX_NVALHEX, QHeaderView::Stretch);
  horizontalHeader()->setSectionResizeMode(CIDX_NVALDEC, QHeaderView::Stretch);
  
  connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(cellclickedSlot(QModelIndex)));
  
}

void tbldlg::createTable()
{
  QString objname = tr("");
  m_topwidget = this;
  int j = 0;
  int linecnt = 0;
  int lpos = 0;
  QString strname = tr("");
  QString strtype = tr("");
  QString straddr = tr("");
  int32_t curr_adr = 0;
  int32_t prev_adr = -1;
  int32_t same_adr = 0;
  uint32_t bitwidth = 0;
  uint32_t nidx = 0;
  uint32_t subcnt = 0;
  uint32_t dcnt = 0;
  strtreginfo reg_info;
  QHash<QString, strtreginfo>* p_reghash = geth_hash();
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  // p_mdl->clear();
  p_reghash->clear();

  do {
    m_topwidget = m_topwidget->parentWidget();
    objname = m_topwidget->objectName();
  } while (objname.compare(tr("ssdbgClass")));

  // serching .tbl files and add to combobox list
  QString exePath = QCoreApplication::applicationDirPath();
  exePath += tr("/regset/");
  QString filename = exePath;
  
  filename += ((ssdbg*)m_topwidget)->m_qComboBox->currentText();
  filename += tr(".tbl");

  QFile tdata(filename);
  
  if (!tdata.open(QIODevice::ReadOnly | QIODevice::Text))
    return;

  QString strFile = QString::fromUtf8(tdata.readAll());
  tdata.close();

  while ((j = strFile.indexOf('@', j)) != -1) {
    linecnt++;
    j++;
  }
  p_mdl->setRowCount(linecnt);

  QStandardItem* newItem = NULL;

  for (int i = 0; i < linecnt; i++) {
    for (int j = 0; j < 5; j++) {
      newItem = new QStandardItem();
      newItem->setTextAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      p_mdl->setItem(i, j, newItem);
    }
  }

  QStringList spline = strFile.split(QRegExp("\n"));

  foreach(const QString &line, spline) {
    if (!line.isEmpty()) {
      QStringList parse = line.split(QRegExp("\\ +"));
      strname = parse.at(0);
      strtype = parse.at(1);
      straddr = parse.at(2);
      parse = straddr.split("@");
      QStringListIterator itr(parse);
      itr.next();
      bitwidth = 0;
      subcnt = 0;
      while (itr.hasNext()) {
        QString tokadr = itr.next();
        int adrpos = tokadr.indexOf('[', 0);
        if (adrpos != -1) {
          curr_adr = tokadr.left(adrpos).toInt(Q_NULLPTR, 16);
          QRegExp expMSB("\\[(\\d+)(:)(\\d+)\\]");
          if (expMSB.indexIn(tokadr, 0) != -1) {
            int msb = expMSB.cap(1).toInt();
            int lsb = expMSB.cap(3).toInt();
            bitwidth += msb - lsb + 1;
            if (curr_adr != prev_adr) {
              same_adr = 0;
              QString tmp = tr("");
              tmp.sprintf("0x%02x", curr_adr);
              p_mdl->item(nidx + subcnt, CIDX_ADDR)->setText(tmp);
              p_mdl->item(nidx + subcnt, CIDX_AVAL)->setText(tr("0x00"));
            }
            else {
              same_adr++;
            }

            if (!strtype.compare(tr("RO"))) {
              for (int k = 0; k < 5; k++)
                p_mdl->item(nidx + subcnt, k)->setBackground(QBrush(QColor(0xe0e0ff), Qt::SolidPattern));
            }
            subcnt++;
          }
          else {
            QMessageBox::warning(this, tr("tbldlg"), tr("pasing error !!"));
            return;
          }
        }
        else {
          QMessageBox::warning(this, tr("tbldlg"), tr("pasing error !!"));
          return;
        }
      }
      if (subcnt == 1 && curr_adr == prev_adr) {
        setSpan(nidx-(same_adr), 0, same_adr+1, 1);
        setSpan(nidx-(same_adr), 1, same_adr+1, 1);
      }
      p_mdl->item(nidx, CIDX_NAME)->setText(strname);

      reg_info.type = strtype;
      reg_info.adr = straddr;
      reg_info.grid_addr_pos = nidx-same_adr;
      reg_info.grid_name_pos = nidx;
      reg_info.bitw = bitwidth;
      
      (*p_reghash)[strname] = reg_info;
      strtregrange rrange = cal_bit_range(strname);
      QString localfmt = tr("");
      localfmt.sprintf(rrange.fmt.toStdString().c_str(), 0);
      
      if (!strtype.compare(tr("RO")))
        m_readonly += (strname + tr("@"));

      if (subcnt > 1 && curr_adr != prev_adr) {
        setSpan(nidx, 2, subcnt, 1);
        if (bitwidth == 1) {
          setSpan(nidx, 3, nidx, 2);
          p_mdl->item(nidx, CIDX_NVALHEX)->setText(tr("OFF (0)"));
        }
        else {
          setSpan(nidx, 3, subcnt, 1);
          setSpan(nidx, 4, subcnt, 1);
          p_mdl->item(nidx, CIDX_NVALHEX)->setText(localfmt);
          p_mdl->item(nidx, CIDX_NVALDEC)->setText(tr("0"));
        }
      }
      else if (bitwidth == 1) {
        setSpan(nidx, 3, 1, 2);
        p_mdl->item(nidx, CIDX_NVALHEX)->setText(tr("OFF (0)"));
      }
      else if (bitwidth > 1) {
        p_mdl->item(nidx, CIDX_NVALHEX)->setText(localfmt);
        p_mdl->item(nidx, CIDX_NVALDEC)->setText(tr("0"));
      }
      nidx += subcnt;
    }
    prev_adr = curr_adr;
    dcnt++;
  }  
}

strtregrange tbldlg::cal_bit_range(const QString& text)
{
  strtregrange brange = {0};
  strtreginfo v_Info;
  QString str = text;
  QHash<QString, strtreginfo>* p_reghash = geth_hash();
  
  int64_t l_min = 0;
  int64_t l_max = 0xffffffff;
  
  brange.valid = false;
  
  if (p_reghash->contains(str)) {
    v_Info = (*p_reghash)[str];
    l_max = (((uint64_t)1 << v_Info.bitw)-1);
    
    brange.valid = true;
    brange.l_min = l_min;
    brange.l_max = l_max;
    brange.fmt.sprintf("0x%%0%dx", (int)floor(ud_log2(l_max) / 4) + 1);
    brange.v_info = v_Info;
  }

  return brange;
}

QHash<QString, strtreginfo>* tbldlg::geth_hash()
{
  static QHash<QString, strtreginfo> reghash;
  
  return &reghash;
}

QStandardItemModel* tbldlg::geth_mdl(bool create)
{
  static QStandardItemModel* lp_mdl = NULL;
  
  if (create) {
    lp_mdl = new QStandardItemModel();
  }
  
  return lp_mdl;
}

int tbldlg::UpdateNameData(const QString& lpsz, bool bSign)
{
	QString adr = tr("");
  QString val_text = tr("");
  QString str_hex = tr("");
  QString str_dec = tr("");
  strtregrange brange;
  long row = 0;
  int64_t val = 0;
	int pos = 0;
	bool readonly = false;
	int width = 0;
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  brange = cal_bit_range(lpsz);
  row = brange.v_info.grid_name_pos;
  
	if(brange.valid){
		
		adr = brange.v_info.adr;
		unsigned long ladr = 0;
    uint32_t rowidx = brange.v_info.grid_addr_pos;
		
    QStringList parse = adr.split("@");
    QStringListIterator itr(parse);
    itr.next();
    while (itr.hasNext()) {
      QString tokadr = itr.next();
      int adrpos = tokadr.indexOf('[', 0);
      if (adrpos != -1) {
        ladr = tokadr.left(adrpos).toInt(Q_NULLPTR, 16);
        QRegExp expMSB("\\[(\\d+)(:)(\\d+)\\]");
        if (expMSB.indexIn(tokadr, 0) != -1) {
          int msb = expMSB.cap(1).toInt();
          int lsb = expMSB.cap(3).toInt();

          unsigned char gval = 0;
          val_text = p_mdl->item(rowidx, CIDX_AVAL)->text();

          // make mask bits
          if (ladr == m_mask_adr && m_mask_adr != 0xffff) {
            *m_mask_bits |= (~(0xff << (msb + 1)) & (0xff << lsb));
          }

          gval = val_text.toInt(Q_NULLPTR, 16);

          while (msb != (lsb - 1)) {
            val <<= 1;
            val |= ((gval >> msb) & 0x01);
            msb--;
          }
          rowidx++;
        }
      }
    }
	}
	else {
    QMessageBox::warning(this, tr("tbldlg"), tr("Can't find string %1").arg(lpsz));
    qDebug() << lpsz;
		return 0;
	}
	
	if(bSign)
		if((val >> (width-1)) & 0x00000001){
			val |= ((0xfffffffe) << (width-1));
		}
  
  if (brange.v_info.bitw > 1){
    val = ud_clip(val, brange.l_min, brange.l_max);
    str_hex.sprintf(brange.fmt.toStdString().c_str(), val);
    str_dec.sprintf("%d", val);
    p_mdl->item(row, CIDX_NVALHEX)->setText(str_hex);
    p_mdl->item(row, CIDX_NVALDEC)->setText(str_dec);
  }
  else {
    p_mdl->item(row, CIDX_NVALHEX)->setText((!val) ? tr("OFF (0)") : tr("ON (1)"));
  }
  
	return val;
}


bool tbldlg::SetValuenWrite(const QString& lpsz, int64_t value, bool write)
{
	QString str = lpsz;
	QString adr = tr("");
  QString valtext = tr("");
	QString dat = tr("");
  QString strdec = tr("");
  QString strhex = tr("");
  strtreginfo	sInfo = { 0 };
  strtregrange brange = { 0 };
	int pos = 0;
  QHash<QString, strtreginfo>* p_reghash = geth_hash();
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  brange = cal_bit_range(str);

	if(p_reghash->contains(str)){
    sInfo = brange.v_info;
		
		adr = sInfo.adr;
		int width = sInfo.bitw;
		unsigned long ladr = 0;
    uint32_t rowidx = sInfo.grid_addr_pos;
    uint32_t nameidx = sInfo.grid_name_pos;
		
    QStringList parse = adr.split("@");
    QStringListIterator itr(parse);
    itr.next();
    
    int satval = ud_clip(value, brange.l_min, brange.l_max);

    if (width == 1) {
      strhex = (!value)? tr("OFF (0)") : tr("ON (1)");
    }
    else {
      strhex.sprintf(brange.fmt.toStdString().c_str(), satval);
      strdec.sprintf("%d", satval);
      p_mdl->item(nameidx, CIDX_NVALDEC)->setText(strdec);
    }
    p_mdl->item(nameidx, CIDX_NVALHEX)->setText(strhex);

    while (itr.hasNext()) {
      QString tokadr = itr.next();
      int adrpos = tokadr.indexOf('[', 0);
      if (adrpos != -1) {
        ladr = tokadr.left(adrpos).toInt(Q_NULLPTR, 16);
        QRegExp expMSB("\\[(\\d+)(:)(\\d+)\\]");
        if (expMSB.indexIn(tokadr, 0) != -1) {
          int msb = expMSB.cap(1).toInt();
          int lsb = expMSB.cap(3).toInt();

          valtext = p_mdl->item(rowidx, CIDX_AVAL)->text();

          unsigned char gval = (unsigned char)valtext.toInt(Q_NULLPTR, 16);

          while (msb != (lsb - 1)) {
            unsigned char origdata = (gval >> msb) & 0x01;
            unsigned char newdata = (satval >> --width) & 0x01;

            if (origdata != newdata) {
              if (newdata)
                gval |= (0x01 << msb);		// reset '0'
              else
                gval &= ~(0x01 << msb);		// set '1'

            }
            msb--;
          }

          // set list data
          dat.sprintf("0x%02x", gval);
          p_mdl->item(rowidx, CIDX_AVAL)->setText(dat);

          if (write) {	// write only if write_flag is true
            brd_byte_wr(ladr, gval);
          }
        }
      }
      rowidx++;
    }
	}
	else {
    QMessageBox::warning(this, tr("tbldlg"), tr("Can't find string %1").arg(str));
    qDebug() << str;
		return false;
	}

	return true;
}

int tbldlg::ReadNGetValue(const QString& lpsz)
{
	QString str = lpsz;
	QString adr = tr("");
  QString valtext = tr("");
	QString dat = tr("");
  QString strdec = tr("");
  QString strhex = tr("");
  strtreginfo	sInfo = { 0 };
  strtregrange brange = { 0 };
	int pos = 0;
  int64_t val = 0;
  int width = 0;
  int gval = 0;
  uint32_t rowidx = 0;
  uint32_t nameidx = 0;
  QHash<QString, strtreginfo>* p_reghash = geth_hash();
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  brange = cal_bit_range(lpsz);
  
	if(p_reghash->contains(str)){
    sInfo = brange.v_info;
		
		adr = sInfo.adr;
		width = sInfo.bitw;
		unsigned long ladr = 0;
    rowidx = sInfo.grid_addr_pos;
    nameidx = sInfo.grid_name_pos;
		
    QStringList parse = adr.split("@");
    QStringListIterator itr(parse);
    itr.next();

    while (itr.hasNext()) {
      QString tokadr = itr.next();
      int adrpos = tokadr.indexOf('[', 0);
      if (adrpos != -1) {
        ladr = tokadr.left(adrpos).toInt(Q_NULLPTR, 16);
        QRegExp expMSB("\\[(\\d+)(:)(\\d+)\\]");
        if (expMSB.indexIn(tokadr, 0) != -1) {
          int msb = expMSB.cap(1).toInt();
          int lsb = expMSB.cap(3).toInt();

          gval = brd_byte_rd(ladr);

          while (msb != (lsb - 1)) {
            val <<= 1;
            val |= ((gval >> msb) & 0x01);
            msb--;
          }

          // set list data
          dat.sprintf("0x%02x", gval);
          p_mdl->item(rowidx, CIDX_AVAL)->setText(dat);
        }
      }
      rowidx++;
    }
	}
	else {
    QMessageBox::warning(this, tr("tbldlg"), tr("Can't find string %1").arg(lpsz));
    qDebug() << lpsz;
		return -1;
	}
  
  int satval = ud_clip(val, brange.l_min, brange.l_max);
  
  if (width == 1) {
    strhex = (!val)? tr("OFF (0)") : tr("ON (1)");
  }
  else {
    strhex.sprintf(brange.fmt.toStdString().c_str(), satval);
    strdec.sprintf("%d", satval);
    p_mdl->item(nameidx, CIDX_NVALDEC)->setText(strdec);
  }
  p_mdl->item(nameidx, CIDX_NVALHEX)->setText(strhex);

	return val;
}

int tbldlg::UpdateAddrData(const QString& lpsz_adr, const QString& lpsz_data)
{
  int row = 0;
  int col = 0;
  int endrow = 0;
  unsigned char fixrow = 1;
  int endcol = 0;
  int namecol = 2;
  int32_t val = 0;
  bool write_op = false;
  
  QString str_adr = lpsz_adr;
  QString str_data = lpsz_data;
  QString str_grid = tr("");
  QString str_val = tr("");
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  do {
    str_grid = p_mdl->item(row, CIDX_ADDR)->text();
    
    if (!str_grid.compare(str_adr)){
      int endcol = col + (columnSpan(row, CIDX_ADDR)-1);
      int endrow = row + (rowSpan(row, CIDX_ADDR)-1);
      int chkrow = rowSpan(row, CIDX_NAME) - 1;
      fixrow = (!chkrow)? 0 : 1;
      
      *m_mask_bits = 0;
      m_mask_adr = (uint32_t)str_adr.toInt(Q_NULLPTR, 16);
      if (str_data.compare("RDOP")){
        brd_byte_wr(str_adr.toInt(Q_NULLPTR, 16), str_data.toInt(Q_NULLPTR, 16));
        write_op = true;
      }
      str_data.sprintf("0x%02x", brd_byte_rd(str_adr.toInt(Q_NULLPTR, 16)));
      
      p_mdl->item(row, CIDX_AVAL)->setText(str_data);
      
      for (int i = row; i <= endrow; i++) {
        QString name = p_mdl->item((!fixrow) ? i : row, CIDX_NAME)->text();
        int minusrow = 1;
        while (name.isEmpty() && fixrow) {
          name = p_mdl->item(row-minusrow++, CIDX_NAME)->text();
        }
        UpdateNameData(name, false);
      }
      
      val = str_data.toInt(Q_NULLPTR, 16);
      str_val.sprintf("0x%02x", val & ((!write_op)? 0xffffffff : *m_mask_bits));
      p_mdl->item(row, CIDX_AVAL)->setText(str_val);
      return (val & ((!write_op)? 0xffffffff : *m_mask_bits));
    }
  } while (getnextrow(&row, 0));
  
  return 0;
}

void tbldlg::UpdateAllByName()
{
  int row = 0;
  QString str = tr("");
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  do {
    str = p_mdl->item(row, CIDX_NAME)->text();
    UpdateNameData(str, false);
  } while (getnextrow(&row, 2));
}

void tbldlg::cellclickedSlot(const QModelIndex & index)
{
  QModelIndex midx = index;
  QString strval = tr("");
  QString strname = tr("");
  strtregrange brange;
  int row = midx.row();
  int col = midx.column();
  int bitwidth = 0;
  QStandardItemModel* p_mdl = geth_mdl(false);

  if (m_qLineEdit != Q_NULLPTR) {
    setIndexWidget(m_editidx, 0);
    m_qLineEdit->setParent(this);
    m_qLineEdit->removeEventFilter(this);
    delete m_qLineEdit;
    m_qLineEdit = Q_NULLPTR;
  }
  
  if (m_qSpinBox != Q_NULLPTR) {
    QModelIndex lidx = p_mdl->index((!m_editidx.row())? 
            m_editidx.row()+1 : m_editidx.row()-1, m_editidx.column());
    setIndexWidget(lidx, 0);
    m_qSlider->setParent(this);
    m_qSlider->removeEventFilter(this);
    m_qSlider->disconnect(this);
    delete m_qSlider;
    m_qSlider = Q_NULLPTR;
    
    setIndexWidget(m_editidx, 0);
    m_qSpinBox->setParent(this);
    m_qSpinBox->removeEventFilter(this);
    m_qSpinBox->disconnect(this);
    delete m_qSpinBox;
    m_qSpinBox = Q_NULLPTR;
  }
  
  m_editidx = index;
  strname = p_mdl->item(row, CIDX_NAME)->text();
  brange = cal_bit_range(strname);

  if (!brange.v_info.type.compare(tr("RO")))
    return;

  strval = p_mdl->itemFromIndex(index)->text();
  
  ((ssdbg*)m_topwidget)->m_pgphdlg->graph_turnoff();

  qDebug() << "row :  " << row << "col :  " << col;

  switch (col) {
    case CIDX_ADDR :
      break;
    case CIDX_AVAL :
      m_qLineEdit = new QLineEdit;
      m_qLineEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
      m_qLineEdit->setText(strval);
      m_qLineEdit->installEventFilter(this);
      setIndexWidget(p_mdl->index(row, CIDX_AVAL), m_qLineEdit);
      m_qLineEdit->setFocus();
      break;
    case CIDX_NAME :
        ReadNGetValue(p_mdl->itemFromIndex(index)->text());
      return;
      break;
    case CIDX_NVALHEX :
      bitwidth = brange.v_info.bitw;
      if (bitwidth == 1) {
        QString str = p_mdl->itemFromIndex(index)->text();
        SetValuenWrite(strname, (!str.compare(tr("OFF (0)")))? 1 : 0, true);
      }
      else {
        m_qLineEdit = new QLineEdit;
        m_qLineEdit->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        m_qLineEdit->setText(strval);
        m_qLineEdit->installEventFilter(this);
        setIndexWidget(p_mdl->index(row, CIDX_NVALHEX), m_qLineEdit);
        m_qLineEdit->setFocus();
      }
      break;
    case CIDX_NVALDEC :
      bool isnumber = false;
      int ival = strval.toInt(&isnumber, 10);
      if (isnumber) {
        m_qSpinBox = new QSpinBox;
        m_qSpinBox->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
        m_qSpinBox->setRange((int32_t)brange.l_min, (int32_t)brange.l_max);
        m_qSpinBox->setValue(ival);
        m_qSpinBox->setEnabled(true);
        m_qSpinBox->installEventFilter(this);
        m_qSpinBox->setObjectName("spedit");
        setIndexWidget(p_mdl->index(row, CIDX_NVALDEC), m_qSpinBox);
        
        m_qSlider = new QSlider(Qt::Horizontal, this);
        m_qSlider->setRange((int32_t)brange.l_min, (int32_t)brange.l_max);
        m_qSlider->setValue(ival);
        
        connect(m_qSpinBox, SIGNAL(valueChanged(int)), this, SLOT(spinValueChanged(int)));
        connect(m_qSlider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
        setIndexWidget(p_mdl->index((!row)? row+1 : row-1, CIDX_NVALDEC), m_qSlider);
      }
      else {
        m_qComboBox = new QComboBox;
        setIndexWidget(p_mdl->index(row, CIDX_NVALDEC), m_qComboBox);
      }
      break;
  }
}

void tbldlg::spinValueChanged(int i)
{
  QString strdec = tr("");
  QString strhex = tr("");
  QString strval = tr("");
  QString strname = tr("");
  strtregrange brange;
  int row = m_editidx.row();
  QStandardItemModel* p_mdl = geth_mdl(false);

  strname = p_mdl->item(m_editidx.row(), CIDX_NAME)->text();
  brange = cal_bit_range(strname);

  // qDebug() << "spin : " << i;

  SetValuenWrite(strname, i, true);

  if (m_qSlider != Q_NULLPTR) {
    change_from_spin = true;
    m_qSlider->setValue(i);
  }
}

void tbldlg::sliderValueChanged(int i)
{
  QString strdec = tr("");
  QString strhex = tr("");
  QString strval = tr("");
  QString strname = tr("");
  strtregrange brange;
  int row = m_editidx.row();
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  strname = p_mdl->item(m_editidx.row(), CIDX_NAME)->text();
  brange = cal_bit_range(strname);

  // qDebug() << "slider : " << i;
  
  SetValuenWrite(strname, i, true);

  if (m_qSpinBox != Q_NULLPTR) {
    m_qSpinBox->setValue(i);
  }
}

bool tbldlg::getnextrow(int* row, int col)
{
  int startrow = (*row);
  int startcol = col;
  int endrow = 0;
  int endcol = 0;
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  if ((*row) < (p_mdl->rowCount()-1)) {
    int chkcol = columnSpan(startrow, startcol) - 1;
    int chkrow = rowSpan(startrow, startcol) - 1;
    if (chkcol || chkrow)  {
      endrow = startrow + chkrow;
      if (endrow == (p_mdl->rowCount() - 1)) {
        return false;
      }
      else {
        (*row) += (endrow - startrow + 1);
      }
    }
    else {
      (*row)++;
    }
    return true;
  }
  
  return false;
}

bool tbldlg::WriteAllData()
{
  int row = 0;
  QString addr = "";
  QString data = "";
  BYTE num_addr = 0;
  BYTE num_data = 0;
  bool wr_ok = false;
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  do {
    addr = p_mdl->item(row, CIDX_ADDR)->text();
    data = p_mdl->item(row, CIDX_AVAL)->text();
    num_addr = addr.toInt(Q_NULLPTR, 16);
    num_data = data.toInt(Q_NULLPTR, 16);
    brd_byte_wr(num_addr, num_data);
  } while (getnextrow(&row, CIDX_ADDR));

  return wr_ok;
}

bool tbldlg::ReadAllData()
{
  QString addr = "";
  QString data = "";
  BYTE num_addr = 0;
  BYTE num_data = 0;
  int row = 0;
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  do {
    addr = p_mdl->item(row, CIDX_ADDR)->text();
    num_addr = addr.toInt(Q_NULLPTR, 16);
    num_data = brd_byte_rd(num_addr);
    data.sprintf("0x%02x", num_data);
    UpdateAddrData(addr, data);
  } while (getnextrow(&row, CIDX_ADDR));
  
  return true;
}

void tbldlg::showEvent(QShowEvent *ev)
{
  showEventhelper();
  QWidget::showEvent(ev);
}

void tbldlg::showEventhelper()
{
  if (m_qSpinBox != Q_NULLPTR) {
    m_qSpinBox->hide();
    m_qSlider->hide();
  }

  if (m_qLineEdit != Q_NULLPTR) {
    m_qLineEdit->hide();
  }
}

bool tbldlg::eventFilter(QObject *obj, QEvent *evt)
{
  int row = 0;
  int col = 0;
  QString str = tr("");
  QString strname = tr("");
  strtregrange brange = { 0 };
  QStandardItemModel* p_mdl = geth_mdl(false);
  
  qDebug() << evt->type();
  qDebug() << obj->objectName();
  qDebug() << obj;

  if (evt->type() == QEvent::KeyPress) {
    QKeyEvent* key = static_cast<QKeyEvent*>(evt);
    int keyval = key->key();
    if (obj == m_qLineEdit) {
      str = m_qLineEdit->text();
      int sp = m_qLineEdit->selectionStart();
      int ep = m_qLineEdit->selectionEnd();
      int pt = ep - sp;
      int len = str.length();
      row = m_editidx.row();
      col = m_editidx.column();
      strname = p_mdl->item(row, CIDX_NAME)->text();
      brange = cal_bit_range(strname);
      int maxlen = (col != CIDX_AVAL)? (int)floor(ud_log2(brange.l_max) / 4)+1+2 : 4;

      if ((keyval < '0' || keyval > '9') && (keyval < Qt::Key_A || keyval > Qt::Key_F) &&
        keyval != Qt::Key_Backspace && keyval != Qt::Key_X && keyval != Qt::Key_Return &&
        keyval != Qt::Key_Enter)
        return true;
      else if (len == 2) {
        if (!str.compare(tr("0x")) && keyval == Qt::Key_Backspace)
          return true;
      }
      else if (len == 0) {
        if (keyval != Qt::Key_0)
          return true;
      }
      else if (len == 1) {
        if (keyval != Qt::Key_X)
          return true;
      }
      else if (len == pt) {
        m_qLineEdit->setText(tr("0x"));
        m_qLineEdit->setFocus();
        if (keyval == Qt::Key_Backspace) {
          m_qLineEdit->setCursorPosition(2);
          return true;
        }
        else if (col == CIDX_AVAL || col == CIDX_NVALHEX) {
          m_qLineEdit->setCursorPosition(3);
        }
      }
      else if (keyval == Qt::Key_Return || keyval == Qt::Key_Enter) {
        if (col == CIDX_AVAL || col == CIDX_NVALHEX) {
          this->setFocus();
          return true;
        }
      }
      else if (len >= maxlen && keyval != Qt::Key_Backspace) {
        return true;
      }
    }

    if (obj == m_qSpinBox) {
      if (keyval == Qt::Key_Return || keyval == Qt::Key_Enter) {
        this->setFocus();
        return true;
      }
    }
  }
  else if (evt->type() == QEvent::FocusOut) {
    if (obj == m_qSpinBox && focusWidget() != m_qSlider) {
      QModelIndex lidx = p_mdl->index((!m_editidx.row()) ?
        m_editidx.row() + 1 : m_editidx.row() - 1, m_editidx.column());

      if (m_qSpinBox != Q_NULLPTR) {
        // setIndexWidget(lidx, 0);

        // m_qSpinBox->setParent(this);
        // m_qSpinBox->removeEventFilter(this);
        // m_qSpinBox->disconnect(this);
        // m_qSpinBox->deleteLater();
        // m_qSpinBox = Q_NULLPTR;
        // 
        // m_qSlider->setParent(this);
        // m_qSlider->removeEventFilter(this);
        // m_qSlider->disconnect(this);
        // m_qSlider->deleteLater();
        // m_qSlider = Q_NULLPTR;
        m_qSpinBox->hide();
        m_qSlider->hide();

      }
    }
    else if (obj == m_qLineEdit){
      row = m_editidx.row();
      col = m_editidx.column();
      str = m_qLineEdit->text();
      strname = p_mdl->item(row, CIDX_NAME)->text();
      int val = str.toInt(Q_NULLPTR, 16);
      if (col == CIDX_NVALHEX) {
        SetValuenWrite(strname, val, true);
      }
      else if (col == CIDX_AVAL) {
        UpdateAddrData(p_mdl->item(row, CIDX_ADDR)->text(), QString(tr("")).sprintf("0x%02x", val));
      }
      
      if (m_qLineEdit != Q_NULLPTR) {
        setIndexWidget(m_editidx, 0);
        // m_qLineEdit->setParent(this);
        // m_qLineEdit->removeEventFilter(this);
        // m_qLineEdit->deleteLater();
        // m_qLineEdit = Q_NULLPTR;
        m_qLineEdit->hide();
      }
    }
  }

  return QObject::eventFilter(obj, evt);
}
