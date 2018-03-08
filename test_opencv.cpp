// g++ test_opencv.cpp -lopencv_core -lopencv_highgui -lopencv_features2d -lopencv_imgproc -lopencv_video

#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>

#include <errno.h>


#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/video/background_segm.hpp>


#include "util.h"





int main (int argc, char * argv [])
{
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);

	cv::Mat M1 (10, 10, CV_8U);
	M1.at<uint8_t>(0,0) = 0xFF;
	M1.at<uint8_t>(0,1) = 0x00;
	M1.at<uint8_t>(0,2) = 0xFF / 2;
	
	cv::namedWindow ("W1", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);

	bool Should_Run = true;
	while (Should_Run)
	{
		cv::imshow ("W1", M1);
		int Key = cv::waitKey (1);
		if (Key == 'q') {Should_Run = 0;};
	}
	return 0;
}

