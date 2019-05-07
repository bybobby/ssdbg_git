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
// File           : tbldlg.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : header file for tableview
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __TBLDBG_H__
#define __TBLDBG_H__

#pragma once

#include <QTableWidget>

QT_FORWARD_DECLARE_CLASS(QStandardItemModel);
QT_FORWARD_DECLARE_CLASS(QStandardItem);
QT_FORWARD_DECLARE_CLASS(QWidget);
QT_FORWARD_DECLARE_CLASS(QMouseEvent);
QT_FORWARD_DECLARE_CLASS(QString);
QT_FORWARD_DECLARE_CLASS(QLineEdit);
QT_FORWARD_DECLARE_CLASS(QSpinBox);
QT_FORWARD_DECLARE_CLASS(QComboBox);
QT_FORWARD_DECLARE_CLASS(QGridLayout);


#define   CIDX_ADDR         0
#define   CIDX_AVAL         1
#define   CIDX_NAME         2
#define   CIDX_NVALHEX      3
#define   CIDX_NVALDEC      4

typedef struct tagInfo
{
	QString type;
	QString adr;	// {width@adr[msb:lsb]@adr[msb:lsb]@...
  uint32_t grid_addr_pos;
  uint32_t grid_name_pos;
  uint32_t bitw;
} strtreginfo;

typedef struct tagRange
{
  bool  valid;
  int64_t l_min;
  int64_t l_max;
  QString fmt;
  strtreginfo v_info;
} strtregrange;


class tbldlg : public QTableView
{
  Q_OBJECT

public:
  tbldlg(QWidget *parent = Q_NULLPTR);
  // QStandardItemModel* m_mdl;
  QString m_readonly;
  void createTable();
  int UpdateNameData(const QString& lpsz, bool bSign);
  bool SetValuenWrite(const QString& lpsz, int64_t value, bool write);
  int UpdateAddrData(const QString& lpsz_adr, const QString& lpsz_data);
  int ReadNGetValue(const QString& lpsz);
  bool WriteAllData();
  bool ReadAllData();

private:
  QWidget* m_topwidget;
  QLineEdit* m_qLineEdit;
  QSpinBox* m_qSpinBox;
  QSlider* m_qSlider;
  QComboBox* m_qComboBox;
  uint32_t  m_mask_adr;
  uint16_t* m_mask_bits;
  QModelIndex m_editidx;
  bool change_from_spin;
  bool change_from_slider;
  strtregrange cal_bit_range(const QString& text);
  void UpdateAllByName();
  bool getnextrow(int* row, int col);
  QHash<QString, strtreginfo>* geth_hash();
  QStandardItemModel* geth_mdl(bool create);
  void showEventhelper();
  
private slots:
  void cellclickedSlot(const QModelIndex & index);
  void spinValueChanged(int i);
  void sliderValueChanged(int i);
  
protected:
  bool eventFilter(QObject *obj, QEvent *evt) override;
  void showEvent(QShowEvent *ev);
};

    
#endif