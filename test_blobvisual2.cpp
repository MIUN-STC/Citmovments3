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

#include "../Lepton/Lepton.h"
#include "../Lepton/Lepton_Pixels.h"


#include "util.h"


void Reciever (struct Lepton_Pixel_Grayscale16 * Pixmap)
{
   size_t const Size = sizeof (struct Lepton_Pixel_Grayscale16 [Lepton3_Width * Lepton3_Height]);
   int R = read (STDIN_FILENO, Pixmap, Size);
   Assert (R == Size, "read error. Readed %d of %d", R, Size);
}




void Thresholder (uint16_t &P, int const * Position)
{
	(void) Position;
	if (P > 70)
	{
		P = UINT16_MAX;
	}
}


int main (int argc, char * argv [])
{
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);


	bool Should_Run = true;

	//Use shared memory between Pixmap and M1.
	//TODO: Rethink matrix variable name.
	struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
	cv::Mat M0 (Lepton3_Height, Lepton3_Width, CV_16U, Pixmap);
	cv::Mat M0_BG (Lepton3_Height, Lepton3_Width, CV_16U);
	cv::Mat M1 (Lepton3_Height, Lepton3_Width, CV_16U);
	
	cv::Mat M2 (Lepton3_Height, Lepton3_Width, CV_8UC1);
	cv::Mat M3 (Lepton3_Height, Lepton3_Width, CV_8UC3);
	cv::Mat Foreground;

	cv::namedWindow ("W1", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	cv::namedWindow ("W2", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	cv::resizeWindow ("W1", Lepton3_Width*5, Lepton3_Height*5);
	cv::resizeWindow ("W2", Lepton3_Width*5, Lepton3_Height*5);
	
	
	//cv::setWindowProperty ("W1", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	

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

	
	//cv::Ptr<cv::BackgroundSubtractor> Subtractor = cv::createBackgroundSubtractorMOG2 (400, 14, true);
	//cv::Ptr<cv::BackgroundSubtractorMOG2> Subtractor = cv::createBackgroundSubtractorMOG2 ();
	//cv::Ptr<cv::BackgroundSubtractorMOG2> Subtractor = cv::createBackgroundSubtractorMOG2 ();
	//cv::Ptr<cv::BackgroundSubtractorKNN> Subtractor = cv::createBackgroundSubtractorKNN (500, 50);

	while (Should_Run)
	{

		Reciever (Pixmap);
		
		cv::absdiff (M0, M0_BG, M1);
		M0_BG = (M0_BG * 0.99) + (M0 * 0.01);
		
		//cv::normalize (M1, M2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		
		M1.forEach <uint16_t> (&Thresholder);
		
		cv::imshow ("W1", M1);
		
		
		cv::normalize (M0_BG, M2, 0, 255, cv::NORM_MINMAX, CV_8UC1);
		cv::imshow ("W2", M2);
		

		
		//cv::GaussianBlur (M0, M0, cv::Size (3, 3), 3.5, 3.5);
		//Subtractor->apply (M0, Foreground);
		//Foreground.convertTo (M2, CV_8U);
		//cv::medianBlur (M2, M2, 3);
		//cv::GaussianBlur (M2, M2, cv::Size (5, 5), 3.5, 3.5);
		/*
		Blobber->detect (M2, Targets);
		cv::cvtColor (M2, M3, cv::COLOR_GRAY2BGR);
		//cv::drawKeypoints (M2, Targets, M3, cv::Scalar (0, 100, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);
		for (size_t I = 0; I < Targets.size (); I = I + 1)
		{
			cv::circle (M3, Targets [I].pt, Targets [I].size, cv::Scalar (255, 0, 0), 1);
		}
		*/

		//cv::imshow ("W1", M3);

		int Key = cv::waitKey (1);
		if (Key == 'q') {Should_Run = false;};
		if (Key == 'u') {M0.copyTo (M0_BG);}
		
		
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

