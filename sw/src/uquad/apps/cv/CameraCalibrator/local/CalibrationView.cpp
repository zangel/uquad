#include "CalibrationView.h"
#include "Application.h"
#include <fstream>

namespace cvcc
{
namespace local
{

    CalibrationView::CalibrationView(QMainWindow *mw)
        : QWidget(mw)
        , Ui::CalibrationView()
    {
        setupUi(this);
        connect(
            m_CmdAddImage, SIGNAL(clicked()),
            this, SLOT(onAddImageClicked())
        );
        
        connect(
            m_CmdReset, SIGNAL(clicked()),
            this, SLOT(onResetClicked())
        );
        
        connect(
            m_CmdCalibrate, SIGNAL(clicked()),
            this, SLOT(onCalibrateClicked())
        );
        
        connect(
            m_CmdSave, SIGNAL(clicked()),
            this, SLOT(onSaveClicked())
        );
        
        Application::instance()->getRemoteControl().remoteControlDelegate() += this;
        
    }
    
    CalibrationView::~CalibrationView()
    {
        Application::instance()->getRemoteControl().remoteControlDelegate() -= this;
    }
    
    void CalibrationView::onRemoteControlMessageReceived(intrusive_ptr<comm::Message> msg)
    {
    }
    
    void CalibrationView::updateCalibrationUI()
    {
        m_CalibrationFx->setText(QString::number(m_Calibration.fx));
        m_CalibrationFy->setText(QString::number(m_Calibration.fy));
        m_CalibrationCx->setText(QString::number(m_Calibration.cx));
        m_CalibrationCy->setText(QString::number(m_Calibration.cy));
        m_CalibrationS->setText(QString::number(m_Calibration.s));
        m_CalibrationMeanError->setText(QString::number(m_CalibrationMError));
    }
    
    void CalibrationView::onAddImageClicked()
    {
        std::vector<::cv::Point2f> ip;
        if(m_CameraView->getChessboardCorners(ip))
        {
            m_ImagePoints.push_back(ip);
        }
    }
    
    void CalibrationView::onResetClicked()
    {
        m_ImagePoints.clear();
    }
    
    void CalibrationView::onCalibrateClicked()
    {
        std::vector< std::vector<::cv::Point3f > > ops(1);
        computeChessboardCorners(m_CameraView->getChessboardSize(), 1.0f, ops[0]);
        ops.resize(m_ImagePoints.size(), ops[0]);
        
        ::cv::Mat cameraMatrix = ::cv::Mat::eye(3, 3, CV_64F);
        ::cv::Mat distCoeffs = ::cv::Mat::zeros(8, 1, CV_64F);
        
        std::vector<::cv::Mat> rvecs, tvecs;
        
        ::cv::Size videoSize = m_CameraView->getVideoSize();
        double rms = ::cv::calibrateCamera(
            ops,
            m_ImagePoints,
            videoSize,
            cameraMatrix,
            distCoeffs,
            rvecs,
            tvecs,
            ::cv::CALIB_FIX_PRINCIPAL_POINT|::cv::CALIB_FIX_K4|::cv::CALIB_FIX_K5
        );
        
        m_Calibration.fx = cameraMatrix.at<double>(0,0)/videoSize.width;
        m_Calibration.fy = cameraMatrix.at<double>(1,1)/videoSize.height;
        
        m_Calibration.cx = cameraMatrix.at<double>(0,2)/videoSize.width;
        m_Calibration.cy = cameraMatrix.at<double>(1,2)/videoSize.height;
        m_Calibration.s = distCoeffs.at<double>(0,0);
        
        m_CalibrationMError = rms;
        
        std::cout << "cameraMatrix=" << cameraMatrix << std::endl;
        std::cout << "dist=" << distCoeffs << std::endl;
        updateCalibrationUI();
    }
    
    void CalibrationView::onSaveClicked()
    {
        QString calibFileName = QFileDialog::getSaveFileName(this, tr("Save calibration"), QString(), tr("All Files (*)"));
        if(calibFileName.isEmpty())
            return;
        
        std::ofstream ccFile(calibFileName.toStdString());
        if(!ccFile.is_open())
            return;
        
        {
            base::OArchive ccArchive(ccFile);
            ccArchive & m_Calibration;
        }
    }
    
    void CalibrationView::computeChessboardCorners(::cv::Size bs, float ss, std::vector<::cv::Point3f> &pts)
    {
        pts.clear();
        for( int i = 0; i < bs.height; ++i )
            for( int j = 0; j < bs.width; ++j )
                pts.push_back(::cv::Point3f(j*ss, i*ss, 0));
    }
    

} //namespace local
} //namespace cvcc