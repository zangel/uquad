#ifndef UQUAD_TOOL_CONFIG_H
#define UQUAD_TOOL_CONFIG_H

#include <uquad/Config.h>
#include <uquad/hal/UQuadDevice.h>
#include <boost/thread.hpp>
#include <boost/chrono.hpp>

#include <QObject>
#include <QApplication>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QSplitter>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QAbstractItemModel>
#include <QTreeView>
#include <QListWidget>
#include <QAbstractButton>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QPushButton>

namespace uquad
{
namespace tool
{

    typedef accumulators::accumulator_set
    <
        float,
		uquad::accumulators::stats
	    <
	    	uquad::accumulators::tag::min,
	    	uquad::accumulators::tag::max,
	    	uquad::accumulators::tag::mean,
	    	uquad::accumulators::tag::variance
	    >
    > stats_t;
    
} //namespace tool
} //namespace uquad


#endif //UQUAD_TOOL_CONFIG_H