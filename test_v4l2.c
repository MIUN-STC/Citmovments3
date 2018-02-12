#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include "../Lepton/Lepton.h"

static int xioctl (int Device, int Request, void * Arg)
{
   int Result;
   do {Result = ioctl (Device, Request, Arg);}
   while ((Result != -1) && (errno == EINTR));
   return Result;
}

static int xioctl_safe (int Device, int Request, void * Arg)
{
   int Result;
   Result = xioctl (Device, Request, Arg);
   assert (Result != -1);
   return Result;
}


static int select_safe (int FD, fd_set * Set, struct timeval * T)
{
   int Result;
   Result = select (FD + 1, Set, NULL, NULL, T);
   assert (Result != -1);
   return Result;
}

void Capture (int Device)
{
   struct v4l2_buffer Buffer = {0};
   Buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   Buffer.memory = V4L2_MEMORY_MMAP;
   Buffer.index = 0;
   xioctl_safe (Device, VIDIOC_QBUF, &Buffer);
   
   fd_set FDS;
   FD_ZERO (&FDS);
   FD_SET (Device, &FDS);
   struct timeval T = {0};
   T.tv_sec = 2;
   select_safe (Device + 1, &FDS, &T);
   printf ("Frame found!\n");
   
   
   xioctl_safe (Device, VIDIOC_DQBUF, &Buffer);
   
	
}


int main (int argc, char * argv [])
{
	assert (argc > 0);
	assert (argv != NULL);
   int Device;
   Device = open ("/dev/video0", O_RDWR);
   assert (Device >= 0);
   
   struct v4l2_capability Cap = {0};
   xioctl_safe (Device, VIDIOC_QUERYCAP, &Cap);
   printf ("Driver       : %s\n", Cap.driver);
   printf ("Card         : %s\n", Cap.card);
   printf ("Bus          : %s\n", Cap.bus_info);
   printf ("Version      : %d.%d\n", (Cap.version >> 16) && 0xFF, (Cap.version >> 24) && 0xFF);
   printf ("Capabilities : %08x\n", Cap.capabilities);
   
   struct v4l2_cropcap Crop = {0};
   Crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   xioctl_safe (Device, VIDIOC_CROPCAP, &Crop);
   printf ("Bounds       : %dx%d+%d+%d\n", Crop.bounds.width, Crop.bounds.height, Crop.bounds.left, Crop.bounds.top);
   printf ("Default      : %dx%d+%d+%d\n", Crop.defrect.width, Crop.defrect.height, Crop.defrect.left, Crop.defrect.top);
   printf ("Aspect       : %d/%d\n", Crop.pixelaspect.numerator, Crop.pixelaspect.denominator);
   
   struct v4l2_fmtdesc Formatdesc = {0};
   Formatdesc.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   char FourCC [5] = {0};
   char C, E;
   while (1)
   {
      int Result = xioctl (Device, VIDIOC_ENUM_FMT, &Formatdesc);
      //printf ("xioctl : %d\n", Result);
      if (Result != 0) {break;};
      strncpy (FourCC, (char *) & Formatdesc.pixelformat, 4);
      C = (Formatdesc.flags & 1) ? 'C' : '.';
      E = (Formatdesc.flags & 2) ? 'E' : '.';
      printf ("Format %02i    : %s %c%c %s\n", Formatdesc.index, FourCC, C, E, Formatdesc.description);
      Formatdesc.index ++;
   }
   
   struct v4l2_format Format = {0};
   Format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   Format.fmt.pix.width = 160;
   Format.fmt.pix.width = 120;
   Format.fmt.pix.pixelformat = V4L2_PIX_FMT_Y16;
   Format.fmt.pix.field = V4L2_FIELD_NONE;
   xioctl_safe (Device, VIDIOC_S_FMT, &Format);
   
   struct v4l2_requestbuffers Request = {0};
   Request.count = 1;
   Request.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   Request.memory = V4L2_MEMORY_MMAP;
   xioctl_safe (Device, VIDIOC_REQBUFS, &Request);

   struct v4l2_buffer Buffer = {0};
   Buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
   Buffer.memory = V4L2_MEMORY_MMAP;
   Buffer.index = 0;
   xioctl_safe (Device, VIDIOC_QUERYBUF, &Buffer);

   void * Bufmap;
   Bufmap = mmap (NULL, Buffer.length, PROT_READ | PROT_WRITE, MAP_SHARED, Device, Buffer.m.offset);
   printf ("Length       : %d\n", Buffer.length);
   printf ("Address      : %p\n", Bufmap);
   printf ("Image Length : %d\n", Buffer.bytesused);
   
   
   
   xioctl_safe (Device, VIDIOC_STREAMON, &Buffer.type);
   
   
   //cv::Mat M1 (160, 120, CV_16U, Bufmap);
   //cv::Mat Foreground;
   //cv::namedWindow ("W1", CV_WINDOW_NORMAL);
   //cv::resizeWindow ("W1", Lepton3_Width, Lepton3_Height);
   //cv::Ptr<cv::BackgroundSubtractor> Subtractor = cv::createBackgroundSubtractorMOG2 ();
   
   int Should_Run = 1;
   while (Should_Run)
   {
      //int Key = cv::waitKey (1);
      //if (Key == 'q') {Should_Run = 0;};
      Capture (Device);
      //Subtractor->apply (M1, Foreground);
      //cv::imshow ("W1", M1);
   }
}


