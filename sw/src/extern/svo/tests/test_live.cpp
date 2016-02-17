// This file is part of SVO - Semi-direct Visual Odometry.
//
// Copyright (C) 2014 Christian Forster <forster at ifi dot uzh dot ch>
// (Robotics and Perception Group, University of Zurich, Switzerland).
//
// SVO is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or any later version.
//
// SVO is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
// FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>
#include <svo/config.h>
#include <svo/frame_handler_mono.h>
#include <svo/map.h>
#include <svo/frame.h>
#include <svo/feature.h>
#include <vector>
#include <string>
#include <vikit/math_utils.h>
#include <vikit/vision.h>
#include <vikit/abstract_camera.h>
#include <vikit/atan_camera.h>
#include <vikit/pinhole_camera.h>
#include <opencv2/opencv.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <sophus/se3.hpp>
#include <iostream>

#define CV_LIVE_WINDOW_NAME "Live"

namespace svo
{
    class LiveNode
    {
        vk::AbstractCamera* cam_;
        svo::FrameHandlerMono* vo_;
        
    public:
        LiveNode();
        ~LiveNode();
        void run();
    };
    
    LiveNode::LiveNode()
    {
        cam_ = new vk::PinholeCamera(
            640, 480, 665.05, 665.05, 319.5, 239.5,
            -8.42e-2, 7.04e-1, 1.749e-3, -1.0659e-2, -2.267
        );
        vo_ = new svo::FrameHandlerMono(cam_);
        vo_->start();
    }
    
    LiveNode::~LiveNode()
    {
        delete vo_;
        delete cam_;
    }
    
    void LiveNode::run()
    {
        cv::VideoCapture capture;
        capture.open(0);
        capture.set(CV_CAP_PROP_FRAME_WIDTH, 640);
        capture.set(CV_CAP_PROP_FRAME_HEIGHT, 480);
        capture.set(CV_CAP_PROP_FPS, 30);
        
        
        cv::namedWindow(CV_LIVE_WINDOW_NAME, 1);
        
        for(int i = 0;;i++)
        {
            cv::Mat img, gray;
            capture >> img;
            
            cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
            
            if(i < 30)
                continue;
            
            vo_->addImage(gray, i*0.03);
            
            if(vo_->lastFrame() != NULL)
            {
                std::for_each(vo_->lastFrame()->fts_.begin(),vo_->lastFrame()->fts_.end(), [&img](Feature *f)
                {
                    cv::circle(img, cv::Point(f->px[0], f->px[1]), 2, cv::Scalar(0, 255, 0), -1, 8);
                });
                
                std::cout /*<< "Frame-Id: " << vo_->lastFrame()->id_ << " \t"*/
                    << "#Features: " << vo_->lastNumObservations() << " \t"
//                    << "Proc. Time: " << vo_->lastProcessingTime()*1000 << "ms \n"
                    << "Quality: " << vo_->trackingQuality() << "\n";

                // access the pose of the camera via vo_->lastFrame()->T_f_w_.
            }
            
            cv::imshow(CV_LIVE_WINDOW_NAME, img);
            
            int c = 0xff & cv::waitKey(capture.isOpened() ? 10 : 500);
            if((c & 255) == 27 || c == 'q' || c == 'Q' )
                break;
        }
    }
    
} // namespace svo

BOOST_AUTO_TEST_CASE(Live)
{
    {
        svo::LiveNode live;
        live.run();
    }
    printf("LiveNode finished.\n");
}

