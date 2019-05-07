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
// File           : gphdlg.cpp
// Version        : V0.1(initial)                                    
//                                                                   
// Description    : Graph Viewer Dialog
// Author         : noah                                             
//                                                                   
// Update History                                                    
//        2018-04-17 (V0.1)   : Initial creation            
//                                                                   
/////////////////////////////////////////////////////////////////////

#include "ctrlid.h"
#include "childdlg/gphdlg.h"
#include "childdlg/dlghandle.h"
#include "childdlg/ssdbg.h"
#include "childdlg/tbldlg.h"
#include "childdlg/dbgdlg.h"
#include "chart/chartdir.h"
#include "chart/mthread.h"
#include "ftdi/ftdi_usb2spi.h"
#include "ftdi/spi_access.h"
#include "serial/uart_conn.h"
#include "serial/uart_acc.h"
#include "mth/rdyqueue.h"
#include "mth/mtfile.h"
#include <algorithm>
#include <string>
#include <cstdlib>
#include <math.h>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <algorithm>

#include <QLayout>
#include <QTimer>
#include <QScrollBar>
#include <QComboBox>
#include <QWidget>
#include <QPainter>
#include <QDebug>
#include <QSignalMapper>
#include <QThread>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>

static const int DataInterval = 20;

using namespace std;

typedef unsigned long DWORD;

DWORD refcolor[] = {
  0x00ff0000,
  0x0000ff00,
  0x004444ff,
  0x00999900,
  0x00009999,
  0x00990099
};

gphdlg::gphdlg(QWidget *parent)
	: QDialog(parent)
{
  QVBoxLayout* chartLayout = new QVBoxLayout;
  QHBoxLayout* dlgLayout = new QHBoxLayout;
  QGridLayout* ctrlLayout = new QGridLayout;
  QSize dlgSize = this->parentWidget()->size();
  QLabel* qLabelPixmap[8];
  QLabel* qLabelText[8];
  QPixmap qPixmap(QSize(11,11));
  QPainter* qPainter = NULL;
  QSignalMapper *signalMapper = new QSignalMapper(this);
  QSignalMapper *sigMapBtn = new QSignalMapper(this);
  
  int btncnt = GPH_BTN_GRAPH;
  
  m_graph_width = dlgSize.width()-20;
  m_graph_height = dlgSize.height()-150;
  
  // Chart Viewer
  m_ChartViewer = new QChartViewer(this);
  connect(m_ChartViewer, SIGNAL(viewPortChanged()), SLOT(onViewPortChanged()));
  connect(m_ChartViewer, SIGNAL(mouseMovePlotArea(QMouseEvent*)),
        SLOT(onMouseMovePlotArea(QMouseEvent*)));
        
  // Horizontal scroll bar
  m_HScrollBar = new QScrollBar(Qt::Horizontal, this);
  connect(m_HScrollBar, SIGNAL(valueChanged(int)), SLOT(onHScrollBarChanged(int)));
  
  int cbncnt = GPH_CBBOX_PTA;
  
  connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(cbboxIndexChanged(int)));
  connect(sigMapBtn, SIGNAL(mapped(int)), this, SLOT(btnclicked(int)));
  
  for (int i=0; i<3; i++){
    m_qButtons[i] = new QPushButton((i==0)? tr("GRAPH ON") : 
                                    (i==1)? tr("DUMP ON") : tr("DECIMAL"), this);
    connect(m_qButtons[i], SIGNAL(clicked()), sigMapBtn, SLOT(map()));
    sigMapBtn->setMapping(m_qButtons[i], btncnt++);
  }
  
  ctrlLayout->addWidget(new QLabel, 0, 0, 1, 2);
  
  // for (int i=0; i<6; i++){
    // qLabelText[i] = new QLabel;
    // qLabelText[i]->setText(QString(tr("")).sprintf("pt %c", 'a'+i));
    // qLabelText[i]->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
    // qLabelPixmap[i] = new QLabel;
    // qLabelPixmap[i]->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    // qLabelPixmap[i]->setStyleSheet(QStringLiteral("QLabel{ max-width: 11px; }"));
    // qPainter = new QPainter(&qPixmap);
    // qPainter->setBrush(QColor(refcolor[i]));
    // qPainter->setPen(QColor(0, 0, 0, 255));
    // qPainter->drawRect(0, 0, 10, 10);
    // delete qPainter;
    // qLabelPixmap[i]->setPixmap(qPixmap);
    // m_qComboBox[i] = new QComboBox;
    // m_qComboBox[i]->setObjectName("gphcombox");
    // connect(m_qComboBox[i], SIGNAL(currentIndexChanged(int)), signalMapper, SLOT(map()));
    // signalMapper->setMapping(m_qComboBox[i], cbncnt++);
    // ctrlLayout->addWidget(qLabelPixmap[i], 2+(i*2), 0, 1, 1);
    // ctrlLayout->addWidget(qLabelText[i],   2+(i*2), 1, 1, 1);
    // ctrlLayout->addWidget(m_qComboBox[i],  3+(i*2), 0, 1, 2);
  // }
  
  cbncnt = GPH_MOUSE_USAGE;
  m_qComboBox[6] = new QComboBox;
  m_qComboBox[6]->setObjectName("gphcombox");
  connect(m_qComboBox[6], SIGNAL(currentIndexChanged(int)), signalMapper, SLOT(map()));
  signalMapper->setMapping(m_qComboBox[6], cbncnt++);
  ctrlLayout->addWidget(m_qComboBox[6], 1, 0, 1, 2);
  
  ctrlLayout->addWidget(m_qButtons[0], 4, 0, 1, 2);
  ctrlLayout->addWidget(m_qButtons[1], 5, 0, 1, 2);
  ctrlLayout->addWidget(m_qButtons[2], 6, 0, 1, 2);
  ctrlLayout->addWidget(new QLabel, 7, 0, 1, 2);
  
  memset(m_td0, 0x00, sizeof(double)*sampleSize);
  
  for (int m=0; m<7; m++)
    memset(m_d0[m], Chart::NoValue, sizeof(int)*sampleSize);
  
  m_tpsink = m_td0;
  
  for (int k=1; k<7; k++) {
    m_psink[k] = m_d0[k];
  }
  
  m_bufsel = false;
  
  m_currentIndex = 0;
  trackLineEndPos = 0;
  trackLineIsAtEnd = true;
  
  m_lastdata = 0;
  m_firstdata = 0;
  m_xyChart = NULL;
  m_draw_chk = 0;
  m_prev_draw_chk = 0;
  m_graph = false;
  m_wfile = false;
  m_binary = false;
  
  m_ChartViewer->setUpdateInterval(1);
  m_ChartViewer->setUpdatesEnabled(true);

  // Enable mouse wheel zooming by setting the zoom ratio to 1.1 per wheel event
  m_ChartViewer->setMouseWheelZoomRatio(1.1);
    
  // Set m_nextDataTime to the current time. It is used by the real time random number
  // generator so it knows what timestamp should be used for the next data point.
  m_nextDataTime = QDateTime::currentDateTime();
  
  // m_pgdate0 = new gdata(onTakeData, this);
  // m_pgdate0->start();

  // m_pdumpp = new dumpp(this);
  // m_pdumpp->start();
  // 
  // m_plogp = new logp(this);
  // m_plogp->start();
  
  // m_pmovedata = new movedata(NULL, this);
  // m_pmovedata->start();
  
  // m_pdrawgraph = new drawgraph(NULL, this);
  // m_pdrawgraph->start();
  
  m_pupdatetimer = new updatetimer(NULL, this);
  m_pupdatetimer->start();

  QueryPerformanceFrequency((LARGE_INTEGER *)&Frequency );

  connect(m_pupdatetimer, SIGNAL(upd_trigger()), SLOT(onChartUpdate()));

  // Set up the chart update timer
  // m_upd_timer = new QTimer(this);
  // connect(m_upd_timer, SIGNAL(timeout()), SLOT(onChartUpdate()));

  // The chart update rate is set to 100ms
  // m_upd_timer->start(100);
    
  chartLayout->addWidget(m_ChartViewer);
  chartLayout->addWidget(m_HScrollBar);
  dlgLayout->addLayout(chartLayout);
  dlgLayout->addLayout(ctrlLayout);
  
  this->setLayout(dlgLayout);

  initaildraw();
  time_increment(true);

  sprintf_s(m_graph_mark0, "pt a <*bgColor=FFCCCC*>");
  sprintf_s(m_graph_mark1, "pt b <*bgColor=CCFFCC*>");
  sprintf_s(m_graph_mark2, "pt c <*bgColor=CCCCFF*>");
  sprintf_s(m_graph_mark3, "pt d <*bgColor=9999CC*>");
  sprintf_s(m_graph_mark4, "pt e <*bgColor=CC9999*>");
  sprintf_s(m_graph_mark5, "pt f <*bgColor=99CC99*>");
  
  // connect(this, SIGNAL(chartupd()), this, SLOT(onChartUpdate()));
  // connect(this, SIGNAL(rdsyncinterrupt()), this, SLOT(onRealTimeRead()));
  connect(this, SIGNAL(takedata()), this, SLOT(onTakeData()));
}

gphdlg::~gphdlg()
{
  delete m_ChartViewer->getChart();
  // delete m_pgdate0;
  // delete m_pgdate1;
  // delete m_pwcmd;
  // delete m_prcmd;
  // delete m_pmovedata;
  // delete m_pdrawgraph;
  m_pupdatetimer->terminate();
  m_pupdatetimer->wait();

  delete m_pupdatetimer;
}

//
// View port changed event
//
void gphdlg::onViewPortChanged()
{
  if (!m_graph) {
    m_pupdatetimer->pause();
    // m_pgdate0->pause();
    return;
  }
  
  // In addition to updating the chart, we may also need to update other controls that
  // changes based on the view port.
  updateControls(m_ChartViewer);
  
  // qDebug() << "OnViewPortChanged !!  "<< __func__ << ",   line : " << __LINE__;
  // Update the chart if necessary
  if (m_ChartViewer->needUpdateChart())
    drawChart(m_ChartViewer);
}

//
// Update controls in the user interface when the view port changed
//
void gphdlg::updateControls(QChartViewer *viewer)
{
  // The logical length of the scrollbar. It can be any large value. The actual value does
  // not matter.
  // const int scrollBarLen = 1000000000;
  const int scrollBarLen = (int)(((uint64_t)1<<29)-1);

  // Update the horizontal scroll bar
  m_HScrollBar->setEnabled(viewer->getViewPortWidth() < 1);
  m_HScrollBar->setPageStep((int)ceil(viewer->getViewPortWidth() * scrollBarLen));
  m_HScrollBar->setSingleStep(min(scrollBarLen / 100, m_HScrollBar->pageStep()));
  m_HScrollBar->setRange(0, scrollBarLen - m_HScrollBar->pageStep());
  m_HScrollBar->setValue((int)(0.5 + viewer->getViewPortLeft() * scrollBarLen));
}

//
// User clicks on the the horizontal scroll bar
//
void gphdlg::onHScrollBarChanged(int value)
{
  if (!m_ChartViewer->isInViewPortChangedEvent())
  {
    // Set the view port based on the scroll bar
    int scrollBarLen = m_HScrollBar->maximum() + m_HScrollBar->pageStep();
    m_ChartViewer->setViewPortLeft(value / (double)scrollBarLen);
    
    // Update the chart display without updating the image maps. (We can delay updating
    // the image map until scrolling is completed and the chart display is stable.)
    m_ChartViewer->updateViewPort(true, false);
  }
}

//
// Draw the chart and display it in the given viewer
//
void gphdlg::drawChart(QChartViewer *viewer)
{
  
  double viewPortStartDate = viewer->getValueAtViewPort("x", viewer->getViewPortLeft());
  double viewPortEndDate = viewer->getValueAtViewPort("x", viewer->getViewPortLeft() +
    viewer->getViewPortWidth());
  
  // if (prevEndDate == viewPortEndDate && viewPortEndDate != 20.001) {
  //   prevEndDate = viewPortEndDate;
  //   return;
  // }
  // prevEndDate = viewPortEndDate;
  // qDebug() << "view end data :  "  << viewPortEndDate 
            // << ",    portleft : " << viewer->getViewPortLeft()
            // << ",    portwidth : " << viewer->getViewPortWidth();
  
  // Extract the part of the data arrays that are visible.
  DoubleArray viewPortTimeStamps;
  DoubleArray viewPortDataSeriesA;
  DoubleArray viewPortDataSeriesB;
  DoubleArray viewPortDataSeriesC;
  // DoubleArray viewPortDataSeriesD;
  // DoubleArray viewPortDataSeriesE;
  // DoubleArray viewPortDataSeriesF;

  if (m_currentIndex > 0) {
    // Get the array indexes that corresponds to the visible start and end dates
    int startIndex = (int)floor(Chart::bSearch(DoubleArray(m_tpsink, m_currentIndex), viewPortStartDate));
    int endIndex = (int)ceil(Chart::bSearch(DoubleArray(m_tpsink, m_currentIndex), viewPortEndDate));
    int noOfPoints = endIndex - startIndex + 1;

    // Extract the visible data
    viewPortTimeStamps = DoubleArray(m_tpsink + startIndex, noOfPoints);
    viewPortDataSeriesA = DoubleArray(m_psink[1] + startIndex, noOfPoints);
    viewPortDataSeriesB = DoubleArray(m_psink[2] + startIndex, noOfPoints);
    viewPortDataSeriesC = DoubleArray(m_psink[3] + startIndex, noOfPoints);
    // viewPortDataSeriesD = DoubleArray(m_psink[4] + startIndex, noOfPoints);
    // viewPortDataSeriesE = DoubleArray(m_psink[5] + startIndex, noOfPoints);
    // viewPortDataSeriesF = DoubleArray(m_psink[6] + startIndex, noOfPoints);
    trackLineEndPos = m_tpsink[m_currentIndex - 1];
  }
    
  //================================================================================
  // Output the chart
  //================================================================================
  
  XYChart *xyChart = new XYChart((std::max)(100, m_graph_width),
        (std::max)(190, m_graph_height), Chart::Transparent, Chart::Transparent, 0);
        
  // We need to update the track line too. If the mouse is moving on the chart (eg. if 
  // the user drags the mouse on the chart to scroll it), the track line will be updated
  // in the MouseMovePlotArea event. Otherwise, we need to update the track line here.
  
  // Set the plotarea at (55, 62) and of size 520 x 175 pixels. Use white (ffffff) 
  // background. Enable both horizontal and vertical grids by setting their colors to 
  // grey (cccccc). Set clipping mode to clip the data lines to the plot area.
  xyChart->setPlotArea(65, 50, xyChart->getWidth() - 85, xyChart->getHeight() - 85, 0x1c1c1c, -1, 0, 0xaaaaaa, 0xaaaaaa);
  xyChart->setClipping();

  // Add a title to the chart using 15 pts Times New Roman Bold Italic font, with a light
  // grey (dddddd) background, black (000000) border, and a glass like raised effect.
  // TextBox* title = xyChart->addTitle("readout data", ":/res/fonts/Everson Mono Bold.ttf", 12);
  // title->setBackground(Chart::Transparent, Chart::Transparent, Chart::flatBorder(1));
  // //title->setMargin(5, 5, 10, 1);
  // title->setFontColor(0xe0e0e0);
      // )->setBackground(0xdddddd, 0x000000, Chart::glassEffect());

  // Add a legend box at the top of the plot area with 9pts Arial Bold font. We set the 
  // legend box to the same width as the plot area and use grid layout (as opposed to 
  // flow or top/down layout). This distributes the 3 legend icons evenly on top of the 
  // plot area.
  LegendBox *b = xyChart->addLegend2(75, 5, 6, ":/res/fonts/Everson Mono Bold.ttf", 11);
  b->setBackground(Chart::Transparent, Chart::Transparent);
  b->setWidth(440);
  b->setFontColor(0xe0e0e0);

  // Configure the y-axis with a 10pts Arial Bold axis title
  TextBox* yLabel = xyChart->yAxis()->setTitle("ADC level", ":/res/fonts/Everson Mono Bold.ttf", 12);
  yLabel->setFontColor(0xe0e0e0);
  yLabel->setMargin(2, 2, 2, 2);
  xyChart->yAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);

  // Configure the x-axis to auto-scale with at least 75 pixels between major tick and 
  // 15  pixels between minor ticks. This shows more minor grid lines on the chart.
  xyChart->xAxis()->setTickDensity(75, 15);
  xyChart->xAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);

  // Set the y-axis tick length to 0 to disable the tick and put the labels closer to the axis.
  xyChart->yAxis()->setTickLength(0);

  // Set the axes width to 2 pixels
  xyChart->xAxis()->setWidth(2);
  xyChart->yAxis()->setWidth(2);

  // Create a line layer to plot the lines
  LineLayer *layer = xyChart->addLineLayer();
  layer->setLineWidth(2);

  // Set the x-axis label format
  // xyChart->xAxis()->setLabelFormat("{value}");

  // Set the x-axis as a date/time axis with the scale according to the view port x range.
  viewer->syncDateAxisWithViewPort("x", xyChart->xAxis());
  
  // Now we add the data to the chart. 

  // In this demo, we do not have too many data points. In real code, the chart may contain a lot
  // of data points when fully zoomed out - much more than the number of horizontal pixels in this
  // plot area. So it is a good idea to use fast line mode.
  layer->setFastLineMode(true);

  // Now we add the 2 data series to the line layer with red (ff0000) and green (00cc00) colors
  layer->setXData(viewPortTimeStamps);
  layer->addDataSet(viewPortDataSeriesA, 0xff0000, m_graph_mark0);
  layer->addDataSet(viewPortDataSeriesB, 0x00ff00, m_graph_mark1);
  layer->addDataSet(viewPortDataSeriesC, 0x4444ff, m_graph_mark2);
  
  if (m_currentIndex > 0)
    xyChart->xAxis()->setDateScale(viewPortStartDate, viewPortEndDate);
  
  // In this demo, we rely on ChartDirector to auto-label the axis. We ask ChartDirector to ensure
  // the x-axis labels are at least 75 pixels apart to avoid too many labels.
  xyChart->xAxis()->setTickDensity(55);
  xyChart->yAxis()->setTickDensity(35);
  
  // We use "hh:nn:ss" as the axis label format.
  xyChart->xAxis()->setLabelFormat("{value|nn:ss}");
  
  // We make sure the tick increment must be at least 1 second.
  xyChart->xAxis()->setMinTickInc(2);
  
  // Set the auto-scale margin to 0.05, and the zero affinity to 0.6
  // xyChart->yAxis()->setAutoScale(0.05, 0.05, 0.6);
  xyChart->yAxis()->setLinearScale(-600, 600, 300, 150);
  
  if (!viewer->isInMouseMoveEvent())
    trackLineLabel(xyChart, trackLineIsAtEnd ? xyChart->getWidth() : viewer->getPlotAreaMouseX());
  
  delete viewer->getChart();
  viewer->setChart(xyChart);
}

//
// Draw track line with data labels
//
double gphdlg::trackLineLabel(XYChart *c, int mouseX)
{
  // Clear the current dynamic layer and get the DrawArea object to draw on it.
  DrawArea *d = c->initDynamicLayer();

  // The plot area object
  PlotArea *plotArea = c->getPlotArea();

  // Get the data x-value that is nearest to the mouse, and find its pixel coordinate.
  double xValue = c->getNearestXValue(mouseX);
  int xCoor = c->getXCoor(xValue);
  
	if (xCoor < plotArea->getLeftX())
		return xValue;

  // Draw a vertical track line at the x-position
  d->vline(plotArea->getTopY(), plotArea->getBottomY(), xCoor, 0x888888);

    // Draw a label on the x-axis to show the track line position.
	ostringstream xlabel;
	xlabel << "<*font,bgColor=000000*> " << c->xAxis()->getFormattedLabel(xValue + 0.00499,
		"hh:nn:ss.ff") << " <*/font*>";
	TTFText *t = d->text(xlabel.str().c_str(), "arialbd.ttf", 10);

  // Restrict the x-pixel position of the label to make sure it stays inside the chart image.
  int xLabelPos = max(0, min(xCoor - t->getWidth() / 2, c->getWidth() - t->getWidth()));
	t->draw(xLabelPos, plotArea->getBottomY() + 6, 0xffffff);
	t->destroy();	

  // Iterate through all layers to draw the data labels
  for (int i = 0; i < c->getLayerCount(); ++i) {
    Layer *layer = c->getLayerByZ(i);

    // The data array index of the x-value
    int xIndex = layer->getXIndexOf(xValue);

    // Iterate through all the data sets in the layer
    for (int j = 0; j < layer->getDataSetCount(); ++j) 	{
      DataSet *dataSet = layer->getDataSetByZ(j);
      const char *dataSetName = dataSet->getDataName();

      // Get the color, name and position of the data label
      int color = dataSet->getDataColor();
      int yCoor = c->getYCoor(dataSet->getPosition(xIndex), dataSet->getUseYAxis());

      // Draw a track dot with a label next to it for visible data points in the plot area
      if ((yCoor >= plotArea->getTopY()) && (yCoor <= plotArea->getBottomY()) && (color !=
          Chart::Transparent) && dataSetName && *dataSetName) {
        d->circle(xCoor, yCoor, 4, 4, color, color);

        ostringstream label;
        label << "<*font,bgColor=" << hex << color << "*> " 
          << c->formatValue(dataSet->getValue(xIndex), "{value|P4}") << " <*font*>";
        t = d->text(label.str().c_str(), "arialbd.ttf", 10);
			
        // Draw the label on the right side of the dot if the mouse is on the left side the
        // chart, and vice versa. This ensures the label will not go outside the chart image.
        if (xCoor <= (plotArea->getLeftX() + plotArea->getRightX()) / 2)
          t->draw(xCoor + 6, yCoor, 0xffffff, Chart::Left);
        else
          t->draw(xCoor - 6, yCoor, 0xffffff, Chart::Right);
          t->destroy();
      }
    }
  }
  
  return xValue;
}

void gphdlg::initaildraw()
{
  XYChart* xyChart;
  DoubleArray viewPortTimeStamps;
  double time[100];

  for (int i = 0; i < 100; i++) {
    time[i] = i*20 / 100.0;
  }

  viewPortTimeStamps = DoubleArray(time, 100);

  // Create an XYChart object extending to the container boundary, with a minimum of 300 x 180 
  xyChart = new XYChart((std::max)(100, m_graph_width),
        (std::max)(190, m_graph_height), Chart::Transparent, Chart::Transparent, 0);

  //================================================================================
  // Configure overall chart appearance.
  //================================================================================
  // Set the plotarea at (55, 62) and of size 520 x 175 pixels. Use white (ffffff) 
  // background. Enable both horizontal and vertical grids by setting their colors to 
  // grey (cccccc). Set clipping mode to clip the data lines to the plot area.
  xyChart->setPlotArea(65, 50, xyChart->getWidth() - 85, xyChart->getHeight() - 85, 0x1c1c1c, -1, 0, 0xaaaaaa, 0xaaaaaa);
  xyChart->setClipping();

  // Add a legend box at the top of the plot area with 9pts Arial Bold font. We set the 
  // legend box to the same width as the plot area and use grid layout (as opposed to 
  // flow or top/down layout). This distributes the 3 legend icons evenly on top of the 
  // plot area.
  LegendBox *b = xyChart->addLegend2(75, 5, 6, ":/res/fonts/Everson Mono Bold.ttf", 11);
  b->setBackground(Chart::Transparent, Chart::Transparent);
  b->setWidth(440);
  b->setFontColor(0xe0e0e0);

  // Configure the y-axis with a 10pts Arial Bold axis title
  TextBox* yLabel = xyChart->yAxis()->setTitle("ADC level", ":/res/fonts/Everson Mono Bold.ttf", 12);
  yLabel->setFontColor(0xe0e0e0);
  yLabel->setMargin(2, 2, 2, 2);
  xyChart->yAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);

  // Configure the x-axis to auto-scale with at least 75 pixels between major tick and 
  // 15  pixels between minor ticks. This shows more minor grid lines on the chart.
  xyChart->xAxis()->setTickDensity(75, 15);
  xyChart->xAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);

  // Set the y-axis tick length to 0 to disable the tick and put the labels closer to the axis.
  xyChart->yAxis()->setTickLength(0);

  // Set the axes width to 2 pixels
  xyChart->xAxis()->setWidth(2);
  xyChart->yAxis()->setWidth(2);

  // Create a line layer to plot the lines
  LineLayer *layer = xyChart->addLineLayer();
  layer->setLineWidth(2);

  // Set the x-axis label format
  xyChart->xAxis()->setLabelFormat("{value}");

  // Set the x-axis as a date/time axis with the scale according to the view port x range.
  // m_ChartViewer->syncDateAxisWithViewPort("x", xyChart->xAxis());

  // Now we add the data to the chart. 

  // In this demo, we do not have too many data points. In real code, the chart may contain a lot
  // of data points when fully zoomed out - much more than the number of horizontal pixels in this
  // plot area. So it is a good idea to use fast line mode.
  layer->setFastLineMode(true);

  // Now we add the 2 data series to the line layer with red (ff0000) and green (00cc00) colors
  layer->setXData(viewPortTimeStamps);

  // In this demo, we rely on ChartDirector to auto-label the axis. We ask ChartDirector to ensure
  // the x-axis labels are at least 75 pixels apart to avoid too many labels.
  xyChart->xAxis()->setTickDensity(55);
  xyChart->yAxis()->setTickDensity(35);
  
  // We use "hh:nn:ss" as the axis label format.
  xyChart->xAxis()->setLabelFormat("{value|nn:ss}");
  
  // We make sure the tick increment must be at least 1 second.
  xyChart->xAxis()->setMinTickInc(1);
  
  // Set the auto-scale margin to 0.05, and the zero affinity to 0.6
  // xyChart->yAxis()->setAutoScale(0.05, 0.05, 0.6);
  xyChart->yAxis()->setLinearScale(-600, 600, 300, 150);
  
  m_ChartViewer->setChart(xyChart);
}

//
// Update the chart and the viewport periodically
//
void gphdlg::onChartUpdate()
{
  QChartViewer *viewer = m_ChartViewer;
  int count;
  double *tbufptr;
  double *bufptr[7];
  
  // Enables auto scroll if the viewport is showing the latest data before the update
  bool autoScroll = (m_currentIndex > 0) && (0.001 + viewer->getValueAtViewPort("x",
    viewer->getViewPortLeft() + (viewer->getViewPortWidth())) >= m_tpsink[m_currentIndex - 1]);
  
  if ((count = m_tbuf.get(&tbufptr)) <= 0)
    return;
    
  for (int i=1; i<4; i++){
    m_buf[i].get(&bufptr[i]);
  }
  
  // if data arrays have insufficient space, we need to remove some old data.
  if (m_currentIndex + count >= sampleSize)
  {
    // For safety, we check if the queue contains too much data than the entire data arrays. If
    // this is the case, we only use the latest data to completely fill the data arrays.
    if (count > sampleSize)
    {
      tbufptr += count - sampleSize;
      for (int i=1; i<4; i++)
        bufptr[i] += count - sampleSize;
      count = sampleSize;
    }
  
    // Remove oldest data to leave space for new data. To avoid frequent removal, we ensure at
    // least 5% empty space available after removal.
    int originalIndex = m_currentIndex;
    m_currentIndex = sampleSize * 95 / 100 - 1;
    if (m_currentIndex > sampleSize - count)
      m_currentIndex = sampleSize - count;
  
  
    for (int i=0; i < m_currentIndex; ++i){
      int srcIndex = i + originalIndex - m_currentIndex;
      m_tpsink[i] = m_tpsink[srcIndex];
      for (int l=1; l<4; l++) {
        m_psink[l][i] = m_psink[l][srcIndex];
      }
    }
    
  }
  
  // Append the data from the queue to the data arrays
  for (int n = 0; n < count; ++n) {
    m_tpsink[m_currentIndex] = tbufptr[n];
    for (int i=1; i<4; i++) {
      m_psink[i][m_currentIndex] = bufptr[i][n];
    }
    ++m_currentIndex;
  }
  
  //
  // As we added more data, we may need to update the full range of the viewport.
  //
  double startDate = (double) m_tpsink[0];
  double endDate = (double) m_tpsink[m_currentIndex-1];

  // Use the initialFullRange (which is 60 seconds in this demo) if this is sufficient.
  double duration = endDate - startDate;
  if (duration < initialFullRange)
    endDate = startDate + initialFullRange;

  // Update the new full data range to include the latest data
  bool axisScaleHasChanged = viewer->updateFullRangeH("x", startDate, endDate,
      Chart::KeepVisibleRange);

  // qDebug() << " locked by : " << __func__ << ",   line : " << __LINE__;
  // set_lock_gphdlg(true);
  if (autoScroll)
  {
    // Scroll the viewport if necessary to display the latest data
    double viewPortEndPos = viewer->getViewPortAtValue("x", (double) endDate);
    if (viewPortEndPos > viewer->getViewPortLeft() + viewer->getViewPortWidth())
    {
      viewer->setViewPortLeft(viewPortEndPos - viewer->getViewPortWidth());
      axisScaleHasChanged = true;
    }
  }

  // Set the zoom in limit as a ratio to the full range
  viewer->setZoomInWidthLimit(zoomInLimit / (viewer->getValueAtViewPort("x", 1) -
          viewer->getValueAtViewPort("x", 0)));
  
  // Prevtime = BeginTime;
  // QueryPerformanceCounter((LARGE_INTEGER *) &BeginTime ); 
  // qDebug() << "chart data update !!  "<< __func__ << ",   line : " << __LINE__ << ",   time : " << 
    // (double)(BeginTime - Prevtime) / (double)((LARGE_INTEGER *)&Frequency)->QuadPart;
  // Trigger the viewPortChanged event. Updates the chart if the axis scale has changed
  // (scrolling or zooming) or if new data are added to the existing axis scale.
  viewer->updateViewPort(axisScaleHasChanged || (duration < initialFullRange), false);
}

//
// Draw track cursor when mouse is moving over plotarea
//
void gphdlg::onMouseMovePlotArea(QMouseEvent *)
{
  double trackLinePos = trackLineLabel((XYChart *)m_ChartViewer->getChart(),
        m_ChartViewer->getPlotAreaMouseX());
  trackLineIsAtEnd = (m_currentIndex <= 0) || (trackLinePos == trackLineEndPos);
  m_ChartViewer->updateDisplay();
}


void gphdlg::addReadString(const QString& filename)
{
  QString readonly = filename;
  
  QPixmap qPixmap(QSize(9, 9));
  QPainter* qPainter = NULL;
  QIcon* qIcon = new QIcon();

  QStringList parse = readonly.split("@");
  // QStringListIterator itr(parse);
  // itr.next();

  for (int i = 0; i < 6; i++) {
    qPainter = new QPainter(&qPixmap);
    qPainter->setBrush(QColor(0, 0, 0, 255));
    qPainter->setPen(QColor(0, 0, 0, 255));
    qPainter->drawRect(0, 0, 8, 8);
    delete qPainter;
    qIcon->addPixmap(qPixmap);
    // m_qComboBox[i]->addItem(*qIcon, "OFF");
  }

  QStringList ttmp = (QStringList() << tr("XA_OUT")
                        << tr("YA_OUT")
                        << tr("ZA_OUT")
                        << tr("XG_OUT")
                        << tr("YG_OUT")
                        << tr("ZG_OUT"));
  // QStringListIterator itr(ttmp);
                        
  // while (itr.hasNext()) {
    // QString tokadr = itr.next();
    
    // if (!tokadr.isEmpty()) {
      // for (int i = 0; i < 6; i++) {
        // QString specific = itr.next();
        // qPainter = new QPainter(&qPixmap);
        // qPainter->setBrush(QColor(refcolor[i]));
        // qPainter->setPen(QColor(0, 0, 0, 255));
        // qPainter->drawRect(0, 0, 8, 8);
        // delete qPainter;
        // qIcon->addPixmap(qPixmap);
        // m_qComboBox[i]->addItem(*qIcon, tokadr);
        // m_qComboBox[i]->addItem(*qIcon, specific);
      // }
    // }
  // }
  ttmp = (QStringList() << tr("Pointer")
        << tr("Zoom In")
        << tr("Zoom Out"));
  m_qComboBox[6]->addItems(ttmp);
}

void gphdlg::cbboxIndexChanged(int nID)
{
  QComboBox *pComboBox = m_qComboBox[GPH_CBID(nID)];
  int cidx = pComboBox->currentIndex();
  
  m_prev_draw_chk = m_draw_chk;
  switch(nID){
  case GPH_CBBOX_PTA :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 0);
    else
      m_draw_chk &= ~(0x01 << 0);
    break;
  case GPH_CBBOX_PTB :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 1);
    else
      m_draw_chk &= ~(0x01 << 1);
    break;
  case GPH_CBBOX_PTC :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 2);
    else
      m_draw_chk &= ~(0x01 << 2);
    break;
  case GPH_CBBOX_PTD :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 3);
    else
      m_draw_chk &= ~(0x01 << 3);
    break;
  case GPH_CBBOX_PTE :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 4);
    else
      m_draw_chk &= ~(0x01 << 4);
    break;
  case GPH_CBBOX_PTF :
    if (cidx > 0)
      m_draw_chk |= (0x01 << 5);
    else
      m_draw_chk &= ~(0x01 << 5);
    break;
  case GPH_MOUSE_USAGE :
      if (cidx == 0) {
        // qDebug() << __func__ << ",   line : " << __LINE__;
        // // set_lock_gphdlg(true);
        m_ChartViewer->setMouseUsage(Chart::MouseUsageScroll);
        // // set_lock_gphdlg(false);
      }
      else if (cidx == 1) {
        // qDebug() << __func__ << ",   line : " << __LINE__;
        // // set_lock_gphdlg(true);
        m_ChartViewer->setMouseUsage(Chart::MouseUsageZoomIn);
        // // set_lock_gphdlg(false);
      }
      else if (cidx == 2) {
        // qDebug() << __func__ << ",   line : " << __LINE__;
        // // set_lock_gphdlg(true);
        m_ChartViewer->setMouseUsage(Chart::MouseUsageZoomOut);
        // // set_lock_gphdlg(false);
      }
    break;
  }

  if (m_draw_chk != 0 && m_prev_draw_chk == 0) {
    thread_resume();
  }
  else if (m_draw_chk == 0) {
    thread_pause();
    m_prev_draw_chk = 0;
  }
}

void gphdlg::graph_turnoff()
{
  if (m_graph) {
    m_draw_chk = 0;
    m_qButtons[GPH_BTNID(GPH_BTN_GRAPH)]->setText(tr("GRAPH ON"));
    ((ssdbg*)(this->parentWidget()))->intr_read(false);
    thread_pause();
    m_graph = false;
  }
}

void gphdlg::btnclicked(int nID)
{
  mtfile* fhdl;

  switch (nID)
  {
    case GPH_BTN_GRAPH :
      if (!m_graph) {        
        m_qButtons[GPH_BTNID(GPH_BTN_GRAPH)]->setText(tr("GRAPH OFF"));
        m_draw_chk = 7;
        ((ssdbg*)(this->parentWidget()))->intr_read(true);
        thread_resume();
        m_graph = true;
      }
      else {
        // if (m_dump_fp != NULL) {
        //   if (m_dump_fp->isOpen()) {
        //     m_dump_fp->close();
        //     m_wfile = false;
        //     UserData* exit_userdata = new UserData(1, 0, 0);
        //     push_file_queue(exit_userdata);
        // 
        //     m_qButtons[GPH_BTNID(GPH_BTN_DUMP)]->setText(tr("DUMP ON"));
        //   }
        // }
        graph_turnoff();
      }
    break;
    case GPH_BTN_DUMP :
      if (!m_wfile) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
          "./",
          tr("csv files (*.csv)"));

        if (fileName.count() < 1)
          return;

        m_dump_fp = new QFile(fileName);
        if (!m_dump_fp->open(QIODevice::WriteOnly | QIODevice::Text))
          return;

        std::string str = fileName.toStdString();
        get_file_id(true);
        m_wfile = true;
        get_dumpf_stream(2, str.c_str());
        m_qButtons[GPH_BTNID(GPH_BTN_DUMP)]->setText(tr("DUMP ING..."));
      }
      else {
        if (m_dump_fp->isOpen())
          m_dump_fp->close();

        m_wfile = false;
        UserData* exit_userdata = new UserData(1, 0, 0);
        push_file_queue(exit_userdata);

        m_qButtons[GPH_BTNID(GPH_BTN_DUMP)]->setText(tr("DUMP ON"));
      }
      // m_pdumpp->set_textstream(new QTextStream(m_dump_fp));
      // m_pdumpp->resume();
    break;
    case GPH_BTN_BIN :
      fhdl = get_mtfile_handle(3);
      if (m_binary) {
        m_binary = false;
        fhdl->m_binary = false;
        m_qButtons[GPH_BTNID(GPH_BTN_BIN)]->setText(tr("DECIMAL"));
      }
      else {
        m_binary = true;
        fhdl->m_binary = true;
        m_qButtons[GPH_BTNID(GPH_BTN_BIN)]->setText(tr("BINARY"));
      }
    break;
  }
}

void gphdlg::close_dump()
{
  m_qButtons[GPH_BTNID(GPH_BTN_DUMP)]->setText(tr("DUMP ON"));
  // m_dump_fp->close();
}

void gphdlg::thread_resume()
{
  m_graph = true;
  // m_pgdate0->resume();
  // QThread::msleep(200);
  // m_pmovedata->resume();
  // QThread::msleep(200);
  // m_pdrawgraph->resume();
  // QThread::msleep(200);
  m_pupdatetimer->resume();
}

void gphdlg::thread_pause()
{
  m_graph = false;
}

QChartViewer* gphdlg::get_hdl_chart(bool newone)
{
  static QChartViewer* p_chartview;
  
  if (newone) {
    p_chartview = new QChartViewer(this);
  }
  
  return p_chartview;
}

void gphdlg::onRealTimeRead()
{
}

void gphdlg::onTakeData()
{
  uint8_t* lp_rcvbuf = uart_rcv_buf(0);
  int16_t reorder_dat = 0;
  uint8_t msbyte = 0;
  uint8_t lsbyte = 0;
  int16_t* wdata_ff = NULL;
  uint16_t* uwdata_ff = NULL;
  static uint32_t ttmp = 0;
  void *pt = NULL;

  lp_rcvbuf += 3;
  
  // if (!(ttmp++ % 512))
    // qDebug() << "take data" << ttmp;

  if (m_wfile) {
    wdata_ff = new int16_t[12 * 3];
    uwdata_ff = new uint16_t[12 * 3];
  }

  for (int k = 0; k < 12; k++) {
    m_tbuf.put(time_increment(false)/1000.0);
    for (int i = 0; i < 3; i++) {
      msbyte = lp_rcvbuf[k * 6 + (i * 2)];
      lsbyte = lp_rcvbuf[k * 6 + (i * 2 + 1)];
      reorder_dat =  (((uint16_t)msbyte)<<2) | (lsbyte & 0x3);
      reorder_dat |= ((msbyte >> 7) & 0x1) ? 0xfc00 : 0x0000;
      m_buf[i+1].put((double)reorder_dat);
      if (m_wfile) {
        wdata_ff[k * 3 + i] = reorder_dat;
        uwdata_ff[k * 3 + i] = (msbyte << 8) | lsbyte;
      }
    }
  }

  if (m_wfile) {
    UserData* file_udata = new UserData(0, get_file_id(false), ((!m_binary) ? (void *)wdata_ff : (void *)uwdata_ff));
    set_lock_file_var();
    push_file_queue(file_udata);
  }
}


