///////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright 2016 Advanced Software Engineering Limited
//
// You may use and modify the code in this file in your application, provided the code and
// its modifications are used only in conjunction with ChartDirector. Usage of this software
// is subjected to the terms and condition of the ChartDirector license.
///////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __GENDATA_H__
#define __GENDATA_H__

#include <QThread>
#include <qglobal.h>
#include "common.h"

QT_FORWARD_DECLARE_CLASS(XYChart);
QT_FORWARD_DECLARE_CLASS(QTextStream);

//
// A Random series generator using the random walk method
//
class gdata : public QThread
{
  Q_OBJECT
  
public:

  // typedef void dhandler(void *param, double elapsedTime, int s0, int s1, int s2, int s3, int s4, int s5);
  typedef void dhandler(void *param);

	gdata(dhandler *handler, void *param);
  virtual ~gdata();

  // stop the thread
  void stop();
  void pause();
  void resume();
  void setbuf(unsigned char* bpt);
  bool wait_loop;
  unsigned char* prevbuf;
  unsigned char m_rdbuf[16384];
  unsigned char m_tbuf[16384];
  bool get_pause(uint8_t set);

protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  gdata & operator=(const gdata&);
  gdata(const gdata&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
  
  
  // The callback function to handle the generated data
	dhandler *handler;
	void *param;
  
};


//
// A Random series generator using the random walk method
//
class wcmd : public QThread
{
  Q_OBJECT
  
public:

	wcmd(void *param);
  virtual ~wcmd();

  // stop the thread
  void stop();
  void pause();
  void resume();
  bool wait_wr;

protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  wcmd & operator=(const wcmd&);
  wcmd(const wcmd&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
  
	void *param;
};


class rcmd : public QThread
{
  Q_OBJECT
  
public:

	rcmd(void *param);
  virtual ~rcmd();

  // stop the thread
  void stop();
  void pause();
  void resume();
  bool wait_reorder;
  unsigned char* m_rbuf;
  unsigned char* m_queue_buf;
  
protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  rcmd & operator=(const rcmd&);
  rcmd(const rcmd&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
	void *param;
};


class movedata : public QThread
{
  Q_OBJECT
  
public:

  typedef void vhandler(void *param);

	movedata(vhandler *handler, void *param);
  virtual ~movedata();

  // stop the thread
  void stop();
  void pause();
  void resume();
  
  bool m_wait;
  int m_cidx;

protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  movedata & operator=(const movedata&);
  movedata(const movedata&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
  
    // The callback function to handle the generated data
	vhandler *handler;
	void *param;
};

class drawgraph : public QThread
{
  Q_OBJECT
  
public:

  typedef void ghandler(void *param);

	drawgraph(ghandler *handler, void *param);
  virtual ~drawgraph();

  // stop the thread
  void stop();
  void pause();
  void resume();
  
  bool m_wait;
  int m_cidx;
  
  XYChart* m_xyChart0;
  XYChart* m_xyChart1;

protected :
  // The thread of the random series generator
  void run();

private :

  char m_graph_mark0[128];
  char m_graph_mark1[128];
  char m_graph_mark2[128];
  char m_graph_mark3[128];
  char m_graph_mark4[128];
  char m_graph_mark5[128];
  
  int m_graph_width;
  int m_graph_height;
  
  // Disable copying and assignment
  drawgraph & operator=(const drawgraph&);
  drawgraph(const drawgraph&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
  bool initialdraw;
  
  // The callback function to handle the generated data
	ghandler *handler;
	void *param;
};


class updatetimer : public QThread
{
  Q_OBJECT
  
public:

  typedef void uhandler(void *param);

	updatetimer(uhandler *handler, void *param);
  virtual ~updatetimer();

  // stop the thread
  void stop();
  void pause();
  void resume();
  bool get_pause_var(uint8_t set);

signals:
  void upd_trigger();

protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  updatetimer & operator=(const updatetimer&);
  updatetimer(const updatetimer&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;

    // The callback function to handle the generated data
	uhandler *handler;
	void *param;
};



class dumpp : public QThread
{
  Q_OBJECT
  
public:

	dumpp(void *param);
  virtual ~dumpp();

  // stop the thread
  void stop();
  void pause();
  void resume();
  void set_textstream(QTextStream* ts_dump);
  bool wait_data;
  
protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  dumpp & operator=(const dumpp&);
  dumpp(const dumpp&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
	void *param;
  QTextStream* m_dump_out;
};


class logp : public QThread
{
  Q_OBJECT
  
public:

	logp(void *param);
  virtual ~logp();

  // stop the thread
  void stop();
  void pause();
  void resume();
  bool wait_data;
  
protected :
  // The thread of the random series generator
  void run();

private :

  // Disable copying and assignment
  logp & operator=(const logp&);
  logp(const logp&);

  // Flag to stop the flag
  bool stopThread;
  bool pauseThread;
	void *param;
};

#endif

