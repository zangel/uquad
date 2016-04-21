#ifndef CTRLPE_LOCAL_CONFIG_H
#define CTRLPE_LOCAL_CONFIG_H

#include <uquad/Config.h>
#include <uquad/math/geom/Ellipsoid.h>
#include <uquad/math/geom/ArcBall.h>
#include <uquad/base/Runloop.h>
#include <uquad/hal/UQuad.h>
#include <uquad/comm/RemoteControlClient.h>
#include <uquad/comm/Message.h>
#include <uquad/comm/RemoteControl.h>

#include <uquad/control/AccelerometerCalibration.h>
#include <uquad/control/GyroscopeCalibration.h>
#include <uquad/control/MagnetometerCalibration.h>
#include <uquad/control/System.h>
#include <uquad/control/UQuadSensors.h>
#include <uquad/control/PoseEstimation.h>
#include <uquad/control/AttitudeControlSimple.h>
#include <uquad/control/PositionControlSimple.h>
#include <uquad/control/QuadMotorsThrustSimple.h>

#include <QObject>
#include <QApplication>
#include <QWindow>
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
#include <QMouseEvent>
#include <QOpenGLWidget>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QFileDialog>

namespace ctrlpe
{
namespace local
{
    using namespace uquad;
    
} //namespace local
} //namespace ctrlpe


#endif //CTRLPE_LOCAL_CONFIG_H
