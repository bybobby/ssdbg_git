///////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2016 Advanced Software Engineering Limited
//
// You may use and modify the code in this file in your application, provided the code and
// its modifications are used only in conjunction with ChartDirector. Usage of this software
// is subjected to the terms and condition of the ChartDirector license.
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "chart/mthread.h"
#include "chart/chartdir.h"
#include "serial/uart_conn.h"
#include "childdlg/dlghandle.h"
#include "childdlg/gphdlg.h"
#include "childdlg/ssdbg.h"
#include "childdlg/dbgdlg.h"
#include "ftdi/ftdi_usb2spi.h"
#include "ftdi/spi_access.h"
#include <math.h>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDebug>


// The period of the data series in milliseconds. This random series implementation just use the 
// windows timer for timing. In many computers, the default windows timer resolution is 1/64 sec,
// or 15.6ms. This means the interval may not be exactly accurate.
const int interval = 1;

//
// Constructor. The handler is the function to call then new data are generated. The "param" is a
// parameter for passing to the handler function.
//
gdata::gdata(gdata::dhandler *handler, void *param)
: stopThread(false), handler(handler), param(param)
, pauseThread(true), wait_loop(true)
{
}

//
// Destructor
//
gdata::~gdata()
{
	stop();
}

//
// Stop the random generator thread
//
void gdata::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void gdata::pause()
{
  pauseThread = true;
  get_pause(1);
}

void gdata::resume()
{
  pauseThread = false;
  get_pause(0);
}

bool gdata::get_pause(uint8_t set)
{
  static bool st_pause = true;

  if (set == 0) {
    st_pause = false;
  }
  else if (set == 1) {
    st_pause = true;
  }

  return st_pause;
}


//
// The random generator thread
//
void gdata::run()
{
  qint64 currentTime = 0;
  qint64 nextTime = 0;
  
	gphdlg* ppw = (gphdlg *)param;
  
    // Random walk variables
	srand(999);
	double series0 = 32;
	double series1 = 63;
	double series2 = 63;
	double series3 = 63;
	double series4 = 63;
	double series5 = 63;
  double upperLimit = 94;
  double scaleFactor = sqrt(interval * 0.1);

  short sdata[6] = { 0 };
  
  // Variables to keep track of the timing
#ifdef DUMMY_DATA
  QElapsedTimer timer;
  timer.start();
#endif

  while (!stopThread)
	{
    while (get_pause(3)) {
      msleep(100);
    }
      
#ifdef DUMMY_DATA
    // Compute the next data value
    currentTime += interval;
    double p = (Chart::chartTime2(nextDataTime.toTime_t())
                 + nextDataTime.time().msec() / 1000.0) * 4;
    double series0 = 20 + cos(p / 5.2) * 10 / (cos(p) * cos(p) + 0.1);
    double series1 = 150 + 100 * sin(p / 27.7) * sin(p / 10.1);
    double series2 = 150 + 100 * cos(p / 6.7) * cos(p / 11.9);
    double series3 = 60 + 100 * sin(p / 8.7) * sin(p / 16.9);
    double series4 = 70 + 100 * cos(p / 12.7) * cos(p / 1.9);
    double series5 = 10 + 80 * sin(p / 3.7) * sin(p / 9.9);

    // Call the handler
    handler(param, currentTime / 1000.0, series0, series1, series2, series3, series4, series5);
    
    nextDataTime = nextDataTime.addMSecs(interval);
    
    // Sleep until next walk
    if ((nextTime += interval) <= currentTime)
      nextTime = currentTime + interval;
    
    msleep((unsigned long)(nextTime - currentTime));
#else
    // wait_loop = true;
    // while (wait_loop)
    //   usleep(10);
    
    // // qDebug() << __func__ << ",   line : " << __LINE__;
    // set_lock_gphdlg(true);
    // // qDebug() << "[LOOP] m_rbuf pt = " << ppw->m_prcmd->m_rbuf;
    // if (ppw->m_prcmd->m_rbuf != NULL) {
    //   for (int i = 0; i < SPI_QUEUE_LEGNTH; i++) {
    //     BYTE rddat = 0;
    //     
    //     for (int k = 0; k < 8; k++) {
    //       rddat |= (((ppw->m_prcmd->m_rbuf[i * 8 + k] & 0x04) >> 2) << (7 - k));
    //     }
    //     
    //     m_rdbuf[((i&0xfffffffe) + ((i+1) % 2))] = rddat;
    //   }
    //   delete[] ppw->m_prcmd->m_rbuf;
    //   ppw->m_prcmd->m_rbuf = NULL;
    //   memcpy(ppw->m_log_cpy, m_rdbuf, sizeof(unsigned char)*SPI_QUEUE_LEGNTH);
    //   ppw->m_plogp->wait_data = false;
    //   set_lock_gphdlg(false);
    //   
    //   // qDebug() << "[0x53] = " << QString(tr("")).sprintf("0x%02x", m_rdbuf[0]);
    //   // qDebug() << "[0x54] = " << QString(tr("")).sprintf("0x%02x", m_rdbuf[1]);
    //   // qDebug() << "[0x55] = " << QString(tr("")).sprintf("0x%02x", m_rdbuf[2]);
    //   // qDebug() << "[0x56] = " << QString(tr("")).sprintf("0x%02x", m_rdbuf[3]);
    //   // short tmp[6] = {0};
    //   // memcpy(tmp, (short *)(m_rdbuf), sizeof(short)*2);
    //   // qDebug() << "trans 0 = " << tmp[0];
    //   // qDebug() << "trans 1 = " << tmp[1];
    //   
    //   int seq_m = 0;
    //   // qDebug() << __func__ << ",   line : " << __LINE__;
    //   set_lock_gphdlg(true);
    //   for (int m=0; m<SPI_QUEUE_LEGNTH; m+=NUM_OF_REAREG) {
    //     memcpy(sdata, (short *)(m_rdbuf+m), sizeof(short)*3);
    //     memcpy(sdata+3, (short *)(m_rdbuf+8+m), sizeof(short)*3);
    //     
    //     memcpy(&ppw->m_dump_cpy[(seq_m++)*6], sdata, sizeof(short)*6);
    //     handler(param, time_increment(false) / 1000.0, sdata[0], sdata[1], sdata[2], sdata[3], sdata[4], sdata[5]);
    //     
    //   }
    //   ppw->m_pdumpp->wait_data = false;
    //   // ppw->m_prcmd->wait_reorder = false;
    //   set_lock_gphdlg(false);
    // }
    // else {
    //   set_lock_gphdlg(false);
    // }
    handler(param);
    msleep(1);
#endif
	}
}


wcmd::wcmd(void *param)
: stopThread(false), param(param)
, pauseThread(true), wait_wr(true)
{
}

//
// Destructor
//
wcmd::~wcmd()
{
	stop();
}

//
// Stop the random generator thread
//
void wcmd::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void wcmd::pause()
{
  pauseThread = true;
}

void wcmd::resume()
{
  pauseThread = false;
}

//
// The random generator thread
//
void wcmd::run()
{
  qint64 currentTime = 0;
  qint64 nextTime = 0;
	gphdlg* ppw = (gphdlg *)param;
  bool resume_first = true;
  
  while (!stopThread)
	{
    while (pauseThread){
      msleep(100);
    }
    spi_queue_write();
    usleep(1);
  }
}


rcmd::rcmd(void *param)
: stopThread(false), param(param)
, pauseThread(true), wait_reorder(true)
{
}

//
// Destructor
//
rcmd::~rcmd()
{
	stop();
}

//
// Stop the random generator thread
//
void rcmd::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void rcmd::pause()
{
  pauseThread = true;
}

void rcmd::resume()
{
  pauseThread = false;
}

//
// The random generator thread
//
void rcmd::run()
{
  qint64 currentTime = 0;
  qint64 nextTime = 0;
	gphdlg* ppw = (gphdlg *)param;
  
  // QDateTime nextDataTime = QDateTime::currentDateTime();
  unsigned char toggle = 0;
  bool togvar = false;
  
  while (!stopThread)
	{
    while (pauseThread){
      msleep(100);
    }
    m_queue_buf = new unsigned char[SPI_QUEUE_BUFF_LEN];
    memset(m_queue_buf, 0, sizeof(unsigned char)*SPI_QUEUE_BUFF_LEN);
    
    toggle++;
    toggle &= 0x3;
    
    m_rbuf = spi_queue_read(m_queue_buf);
    // qDebug() << __func__ << ",   line : " << __LINE__;
    
    set_lock_gphdlg(true);
    if (!togvar) {
      ppw->m_pgdate0->wait_loop = false;
      togvar = true;
    }
    else {
      ppw->m_pgdate1->wait_loop = false;
      togvar = false;
    }
    set_lock_gphdlg(false);
  }
}


movedata::movedata(movedata::vhandler *handler, void *param)
: stopThread(false), handler(handler), param(param)
, m_wait(true), pauseThread(true)
{
}

//
// Destructor
//
movedata::~movedata()
{
	stop();
}

//
// Stop the random generator thread
//
void movedata::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void movedata::pause()
{
  pauseThread = true;
}

void movedata::resume()
{
  pauseThread = false;
}


//
// The random generator thread
//
void movedata::run()
{
  gphdlg* ppw = (gphdlg *)param;
  m_cidx = 0;
  uint32_t lv_sampleSize = 0;
  
  while(!stopThread)
  {

    //
    // Get new data from the queue and append them to the data arrays
    //
    int count;
    // DataPacket *packets;
    double *tbufptr;
    double *bufptr[7];
    
    m_wait = true;
    
    // qDebug() << __func__ << ",   line : " << __LINE__;
    set_lock_gphdlg(true);
    if ((count = ppw->m_tbuf.get(&tbufptr)) <= 0)
        goto next;
      
    for (int i=1; i<7; i++){
      if ((count = ppw->m_buf[i].get(&bufptr[i])) <= 0)
        goto next;
    }
    // qDebug() << "count : " << count;
  
    lv_sampleSize = ppw->sampleSize;
    
    // if data arrays have insufficient space, we need to remove some old data.
    if (m_cidx + count >= lv_sampleSize)
    {
      // For safety, we check if the queue contains too much data than the entire data arrays. If
      // this is the case, we only use the latest data to completely fill the data arrays.
      if (count > lv_sampleSize)
      {
        tbufptr += count - lv_sampleSize;
        for (int i=1; i<7; i++)
          bufptr[i] += count - lv_sampleSize;
        count = lv_sampleSize;
      }
  
      // Remove oldest data to leave space for new data. To avoid frequent removal, we ensure at
      // least 5% empty space available after removal.
      int originalIndex = m_cidx;
      m_cidx = lv_sampleSize * 95 / 100 - 1;
      if (m_cidx > lv_sampleSize - count)
        m_cidx = lv_sampleSize - count;
  
      int srcIndex = originalIndex - m_cidx;
      // memcpy(ppw->m_tpsrc, ppw->m_tpsrc+srcIndex, sizeof(double)*m_cidx);
      // for (int i=1; i<7; i++) {
      //   memcpy(ppw->m_psrc[i], ppw->m_psrc[i]+srcIndex, sizeof(double)*m_cidx);
      // }
      
      // qDebug() << "cidx : " << m_cidx << "  srcidx : " << srcIndex;
    }
  
    // Append the data from the queue to the data arrays
    // memcpy(ppw->m_tpsrc+m_cidx, tbufptr, sizeof(double)*count);
    // 
    // for (int i=1; i<7; i++) {
    //   memcpy(ppw->m_psrc[i]+m_cidx, bufptr[i], sizeof(double)*count);
    // }
    
    //ppw->m_prevIndex = m_cidx;
    m_cidx += count;
    // ppw->m_firstdata = ppw->m_tpsrc[0];
    // ppw->m_lastdata = ppw->m_tpsrc[m_cidx-1];
    
    next :
    set_lock_gphdlg(false);
    while (m_wait) {
      if (stopThread)
        break;
      msleep(2);
    }
    while (pauseThread)
      msleep(100);
  }
}


drawgraph::drawgraph(drawgraph::ghandler *handler, void *param)
: stopThread(false), handler(handler), param(param)
, m_wait(true), pauseThread(true), initialdraw(true)
{
  sprintf_s(m_graph_mark0, "pt a <*bgColor=FFCCCC*>");
  sprintf_s(m_graph_mark1, "pt b <*bgColor=CCFFCC*>");
  sprintf_s(m_graph_mark2, "pt c <*bgColor=CCCCFF*>");
  sprintf_s(m_graph_mark3, "pt d <*bgColor=9999CC*>");
  sprintf_s(m_graph_mark4, "pt e <*bgColor=CC9999*>");
  sprintf_s(m_graph_mark5, "pt f <*bgColor=99CC99*>");
  
  m_graph_width = ((gphdlg *)param)->m_graph_width;
  m_graph_height = ((gphdlg *)param)->m_graph_height;
  
  m_xyChart0 = NULL;
  m_xyChart1 = NULL;
}

//
// Destructor
//
drawgraph::~drawgraph()
{
	stop();
}

//
// Stop the random generator thread
//
void drawgraph::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}


void drawgraph::pause()
{
  pauseThread = true;
}

void drawgraph::resume()
{
  pauseThread = false;
}


//
// The random generator thread
//
void drawgraph::run()
{
  // gphdlg* ppw = (gphdlg *)param;
  // QChartViewer *viewer = ppw->m_ChartViewer;
  // uint8_t lv_draw_chk = 0;
  // 
  // while (!stopThread)
  // {
  //   XYChart* xyChart;
  // 
  //   // qDebug() << __func__ << ",   line : " << __LINE__;
  //   set_lock_gphdlg(true);
  //   
  //   if (m_xyChart0 == NULL) {
  //     // Create an XYChart object extending to the container boundary, with a minimum of 300 x 180 
  //     m_xyChart0 = new XYChart((std::max)(100, m_graph_width),
  //       (std::max)(190, m_graph_height), Chart::Transparent, Chart::Transparent, 0);
  //     xyChart = m_xyChart0;
  //   }
  //   else if (m_xyChart1 == NULL) {
  //     m_xyChart1 = new XYChart((std::max)(100, m_graph_width),
  //       (std::max)(190, m_graph_height), Chart::Transparent, Chart::Transparent, 0);
  //     xyChart = m_xyChart1;
  //   }
  //   else {
  //     qDebug() << "Error : generation chart!!";
  //   }
  // 
  //   m_wait = true;
  // 
  //   // // qDebug() << __func__ << ",   line : " << __LINE__;
  //   
  //   // Get the start date and end date that are visible on the chart.
  //   // double viewPortStartDate = viewer->getValueAtViewPort("x", viewer->getViewPortLeft());
  //   // double viewPortEndDate = viewer->getValueAtViewPort("x", viewer->getViewPortLeft() +
  //   //  viewer->getViewPortWidth());
  // 
  //   // Extract the part of the data arrays that are visible.
  //   DoubleArray viewPortTimeStamps;
  //   DoubleArray viewPortDataSeriesA;
  //   DoubleArray viewPortDataSeriesB;
  //   DoubleArray viewPortDataSeriesC;
  //   DoubleArray viewPortDataSeriesD;
  //   DoubleArray viewPortDataSeriesE;
  //   DoubleArray viewPortDataSeriesF;
  // 
  //   // if (ppw->m_prevIndex > 0) {
  //   //   // Get the array indexes that corresponds to the visible start and end dates
  //   //   int startIndex = (int)floor(Chart::bSearch(DoubleArray(ppw->m_tpsink, ppw->m_prevIndex), viewPortStartDate));
  //   //   int endIndex = (int)ceil(Chart::bSearch(DoubleArray(ppw->m_tpsink, ppw->m_prevIndex), viewPortEndDate));
  //   //   int noOfPoints = endIndex - startIndex + 1;
  //   // 
  //   //   // Extract the visible data
  //   //   viewPortTimeStamps = DoubleArray(ppw->m_tpsink + startIndex, noOfPoints);
  //   //   viewPortDataSeriesA = DoubleArray(ppw->m_psink[1] + startIndex, noOfPoints);
  //   //   viewPortDataSeriesB = DoubleArray(ppw->m_psink[2] + startIndex, noOfPoints);
  //   //   viewPortDataSeriesC = DoubleArray(ppw->m_psink[3] + startIndex, noOfPoints);
  //   //   viewPortDataSeriesD = DoubleArray(ppw->m_psink[4] + startIndex, noOfPoints);
  //   //   viewPortDataSeriesE = DoubleArray(ppw->m_psink[5] + startIndex, noOfPoints);
  //   //   viewPortDataSeriesF = DoubleArray(ppw->m_psink[6] + startIndex, noOfPoints);
  //   // 
  //   // }
  //   // set_lock_gphdlg(false);
  // 
  //   //================================================================================
  //   // Configure overall chart appearance.
  //   //================================================================================
  // 
  // 
  //   //xyChart->setRoundedFrame(m_extBgColor);
  // 
  //   // Set the plotarea at (55, 62) and of size 520 x 175 pixels. Use white (ffffff) 
  //   // background. Enable both horizontal and vertical grids by setting their colors to 
  //   // grey (cccccc). Set clipping mode to clip the data lines to the plot area.
  //   xyChart->setPlotArea(65, 50, xyChart->getWidth() - 85, xyChart->getHeight() - 85, 0x1c1c1c, -1, 0, 0xaaaaaa, 0xaaaaaa);
  //   xyChart->setClipping();
  // 
  //   // Add a title to the chart using 15 pts Times New Roman Bold Italic font, with a light
  //   // grey (dddddd) background, black (000000) border, and a glass like raised effect.
  //   // TextBox* title = xyChart->addTitle("readout data", ":/res/fonts/Everson Mono Bold.ttf", 12);
  //   // title->setBackground(Chart::Transparent, Chart::Transparent, Chart::flatBorder(1));
  //   // //title->setMargin(5, 5, 10, 1);
  //   // title->setFontColor(0xe0e0e0);
  //       // )->setBackground(0xdddddd, 0x000000, Chart::glassEffect());
  // 
  //   // Add a legend box at the top of the plot area with 9pts Arial Bold font. We set the 
  //   // legend box to the same width as the plot area and use grid layout (as opposed to 
  //   // flow or top/down layout). This distributes the 3 legend icons evenly on top of the 
  //   // plot area.
  //   LegendBox *b = xyChart->addLegend2(75, 5, 6, ":/res/fonts/Everson Mono Bold.ttf", 11);
  //   b->setBackground(Chart::Transparent, Chart::Transparent);
  //   b->setWidth(440);
  //   b->setFontColor(0xe0e0e0);
  // 
  //   // Configure the y-axis with a 10pts Arial Bold axis title
  //   TextBox* yLabel = xyChart->yAxis()->setTitle("ADC level", ":/res/fonts/Everson Mono Bold.ttf", 12);
  //   yLabel->setFontColor(0xe0e0e0);
  //   yLabel->setMargin(2, 2, 2, 2);
  //   xyChart->yAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);
  // 
  //   // Configure the x-axis to auto-scale with at least 75 pixels between major tick and 
  //   // 15  pixels between minor ticks. This shows more minor grid lines on the chart.
  //   xyChart->xAxis()->setTickDensity(75, 15);
  //   xyChart->xAxis()->setLabelStyle(":/res/fonts/Everson Mono Bold.ttf", 11, 0xe0e0e0, 0);
  // 
  //   // Set the y-axis tick length to 0 to disable the tick and put the labels closer to the axis.
  //   xyChart->yAxis()->setTickLength(0);
  // 
  //   // Set the axes width to 2 pixels
  //   xyChart->xAxis()->setWidth(2);
  //   xyChart->yAxis()->setWidth(2);
  // 
  //   // Create a line layer to plot the lines
  //   LineLayer *layer = xyChart->addLineLayer();
  //   layer->setLineWidth(2);
  // 
  //   // Set the x-axis label format
  //   xyChart->xAxis()->setLabelFormat("{value}");
  // 
  //   // qDebug() << __func__ << ",   line : " << __LINE__;
  //   set_lock_gphdlg(true);
  //   // Set the x-axis as a date/time axis with the scale according to the view port x range.
  //   viewer->syncDateAxisWithViewPort("x", xyChart->xAxis());
  //   set_lock_gphdlg(false);
  //   
  //   // Now we add the data to the chart. 
  // 
  //   // In this demo, we do not have too many data points. In real code, the chart may contain a lot
  //   // of data points when fully zoomed out - much more than the number of horizontal pixels in this
  //   // plot area. So it is a good idea to use fast line mode.
  //   layer->setFastLineMode(true);
  // 
  //   // Now we add the 2 data series to the line layer with red (ff0000) and green (00cc00) colors
  //   layer->setXData(viewPortTimeStamps);
  //   // if (lv_draw_chk != 0 && ppw->m_prevIndex > 0){
  //       // qDebug() << viewPortDataSeriesA[0]
  //       // << ",    " << viewPortDataSeriesA[1]
  //       // << ",    " << viewPortDataSeriesA[2]
  //       // << ",    " << viewPortDataSeriesA[3]
  //       // << ",    " << viewPortDataSeriesA[4]
  //       // << ",    " << viewPortDataSeriesA[5];
  //   // }
  // 
  //   // qDebug() << __func__ << ",   line : " << __LINE__;
  //   set_lock_gphdlg(true);
  //   lv_draw_chk = ppw->m_draw_chk;
  //   set_lock_gphdlg(false);
  //   
  //   if ((lv_draw_chk >> 0)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesA, 0xff0000, m_graph_mark0);
  //   if ((lv_draw_chk >> 1)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesB, 0x00ff00, m_graph_mark1);
  //   if ((lv_draw_chk >> 2)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesC, 0x4444ff, m_graph_mark2);
  //   if ((lv_draw_chk >> 3)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesD, 0x999900, m_graph_mark3);
  //   if ((lv_draw_chk >> 4)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesE, 0x009999, m_graph_mark4);
  //   if ((lv_draw_chk >> 5)& 0x01)
  //     layer->addDataSet(viewPortDataSeriesF, 0x990099, m_graph_mark5);
  //   
  //   // In this demo, we rely on ChartDirector to auto-label the axis. We ask ChartDirector to ensure
  //   // the x-axis labels are at least 75 pixels apart to avoid too many labels.
  //   xyChart->xAxis()->setTickDensity(55);
  //   xyChart->yAxis()->setTickDensity(35);
  //   
  //   // We use "hh:nn:ss" as the axis label format.
  //   xyChart->xAxis()->setLabelFormat("{value|nn:ss}");
  // 
  //   // We make sure the tick increment must be at least 1 second.
  //   xyChart->xAxis()->setMinTickInc(2);
  // 
  //   // Set the auto-scale margin to 0.05, and the zero affinity to 0.6
  //   // xyChart->yAxis()->setAutoScale(0.05, 0.05, 0.6);
  //   xyChart->yAxis()->setLinearScale(-35000, 35000, 14000, 7000);
  //   
  //   ppw->m_xyChart = xyChart;
  //   
  //   //viewer->setChart(xyChart);
  //   
  //   while (pauseThread)
  //     msleep(100);
  //   
  //   next :
  //   while (m_wait) {
  //     if (stopThread)
  //       break;
  //     msleep(2);
  //   }
  // }
}


//
// Constructor. The handler is the function to call then new data are generated. The "param" is a
// parameter for passing to the handler function.
//
updatetimer::updatetimer(updatetimer::uhandler *handler, void *param)
: stopThread(false), handler(handler), param(param)
, pauseThread(true)
{
}

//
// Destructor
//
updatetimer::~updatetimer()
{
	stop();
}

//
// Stop the random generator thread
//
void updatetimer::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}


void updatetimer::pause()
{
  //pauseThread = true;
  get_pause_var(1);
}

void updatetimer::resume()
{
  // pauseThread = false;
  get_pause_var(0);
}

bool updatetimer::get_pause_var(uint8_t set)
{
  static bool st_pause = true;

  if (set == 0) {
    st_pause = false;
  }
  else if (set == 1) {
    st_pause = true;
  }

  return st_pause;
}

//
// The random generator thread
//
void updatetimer::run()
{
  gphdlg* ppw = (gphdlg *)param;
  
  while (!stopThread)
	{
    while (get_pause_var(3))
      delay(50);
    emit upd_trigger();
    delay(20);
	}
}

dumpp::dumpp(void *param)
: stopThread(false), param(param)
, pauseThread(true), m_dump_out(NULL)
, wait_data(true)
{
}

//
// Destructor
//
dumpp::~dumpp()
{
	stop();
}

//
// Stop the random generator thread
//
void dumpp::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void dumpp::pause()
{
  pauseThread = true;
}

void dumpp::resume()
{
  pauseThread = false;
}

void dumpp::set_textstream(QTextStream* ts_dump)
{
  if (ts_dump != NULL)
    m_dump_out = ts_dump;
}

//
// The random generator thread
//
void dumpp::run()
{
	gphdlg* ppw = (gphdlg *)param;
  short lv_data[6*DUMP_NUM];
  uint32_t cnt = 0;
  QString str = tr("");
  
  while (!stopThread)
	{
    while (pauseThread){
      cnt = 0;
      wait_data = true;
      msleep(100);
    }
    
    while(wait_data)
      usleep(100);
    
    wait_data = true;
    
    // qDebug() << __func__ << ",   line : " << __LINE__;
    set_lock_gphdlg(true);
    memcpy(lv_data, ppw->m_dump_cpy, sizeof(short)*6*DUMP_NUM);
    set_lock_gphdlg(false);
    
    if (m_dump_out != NULL){
      str.clear();
      for(int i=0; i<DUMP_NUM; i++) {
        str.append(QString(tr("")).sprintf("0x%08x,%d,%d,%d,%d,%d,%d\n", cnt++, 
                    lv_data[0+i*6], lv_data[1+i*6], lv_data[2+i*6],
                    lv_data[3+i*6], lv_data[4+i*6], lv_data[5+i*6]));
      }
      *m_dump_out << str;
      
      //if (cnt > 3000000) {
      if (cnt > 300000) {
        // qDebug() << __func__ << ",   line : " << __LINE__;
        set_lock_gphdlg(true);
        delete m_dump_out;
        m_dump_out = NULL;
        ppw->close_dump();
        set_lock_gphdlg(false);
        pauseThread = true;
      }
    }
  }
}

logp::logp(void *param)
: stopThread(false), param(param)
, pauseThread(true), wait_data(true)
{
}

//
// Destructor
//
logp::~logp()
{
	stop();
}

//
// Stop the random generator thread
//
void logp::stop()
{
  stopThread = true;
  wait();
  stopThread = false;
}

void logp::pause()
{
  pauseThread = true;
}

void logp::resume()
{
  pauseThread = false;
}

//
// The random generator thread
//
void logp::run()
{
	gphdlg* ppw = (gphdlg *)param;
  unsigned char lv_data[SPI_QUEUE_LEGNTH];
  uint32_t cnt = 0;
  QString str = tr("");
  
  while (!stopThread)
	{
    while (pauseThread){
      cnt = 0;
      wait_data = true;
      msleep(100);
    }
    while(wait_data)
      usleep(100);
    
    wait_data = true;
    
    // qDebug() << __func__ << ",   line : " << __LINE__;
    set_lock_gphdlg(true);
    memcpy(lv_data, ppw->m_log_cpy, sizeof(unsigned char)*SPI_QUEUE_LEGNTH);
    set_lock_gphdlg(false);
    
    str.clear();
    for(int i=0; i<SPI_QUEUE_LEGNTH; i++) {
      str.append(QString(tr("")).sprintf("read  -> address : 0x53,   data = 0x%02x\n", lv_data[i*14+0]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x54,   data = 0x%02x\n", lv_data[i*14+1]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x55,   data = 0x%02x\n", lv_data[i*14+2]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x56,   data = 0x%02x\n", lv_data[i*14+3]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x57,   data = 0x%02x\n", lv_data[i*14+4]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x58,   data = 0x%02x\n", lv_data[i*14+5]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x59,   data = 0x%02x\n", lv_data[i*14+6]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5a,   data = 0x%02x\n", lv_data[i*14+7]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5b,   data = 0x%02x\n", lv_data[i*14+8]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5c,   data = 0x%02x\n", lv_data[i*14+9]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5d,   data = 0x%02x\n", lv_data[i*14+10]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5e,   data = 0x%02x\n", lv_data[i*14+11]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x5f,   data = 0x%02x\n", lv_data[i*14+12]));
      str.append(QString(tr("")).sprintf("read  -> address : 0x60,   data = 0x%02x\n", lv_data[i*14+13]));
    }
    set_lock_dbgdlg(true);
    ((ssdbg*)ppw->parentWidget())->m_pdbgdlg->appendline(str);
    set_lock_dbgdlg(false);
  }
}

