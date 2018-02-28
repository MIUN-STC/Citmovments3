// g++ test_blobber.cpp -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video -o blobber && ./grab | ./blobber

#include <iostream>
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
   assert (R == Size);
}


int main (int argc, char * argv [])
{
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);


	int Should_Run = 1;

	struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
	cv::Mat M1 (Lepton3_Height, Lepton3_Width, CV_16U, Pixmap);
	cv::Mat M2 (Lepton3_Height, Lepton3_Width, CV_8UC1);
	cv::Mat M3 (Lepton3_Height, Lepton3_Width, CV_8UC3);
	cv::Mat Foreground;

	cv::namedWindow ("W1", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	cv::setWindowProperty ("W1", CV_WND_PROP_FULLSCREEN, CV_WINDOW_FULLSCREEN);
	//cv::resizeWindow ("W1", Lepton3_Width, Lepton3_Height);

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
	std::vector<cv::KeyPoint> Keypoints;
	std::vector<cv::KeyPoint> Keypoints1 (4);

	cv::Ptr<cv::BackgroundSubtractor> Subtractor = cv::createBackgroundSubtractorMOG2 ();

	cv::RNG rng (0xFFF1231);

	for (size_t I = 0; I < Keypoints1.size (); I = I + 1)
	{
		Keypoints1 [I].class_id = I;
		Keypoints1 [I].pt.x = rng.uniform (0, Lepton3_Width);
		Keypoints1 [I].pt.y = rng.uniform (0, Lepton3_Height);
	}


	while (Should_Run)
	{

		Reciever (Pixmap);
		Subtractor->apply (M1, Foreground);
		Foreground.convertTo (M2, CV_8U);
		cv::GaussianBlur (M2, M2, cv::Size (11, 11), 3.5, 3.5);
		Blobber->detect (M2, Keypoints);
		cv::cvtColor (M2, M3, cv::COLOR_GRAY2BGR);
		//cv::drawKeypoints (M2, Keypoints, M3, cv::Scalar (0, 100, 255), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

		
		for (size_t I = 0; I < Keypoints.size (); I = I + 1)
		{
			float Distance_Min = 10000;
			float Distance;
			ssize_t Index_Min = -1;
			for (size_t J = 0; J < Keypoints1.size (); J = J + 1)
			{
				Distance = cv::norm (Keypoints1 [J].pt - Keypoints [I].pt);
				if ((Keypoints1 [J].octave > 0) && (Distance > 10.0f)) {continue;};
				if (Distance < Distance_Min)
				{
					Distance_Min = Distance;
					Index_Min = J;
				}
			}
			if (Index_Min == -1) {continue;};
			Keypoints [I].class_id = Keypoints1 [Index_Min].class_id;
			Keypoints1 [Index_Min].pt = Keypoints [I].pt;
			
			Keypoints1 [Index_Min].octave = 100;
		}

		for (size_t I = 0; I < Keypoints1.size (); I = I + 1)
		{
			if (Keypoints1 [I].octave > 0) 
			{
				Keypoints1 [I].octave -= 1;
			};
		}
		
		
		
		
		for (size_t I = 0; I < Keypoints1.size (); I = I + 1)
		{
			char Buffer [128];
			sprintf (Buffer, "%d", Keypoints1 [I].class_id);
			cv::putText (M3, Buffer, Keypoints1 [I].pt + cv::Point2f (-3.0f, 3.0f), CV_FONT_HERSHEY_SCRIPT_SIMPLEX, 0.4, cv::Scalar (0, 0, 255), 1);
			cv::circle (M3, Keypoints1 [I].pt, 6.0f, cv::Scalar (0, 255, 0), 1);
		}


		for (size_t I = 0; I < Keypoints.size (); I = I + 1)
		{
			cv::circle (M3, Keypoints [I].pt, 6.0f, cv::Scalar (255, 0, 0), 1);
		}

		cv::imshow ("W1", M3);
		//cv::imshow ("W1", M2);

		int Key = cv::waitKey (1);
		if (Key == 'q') {Should_Run = 0;};
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

