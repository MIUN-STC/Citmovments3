#pragma once

#include <float.h>
#include <stdint.h>

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


void Map_Linear_u16v_float
(
   uint16_t const * Source, 
   float * Destination, 
   size_t Count, 
   uint16_t A0, 
   uint16_t A1, 
   uint16_t B0, 
   uint16_t B1
)
{
   for 
   (size_t I = 0; I < Count; I = I + 1)
   {Destination [I] = (float) Map_Linear_int (Source [I], A0, A1, B0, B1);}
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
   //new scale
   X = X * DB;
   //zero division protection
   if (DA == 0) {return B1;};
   X = X / DA;
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
