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
// File           : gphdlg.h
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : graph debugger header file
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#ifndef __GPHDBG_H__
#define __GPHDBG_H__

#pragma once

#include "chart/qchartviewer.h"
#include "chart/qdoublebufferedqueue.h"
#include "common.h"
#include <QDialog>
#include <QDateTime>
#include <Windows.h>

QT_FORWARD_DECLARE_CLASS(QTimer);
QT_FORWARD_DECLARE_CLASS(QScrollBar);
QT_FORWARD_DECLARE_CLASS(QComboBox);
QT_FORWARD_DECLARE_CLASS(QChartViewer);
QT_FORWARD_DECLARE_CLASS(QLabel);
QT_FORWARD_DECLARE_CLASS(gdata);
QT_FORWARD_DECLARE_CLASS(movedata);
QT_FORWARD_DECLARE_CLASS(drawgraph);
QT_FORWARD_DECLARE_CLASS(updatetimer);
QT_FORWARD_DECLARE_CLASS(wcmd);
QT_FORWARD_DECLARE_CLASS(rcmd);
QT_FORWARD_DECLARE_CLASS(dumpp);
QT_FORWARD_DECLARE_CLASS(logp);
QT_FORWARD_DECLARE_CLASS(QFile);

class gphdlg : public QDialog
{
  Q_OBJECT

public:
  gphdlg(QWidget *parent = Q_NULLPTR);
  ~gphdlg();
  
private :
  // The initial full range is set to 60 seconds of data.
  static const int initialFullRange = 20;

  // The maximum zoom in is 10 seconds.
  static const int zoomInLimit = 1;
  
  // static void OnData(void *self, double elapsedTime, int s0, int s1, int s2, int s3, int s4, int s5);
  
  QDateTime m_nextDataTime;           // Used by the random number generator to generate realtime data.
  QComboBox* m_qComboBox[7];

  QScrollBar* m_HScrollBar;
  QPushButton* m_qButtons[8];
  
  double trackLineEndPos;
  bool trackLineIsAtEnd;
  QFile* m_dump_fp;
  double prevEndDate;
  int64_t Frequency;
  int64_t BeginTime;
  int64_t Endtime;
  int64_t Prevtime;
  
  char m_graph_mark0[128];
  char m_graph_mark1[128];
  char m_graph_mark2[128];
  char m_graph_mark3[128];
  char m_graph_mark4[128];
  char m_graph_mark5[128];
  
  void drawChart(QChartViewer *viewer);           // Draw chart
  double trackLineLabel(XYChart *c, int mouseX);    // Draw track cursor
  void updateControls(QChartViewer *viewer);      // Update other controls as viewport changes
  void initaildraw();

public :
  void addReadString(const QString& filename);
  void gen_rdbuf_seq();
  void thread_resume();
  void thread_pause();
  void close_dump();
  void graph_turnoff();

  // The number of samples per data series used in this demo
  static const int sampleSize = 10000;
  
  int m_currentIndex;         // Index of the array position to which new values are added.
  double m_firstdata;
  double m_lastdata;
  
  QDoubleBufferedQueue<double> m_tbuf;
  QDoubleBufferedQueue<double> m_buf[7];
  
  double m_td0[sampleSize];
	double m_d0[7][sampleSize];
  
  double* m_tpsink;
  double* m_psink[7];
  // double* m_tpsrc;
  // double* m_psrc[7];
  short m_dump_cpy[6*DUMP_NUM];
  unsigned char m_log_cpy[SPI_QUEUE_LEGNTH];
  
  gdata*        m_pgdate0;
  gdata*        m_pgdate1;
  wcmd*         m_pwcmd;
  rcmd*         m_prcmd;
  movedata*     m_pmovedata;
  drawgraph*    m_pdrawgraph;
  updatetimer*  m_pupdatetimer;
  dumpp*        m_pdumpp;
  logp*         m_plogp;
  
  QChartViewer* m_ChartViewer;
  QTimer* m_upd_timer;
  QChartViewer* get_hdl_chart(bool newone);
  bool m_bufsel;
  bool m_pause;
  bool m_graph;
  bool m_wfile;
  bool m_binary;
  XYChart* m_xyChart;
  
  int m_graph_width;
  int m_graph_height;
  uint8_t m_draw_chk;
  uint8_t m_prev_draw_chk;

signals :
  void chartupd();
  // void rdsyncinterrupt();
  void takedata();
  
public slots:
  void cbboxIndexChanged(int nID);
  void btnclicked(int nID);
  
  
private slots:
  void onMouseMovePlotArea(QMouseEvent *event);   // Mouse move on plot area
  void onViewPortChanged();                       // Viewport has changed
  void onHScrollBarChanged(int value);            // Scrollbar changed
  void onChartUpdate();   // Update the chart.
  void onRealTimeRead();
  void onTakeData();
};


#endif
