#pragma once

#include <float.h>
#include <stdint.h>
#include <math.h>

int Map_Linear_int (int X, int A0, int A1, int B0, int B1)
{
   //Crop
   if (X < A0) {return B0;}
   if (X > A1) {return B1;}
   int DA;
   int DB;
   //Delta
   DA = A1 - A0;
   DB = B1 - B0;
   //Accurate integer division.
   //Round up or down
   if (DA > DB) {DA = DA + 1; DB = DB + 1;}
   //X = (X-A0) * ((DB + K) / (DA + K)) + B0
   //Move to zero.
   X = X - A0;
   //Scale up before scaling down is important for integers.
   X = X * DB;
   //Zero division protection.            
   if (DA == 0) {return B1;};
   //Scale down
   X = X / DA;
   //Apply offset 
   X = X + B0;
   assert (X <= B1);
   assert (X >= B0);
   return X;
}


float Map_Linear_float 
(
   float X, 
   float A0, 
   float A1, 
   float B0, 
   float B1
)
{
   //cropping
   if (X < A0) {return B0;}
   if (X > A1) {return B1;}
   //calculate delta
   float DA;
   float DB;
   DA = A1 - A0;
   DB = B1 - B0;
   //move to zero
   X = X - A0;
   //zero division protection
   if (DA == 0) {return B1;};
   X = X / DA;
   //new scale
   X = X * DB;
   //new offset
   X = X + B0;
   return X;
}


void Map_Linear_floatv
(
   float const * Source, 
   float * Destination, 
   size_t Count, 
   float A0, 
   float A1, 
   float B0, 
   float B1
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = Map_Linear_float (Source [I], A0, A1, B0, B1);}
}


void Map_Linear_u16v_float
(
   uint16_t const * Source, 
   float * Destination, 
   size_t Count, 
   uint16_t A0, 
   uint16_t A1, 
   float B0, 
   float B1
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = (float) Map_Linear_float (Source [I], A0, A1, B0, B1);}
}


void Map_Linear_u16v_u16v
(
	size_t Count,
	uint16_t const Source [Count], 
	uint16_t Destination [Count], 
	uint16_t A0, 
	uint16_t A1, 
	float B0, 
	float B1
)
{
	for 
	(size_t I = 0; I < Count; I = I + 1)
	{Destination [I] = (float) Map_Linear_int (Source [I], A0, A1, B0, B1);}
}


void 
Find_Range_u16 
(uint16_t Data, uint16_t * Min, uint16_t * Max)
{
   if (Data > *Max) {*Max = Data;}
   if (Data < *Min) {*Min = Data;}
}


void 
Find_Range_u16v 
(uint16_t const * Data, size_t Count, uint16_t * Min, uint16_t * Max)
{
   assert ((Count > 0 && Data != NULL) || (Count == 0));
   for 
   (size_t I = 0; I < Count; I = I + 1) 
   {Find_Range_u16 (Data [I], Min, Max);}
   assert (*Max >= *Min);
}


void 
Find_Range_Index_u16v 
(
	uint16_t const * Data, 
	size_t Count, 
	uint16_t * Min_Value, 
	uint16_t * Max_Value, 
	size_t * Min_Index,
	size_t * Max_Index
)
{
	assert ((Count > 0 && Data != NULL) || (Count == 0));
	for (size_t I = 0; I < Count; I = I + 1) 
	{
		if (Data [I] > *Max_Value) 
		{
			*Max_Value = Data [I];
			if (Max_Index != NULL) {*Max_Index = I;} 
		}
		if (Data [I] < *Min_Value) 
		{
			*Min_Value = Data [I];
			if (Min_Index != NULL) {*Min_Index = I;} 
		}
	}
	assert (*Max_Value >= *Min_Value);
}


void 
Find_Range_Index2_u16v 
(
	uint16_t const * Data, 
	size_t Width, 
	size_t Height, 
	uint16_t * Min_Value, 
	uint16_t * Max_Value, 
	size_t * Min_Index,
	size_t * Max_Index
)
{
	for (size_t X = 0; X < Width; X = X + 1) 
	for (size_t Y = 0; Y < Height; Y = Y + 1) 
	{
		size_t I = Width * Y + X;
		
		if ((Max_Value != NULL) && (Data [I] > *Max_Value)) 
		{
			*Max_Value = Data [I];
			if (Max_Index != NULL) 
			{
				Max_Index [0] = X;
				Max_Index [1] = Y;
			} 
		}
		if ((Min_Value != NULL) && (Data [I] < *Min_Value)) 
		{
			*Min_Value = Data [I];
			if (Min_Index != NULL) 
			{
				Min_Index [0] = X;
				Min_Index [1] = Y;
			} 
		}
	}
}


void 
Find_Range_float 
(float Data, float * Min, float * Max)
{
   if (Data > *Max) {*Max = Data;}
   if (Data < *Min) {*Min = Data;}
}


void 
Find_Range_floatv 
(float const * Data, size_t Count, float * Min, float * Max)
{
   assert ((Count > 0 && Data != NULL) || (Count == 0));
   for 
   (size_t I = 0; I < Count; I = I + 1) 
   {Find_Range_float (Data [I], Min, Max);}
   assert (*Max >= *Min);
}


void Map_u16v_floatv 
(
   uint16_t const * Source, 
   float * Destination, 
   size_t Count
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = (float) Source [I];}
}


float Crop_float (float Value, float Min, float Max)
{
	if (Value > Max) {return Max;};
	if (Value < Min) {return Min;};
	return Value;
}


float Random_float (float Min, float Max)
{
	float F = (float) rand ();
	return Map_Linear_float (F, 0, RAND_MAX, Min, Max);
}


void Random_Circle_XY_float 
(
	float OX,
	float OY,
	float * X,
	float * Y,
	float Radius,
	float XX,
	float YY,
	float Width,
	float Height
)
{
	float Angle;
	Angle = Map_Linear_float ((float) rand (), 0, RAND_MAX, 0, 2 * M_PI);
	Radius = Map_Linear_float ((float) rand (), 0, RAND_MAX, 0, Radius);
	*X = OX + cos (Angle) * Radius;
	*Y = OY + sin (Angle) * Radius;
	*X = Crop_float (*X, XX, Width);
	*Y = Crop_float (*Y, YY, Height);
}


void Subtract_floatv 
(
	size_t Dim, 
	float Left [Dim], 
	float Right [Dim], 
	float Result [Dim]
)
{
	for (size_t I = 0; I < Dim; I = I + 1)
	{
		Result [I] = Left [I] - Right [I];
	}
}


void Random_Rectangle_floatv
(
	size_t Dim,
	float Result [Dim],
	float Min [Dim],
	float Max [Dim]
)
{
	for (size_t I = 0; I < Dim; I = I + 1)
	{
		Result [I] = Random_float (Min [I], Max [I]);
	}
}


void Random_Delta_Rectangle_floatv
(
	size_t Dim,
	float Result [Dim],
	float Min [Dim],
	float Max [Dim]
)
{
	for (size_t I = 0; I < Dim; I = I + 1)
	{
		Result [I] = Result [I] + Random_float (Min [I], Max [I]);
	}
}


void Random_Delta_Square_floatv
(
	size_t Dim,
	float Result [Dim],
	float Amount
)
{
	for (size_t I = 0; I < Dim; I = I + 1)
	{
		Result [I] = Result [I] + Random_float (-Amount, Amount);
	}
}


void Crop_Rectangle_floatv
(
	size_t Dim,
	float Value [Dim],
	float Result [Dim],
	float Min [Dim],
	float Max [Dim]
)
{
	for (size_t I = 0; I < Dim; I = I + 1)
	{
		Result [I] = Crop_float (Value [I], Min [I], Max [I]);
	}
}
