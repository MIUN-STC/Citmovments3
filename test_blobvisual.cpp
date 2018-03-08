// g++ test_blobmqtt.cpp -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video -o blobber && ./grab | ./blobber

#include <stdio.h>
#include <unistd.h>
#include <vector>

#include <errno.h>


#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/background_segm.hpp>

#include "CM.hpp"

#include "../Lepton/Lepton.h"
#include "../Lepton/Lepton_Pixels.h"


#include "util.h"


void Reciever (struct Lepton_Pixel_Grayscale16 * Pixmap)
{
   size_t const Size = sizeof (struct Lepton_Pixel_Grayscale16 [Lepton3_Width * Lepton3_Height]);
   int R = read (STDIN_FILENO, Pixmap, Size);
   Assert (R == Size, "read error. Readed %d of %d", R, Size);
}


int main (int argc, char * argv [])
{
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);


	bool Should_Run = true;

	struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
	cv::Mat M1 (Lepton3_Height, Lepton3_Width, CV_16U, Pixmap);
	cv::Mat M2 (Lepton3_Height, Lepton3_Width, CV_8UC1);
	cv::Mat M3 (Lepton3_Height, Lepton3_Width, CV_8UC3);
	cv::Mat Foreground;

	cv::namedWindow ("W1", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	//cv::setWindowProperty ("W1", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	cv::resizeWindow ("W1", Lepton3_Width*5, Lepton3_Height*5);

	cv::SimpleBlobDetector::Params Params;
	Params.minThreshold = 60;
	Params.maxThreshold = 255;
	Params.filterByColor = false;
	Params.blobColor = 100;
	Params.filterByArea = true;
	Params.minArea = 10;
	Params.maxArea = 100;
	Params.filterByCircularity = false;
	Params.minCircularity = 0.1;
	Params.maxCircularity = 1.0;
	Params.filterByConvexity = false;
	Params.minConvexity = 0.0;
	Params.maxConvexity = 0.5;
	Params.filterByInertia = false;
	Params.minInertiaRatio = 0.0;
	Params.maxInertiaRatio = 0.5;

	cv::Ptr<cv::SimpleBlobDetector> Blobber = cv::SimpleBlobDetector::create (Params);
	std::vector<cv::KeyPoint> Targets;
	std::vector<CM_Tracker> Trackers (4);

	cv::Ptr<cv::BackgroundSubtractor> Subtractor = cv::createBackgroundSubtractorMOG2 ();

	cv::RNG rng (0xFFF1231);

	for (size_t I = 0; I < Trackers.size (); I = I + 1)
	{
		Trackers [I].Id = I;
		Trackers [I].P.x = rng.uniform (0, Lepton3_Width);
		Trackers [I].P.y = rng.uniform (0, Lepton3_Height);
	}
	
	struct CM_Counter Counter = {0, 0, 0, 0};
	


	while (Should_Run)
	{

		Reciever (Pixmap);
		Subtractor->apply (M1, Foreground);
		Foreground.convertTo (M2, CV_8U);
		cv::GaussianBlur (M2, M2, cv::Size (11, 11), 3.5, 3.5);
		Blobber->detect (M2, Targets);
		cv::cvtColor (M2, M3, cv::COLOR_GRAY2BGR);
		//cv::drawTargets (M2, Targets, M3, cv::Scalar (0, 100, 255), cv::DrawMatchesFlags::DRAW_RICH_Targets);
		
		Persistent_Tracker (Targets, Trackers);
		Countman (Trackers, Counter);
		
		
		
		for (size_t I = 0; I < Trackers.size (); I = I + 1)
		{
			char Buffer [128];
			sprintf (Buffer, "%d", Trackers [I].Id);
			cv::putText (M3, Buffer, Trackers [I].P + cv::Point2f (-3.0f, 3.0f), CV_FONT_HERSHEY_SCRIPT_SIMPLEX, 0.4, cv::Scalar (0, 0, 255), 1);
			
			if (Trackers [I].Persistence > 0)
			{
				cv::circle (M3, Trackers [I].P, 6.0f, cv::Scalar (0, 255, 0), 1);
			}
			
			
			cv::line (M3, Trackers [I].P - (Trackers [I].D * 40.0f), Trackers [I].P + (Trackers [I].D * 40.0f), cv::Scalar (0, 255, 100));
		}


		for (size_t I = 0; I < Targets.size (); I = I + 1)
		{
			cv::circle (M3, Targets [I].pt, 6.0f, cv::Scalar (255, 0, 0), 1);
		}
		
		
		cv::rectangle (M3, CM_N, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_S, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_W, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_E, cv::Scalar (255, 0, 255));
		
		cv::rectangle (M3, CM_NE, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_NW, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_SE, cv::Scalar (255, 0, 255));
		cv::rectangle (M3, CM_SW, cv::Scalar (255, 0, 255));
		
		cv::imshow ("W1", M3);

		int Key = cv::waitKey (1);
		if (Key == 'q') {Should_Run = false;};
		if (Key == 'u') {Foreground = M1;}
		
		
		if (Key == 'p') 
		{
			while (1) 
			{
				Key = cv::waitKey (1);
				if (Key == 'p') {break;};
			}
		};
		
		
	}
	return 0;
}

