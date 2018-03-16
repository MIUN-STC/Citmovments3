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

#include "util.h"
#include "map.h"


void Reciever (uint16_t Pixmap [])
{
   size_t const Size = sizeof (uint16_t [Lepton3_Width * Lepton3_Height]);
   int R = read (STDIN_FILENO, Pixmap, Size);
   Assert (R == Size, "read error. Readed %d of %d", R, Size);
}


int main (int argc, char * argv [])
{
	Assert (argc == 1, "argc %i.", argc);
	Assert (argv[0] != NULL, "argv0 %p.", argv[0]);
	
	bool Should_Run = true;
	
	uint16_t Pixmap [Lepton3_Width * Lepton3_Height];
	float Background1 [Lepton3_Width * Lepton3_Height];
	float Foreground1 [Lepton3_Width * Lepton3_Height];
	float Source1 [Lepton3_Width * Lepton3_Height];
	uint8_t Result1 [Lepton3_Width * Lepton3_Height];
	uint8_t Result2 [Lepton3_Width * Lepton3_Height];
	
	
	cv::Mat M1 (Lepton3_Height, Lepton3_Width, CV_8U, Result1);
	cv::Mat M2 (Lepton3_Height, Lepton3_Width, CV_8U, Result2);


	cv::namedWindow ("W1", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	cv::namedWindow ("W2", CV_WINDOW_NORMAL | CV_WINDOW_OPENGL);
	cv::resizeWindow ("W1", Lepton3_Width*5, Lepton3_Height*5);
	cv::resizeWindow ("W2", Lepton3_Width*5, Lepton3_Height*5);
	
	while (Should_Run)
	{

		Reciever (Pixmap);
		Copy_u16v_floatv (Lepton3_Width * Lepton3_Height, Pixmap, Source1);
		Background_Subtraction_floatv (Lepton3_Width * Lepton3_Height, Source1, Foreground1, Background1, 0.01f, true);
		Copy_floatv_u8v (Lepton3_Width * Lepton3_Height, Foreground1, Result1);
		Copy_floatv_u8v (Lepton3_Width * Lepton3_Height, Background1, Result2);
		cv::imshow ("W1", M1);
		cv::normalize (M2, M2, 0, 255, cv::NORM_MINMAX, CV_8U);
		cv::imshow ("W2", M2);
		
		int Key = cv::waitKey (1);
		switch (Key)
		{
			case 'q':
			Should_Run = false;
			break;
			
			case 'u':
			Copy_u16v_floatv (Lepton3_Width * Lepton3_Height, Pixmap, Background1);
			break;
			
			case 'p':
			while (1) 
			{
				Key = cv::waitKey (1);
				if (Key == 'p') {break;};
			}
			break;
		}
	}
	return 0;
}


