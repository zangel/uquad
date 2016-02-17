#include "yuv.h"
#include "util.h"
#include "video_device.h"
#include "capture.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <errno.h>

static int DEVICE_DESCRIPTOR = -1;
static int width = 320;
static int height = 240;
static int* RGB_BUFFER = NULL;
static int* Y_BUFFER = NULL;

int print_formats()
{
	struct v4l2_fmtdesc fmt;
	int fmt = 0;


}

int print_info()
{
	struct v4l2_capability cap;
	if(-1 == xioctl(DEVICE_DESCRIPTOR, VIDIOC_QUERYCAP, &cap))
	{
		if(EINVAL == errno)
		{
			printf("not a valid V4L2 device");
			return ERROR_LOCAL;
		}
		else
		{
			return errnoexit("VIDIOC_QUERYCAP");
		}
	}

	printf("Camera info:\n");
	printf("\tdriver:%s\n", cap.driver);
	printf("\tcard:%s\n", cap.card);
	printf("\tbus_info:%s\n", cap.bus_info);

	return 0;
}

int main(int argc, char **argv)
{
    int result = open_device(argv[1], &DEVICE_DESCRIPTOR);
    
    if(result == ERROR_LOCAL)
    {
    	printf("failed to open camera!");
        return result;
    }

    print_info();


    result = init_device(DEVICE_DESCRIPTOR, width, height);
    if(result == ERROR_LOCAL)
    {
    	printf("failed to init camera!");
        return result;
    }

    result = start_capture(DEVICE_DESCRIPTOR);
    if(result != SUCCESS_LOCAL)
    {
        stop_camera(&DEVICE_DESCRIPTOR, RGB_BUFFER, Y_BUFFER);
        printf("Unable to start capture, resetting device");
    }
    else
    {
    	printf("Camera opened!");
    }
    
    stop_camera(&DEVICE_DESCRIPTOR, RGB_BUFFER, Y_BUFFER);

    return result;

}
