#include "Camera.h"
#include <sys/stat.h>
#include <linux/videodev2.h>

namespace uquad
{
namespace hal
{
	namespace
	{
		static int xioctl(int fd, int request, void *arg)
		{
			int r;
			do
			{
				r = ioctl(fd, request, arg);
			}
			while(-1 == r && EINTR == errno);
			return r;
		}

		static bool getCameraInfo(int fd)
		{
			std::cout << "formats:" << std::endl;
			int ipfmt = 0;
			while(true)
			{
				v4l2_fmtdesc pfmt;
				pfmt.index = ipfmt;
				pfmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
				if(-1 == xioctl(fd, VIDIOC_ENUM_FMT, &pfmt))
				{
					break;
				}
				std::cout << "fmt[" << ipfmt++ << "]:" << pfmt.description << std::endl;

				std::cout << "\tframesizes:" << std::endl;
				int ifrmsz = 0;
				while(true)
				{
					v4l2_frmsizeenum frmsz;
					frmsz.index = ifrmsz;
					frmsz.pixel_format = pfmt.pixelformat;
					if(-1 == xioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsz))
					{
						break;
					}
					std::cout << "\t" << ifrmsz++;

					switch(frmsz.type)
					{
					case V4L2_FRMSIZE_TYPE_DISCRETE:
						std::cout << "-D:" << frmsz.discrete.width << "x" << frmsz.discrete.height << std::endl;
						break;

					case V4L2_FRMSIZE_TYPE_CONTINUOUS:
						std::cout << "-C:" << std::endl;
						break;

					case V4L2_FRMSIZE_TYPE_STEPWISE:
						std::cout << "-S:" << std::endl;
						break;
					}

					if(frmsz.type != V4L2_FRMSIZE_TYPE_DISCRETE)
						continue;

					std::cout << "\t\tframerates:" << std::endl;
					int ifrmrt = 0;
					while(true)
					{
						v4l2_frmivalenum frmrt;
						frmrt.index = ifrmrt;
						frmrt.pixel_format = pfmt.pixelformat;
						frmrt.width = frmsz.discrete.width;
						frmrt.height = frmsz.discrete.height;
						if(-1 == xioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmrt))
						{
							break;
						}
						std::cout << "\t\t" << ifrmrt++;
						switch(frmrt.type)
						{
						case V4L2_FRMIVAL_TYPE_DISCRETE:
							std::cout << "-D:" << frmrt.discrete.numerator << "/" << frmrt.discrete.denominator << std::endl;
							break;

						case V4L2_FRMIVAL_TYPE_CONTINUOUS:
							std::cout << "-C:" << std::endl;
							break;

						case V4L2_FRMIVAL_TYPE_STEPWISE:
							std::cout << "-S:" << std::endl;
							break;
						}
					}
					std::cout << std::endl;
				}
				std::cout << std::endl;
			}

			return true;
		}
	}

	uint32_t Camera::Impl::numCameras()
	{
		for(int vd=0;vd<4;vd++)
		{
			std::string dev_name = "/dev/video" + lexical_cast<std::string>(vd);

			struct stat st;
			if(-1 == ::stat(dev_name.c_str(), &st))
			{
				continue;
			}

			if(!S_ISCHR(st.st_mode))
			{
				continue;
			}

			int fd = ::open(dev_name.c_str(), O_RDWR | O_NONBLOCK, 0);

			if(-1 == fd)
			{
			    continue;
			}

			std::cout << "Camera info[" << dev_name << "]:" << std::endl;
			std::cout << std::endl;


			if(getCameraInfo(fd))
				::close(fd);
			else
				::close(fd);
		}

		return true;
	}

	Camera::Impl::Impl(Camera &camera, uint32_t camidx)
		: m_Camera(camera)
		, m_FD(-1)
	{

	}

    Camera::Impl::~Impl()
    {
    }

    bool Camera::Impl::isOpen() const
    {
    	return m_FD > 0;
    }

    system::error_code Camera::Impl::open()
    {
    	return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Camera::Impl::close()
    {
    	return system::errc::make_error_code(system::errc::success);
    }

    system::error_code Camera::Impl::getCaptureFPSRange(Camera::FPSRange &fr) const
    {
    	return system::errc::make_error_code(system::errc::success);
    }

} //namespace hal
} //namespace uquad
