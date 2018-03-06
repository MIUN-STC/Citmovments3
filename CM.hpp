#pragma once

#include "../Lepton/Lepton.h"
#include "../Lepton/Lepton_Pixels.h"


struct CM_Tracker
{
	//Position
	//Can be used to check if the target is at the border.
	cv::Point2f P;
	
	//Delta position
	//Can be used to calculate the angle of speed direction.
	cv::Point2f D;
	
	//Angle of delta position.
	//Can be used to check angle of departure.
	float Angle;
	
	//Tracking duration
	//Can be used to filter out false targets.
	int Duration;
	
	// 0          : Tracker has no target and is not tracking.
	// 1 .. Max-1 : Tracker has a target but can not find it.
	// Max        : Tracker has a target and is tracking.
	//Can be used to check if the target has actually left the premise.
	//Can be used to update the model according to different states.
	//Can be used to smooth the state beetween target existance and non existance (rounding up to state existing).
	int Persistence;
	
	//Why do we need this?
	//Only used for visual graphics.
	//The Id can be the tracker index instead.
	int Id;
	

};


void Persistent_Tracker 
(
	std::vector<cv::KeyPoint>& Targets,
	std::vector<CM_Tracker>& Trackers,
	float Proximity = 10.0f,
	int Persistence = 100
)
{
	for (size_t I = 0; I < Trackers.size (); I = I + 1)
	{
		if (Trackers [I].Persistence < Persistence)
		{
			//Not tracking then decrese the delta position.
			Trackers [I].D = Trackers [I].D * 0.95f;
		}
		
		//Negative not allowed.
		if (Trackers [I].Persistence > 0) 
		{
			//Tracker that does not track will lose interest and 
			//release it self to tracke other targets.
			//Persistence will be set to max if it tracking.
			Trackers [I].Persistence -= 1;
		};
	}
	
	
	for (size_t I = 0; I < Targets.size (); I = I + 1)
	{
		float Distance_Min = 10000;
		float Distance;
		ssize_t Index_Min = -1;
		
		//Find the closest tracker to the target and
		//exchange information.
		for (size_t J = 0; J < Trackers.size (); J = J + 1)
		{
			Distance = cv::norm (Trackers [J].P - Targets [I].pt);
			
			//It is very important to update used trackers also.
			//If the tracker is being used then only track the target in proximity.
			if ((Trackers [J].Persistence > 0) && (Distance > Proximity)) {continue;};
			if (Distance < Distance_Min)
			{
				Distance_Min = Distance;
				Index_Min = J;
			}
		}
		
		if (Index_Min == -1) {continue;};
		
		Trackers [Index_Min].D = 0.9f * Trackers [Index_Min].D + (Targets [I].pt - Trackers [Index_Min].P) * 0.1f;
		Trackers [Index_Min].Angle = atan2f (Trackers [Index_Min].D.y, Trackers [Index_Min].D.x);
		
		Trackers [Index_Min].P = Targets [I].pt;
		Trackers [Index_Min].Persistence = Persistence;
		Trackers [Index_Min].Duration += 1.0f;
	}
}


struct CM_Counter
{
	size_t N;
	size_t S;
	size_t W;
	size_t E;
};

size_t const CM_Size = 10;
cv::Rect const CM_N (CM_Size, 0, Lepton3_Width - (CM_Size*2), CM_Size);
cv::Rect const CM_S (CM_Size, Lepton3_Height - CM_Size, Lepton3_Width - (CM_Size*2), CM_Size);
cv::Rect const CM_W (0, CM_Size, CM_Size, Lepton3_Height - (CM_Size*2));
cv::Rect const CM_E (Lepton3_Width - CM_Size, CM_Size, CM_Size, Lepton3_Height - (CM_Size*2));

cv::Rect const CM_NW (0, 0, CM_Size, CM_Size);
cv::Rect const CM_NE (Lepton3_Width - CM_Size, 0, CM_Size, CM_Size);
cv::Rect const CM_SW (0, Lepton3_Height - CM_Size, CM_Size, CM_Size);
cv::Rect const CM_SE (Lepton3_Width - CM_Size, Lepton3_Height - CM_Size, CM_Size, CM_Size);



void Countman 
(
	std::vector<CM_Tracker>& Trackers,
	struct CM_Counter& Counter
)
{
	for (size_t I = 0; I < Trackers.size (); I = I + 1)
	{
		//Check if the target has been gone for a while.
		//This is making sure that the target is not deciding to go
		//back when it is outside the view of camera.
		if (Trackers [I].Persistence != 1) {continue;}
		
		//Check if the target has been tracked for a while. 
		float Duration = Trackers [I].Duration;
		if (Duration < 30.0f){continue;}
		Trackers [I].Duration = 0;
		
		//Flag variable for if the target has beed counted or not.
		bool Counted = false;
		
		//Rename target position
		cv::Point2f P = Trackers [I].P;
		
		//Angle of the targets direction in degrees.
		float Angle = (180.0f / M_PI) * Trackers [I].Angle;
		
		//Check if the target is whithin the counting box.
		if (0) {}
		else if (CM_N.contains (P)) {Counter.N ++; Counted = true;}
		else if (CM_S.contains (P)) {Counter.S ++; Counted = true;}
		else if (CM_W.contains (P)) {Counter.W ++; Counted = true;}
		else if (CM_E.contains (P)) {Counter.E ++; Counted = true;}
		else if (CM_NE.contains (P)) 
		{
			//Angle of departure in the corner.
			if (Angle < -45.0f) {Counter.S ++;}
			else {Counter.E ++;}
			Counted = true;
		}
		else if (CM_SE.contains (P)) 
		{
			//Angle of departure in the corner.
			if (Angle < 45.0f) {Counter.E ++;}
			else {Counter.N ++;}
			Counted = true;
		}
		else if (CM_NW.contains (P)) 
		{
			//Angle of departure in the corner.
			if (Angle < 255.0f) {Counter.W ++;}
			else {Counter.N ++;}
			Counted = true;
		}
		else if (CM_SW.contains (P)) 
		{
			//Angle of departure in the corner.
			if (Angle < 135.0f) {Counter.S ++;}
			else {Counter.W ++;}
			Counted = true;
		}
		
		if (Counted)
		{
			printf ("ID    = %i\n", Trackers [I].Id);
			printf ("Angle = %f\n", Angle);
			printf ("N : %d\n", Counter.N);
			printf ("S : %d\n", Counter.S);
			printf ("W : %d\n", Counter.W);
			printf ("E : %d\n", Counter.E);
			printf ("\n");
			fflush (stdout);
			//Trackers [I].P = {Lepton3_Width / 2.0f, Lepton3_Height/2.0f};
			Trackers [I].D = {0.0f, 0.0f};
		}
	}
}
