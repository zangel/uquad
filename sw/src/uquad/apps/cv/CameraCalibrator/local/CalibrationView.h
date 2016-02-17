#ifndef CVCC_LOCAL_CALIBRATION_VIEW_H
#define CVCC_LOCAL_CALIBRATION_VIEW_H

#include "Config.h"
#include "ui_CalibrationView.h"
#include <uquad/comm/RemoteControl.h>
#include <uquad/cv/CameraCalibration.h>


namespace cvcc
{
namespace local
{
    class CalibrationView
        : public QWidget
        , protected Ui::CalibrationView
        , protected comm::RemoteControlDelegate
    {
    private:
        
        Q_OBJECT
    protected:
    
    public:
        CalibrationView(QMainWindow *mw);
        ~CalibrationView();
        
    protected:
        void onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg);
        
        Q_INVOKABLE void updateCalibrationUI();
        
        
    protected slots:
        void onAddImageClicked();
        void onResetClicked();
        void onCalibrateClicked();
        void onSaveClicked();
        
        
        void computeChessboardCorners(::cv::Size bs, float ss, std::vector<::cv::Point3f> &points);
        
        
    private:
        uquad::cv::CameraCalibration m_Calibration;
        double m_CalibrationMError;
        std::vector< std::vector<::cv::Point2f> > m_ImagePoints;
    };
    
    
} //namespace local
} //namespace cvcc

#endif //CVCC_LOCAL_CALIBRATION_VIEW_H