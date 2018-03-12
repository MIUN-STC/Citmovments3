//gcc test_grab.c -std=gnu11 -fdiagnostics-color -Wall -Wno-missing-braces -lwiringPi -o grab

#include "util.h"

#define Lepton_Assert Assert_C

#include "../Lepton/Lepton_SPI.h"
#include "../Lepton/Lepton_I2C.h"
#include "../Lepton/Lepton_Strings.h"
#include "../Lepton/Lepton_Stream.h"
#include "../Lepton/Lepton_Endian.h"

//printf
#include <stdio.h>

//timerfd_create
#include <sys/timerfd.h>

//read
#include <unistd.h>

//strerror
#include <string.h>

//errrno
#include <errno.h>

//
#include <signal.h>

//INT_EDGE_RISING
//wiringPiISR
//wiringPiSetup
#include <wiringPi.h>



int Lep_Execute 
(
	int Device, 
	int Command,
	void * Data,
	size_t Size,
	int Micro_Sleep, 
	size_t Trial_Count
)
{
	int Stage = 0;
	uint16_t Status;
	{
		char Buffer [17] = {'\0'};
		Lepton_Strings_Base_Converter (be16toh (Status), Buffer, 16, 2);
		Log ("Lepton device (%d): status = %16s", Device, Buffer);
	}
	for (size_t I = 0; I < Trial_Count; I = I + 1)
	{
		Log ("Lepton device (%d): execute command, trial %i", Device, (int) I);
		Lepton_I2C_Execute 
		(
			Device, 
			Command, 
			Data,
			Size,
			&(Status),
			&(Stage)
		);
		{
			char Buffer [17] = {'\0'};
			Lepton_Strings_Base_Converter (be16toh (Status), Buffer, 16, 2);
			Log ("Lepton device (%d): status = %16s", Device, Buffer);
		}
		if (Stage == 0) {return 1;}
		usleep (Micro_Sleep);
	}
	return 0;
}


void Reboot (int Device)
{
	int const Micro_Sleep = 10;
	size_t const Trial_Count = 10;
	
	{
		int Status;
		Status = Lepton_I2C_Status (Device);
		char Buffer [17] = {'\0'};
		Lepton_Strings_Base_Converter (be16toh (Status), Buffer, 16, 2);
		Log ("Lepton device (%d): status = %16s", Device, Buffer);
	}
	
	
	Log ("Lepton device (%d): Rebooting", Device);
	Lepton_I2C_Write_Command (Device, Lepton_I2C_Command_Reboot);
	sleep (3);
	
	
	Log ("Lepton device (%d): Enabling vsync", Device);
	{
		struct Lepton_I2C_GPIO_Mode GPIO_Mode;
		GPIO_Mode.Value = htobe16 (Lepton_I2C_GPIO_Mode_Vsync);
		Lep_Execute 
		(
			Device, 
			Lepton_I2C_Command_GPIO_Mode_Set,
			(void *) &GPIO_Mode,
			sizeof (GPIO_Mode),
			Micro_Sleep,
			Trial_Count
		);
	}
	
	Log ("Lepton device (%d): Disabling FFC", Device);
	{
		struct Lepton_I2C_Shutter_Mode Mode;
		int Result;
		
		Result = Lep_Execute 
		(
			Device, 
			Lepton_I2C_Command_FFC_Mode_Get,
			(void *) &Mode,
			sizeof (Mode),
			Micro_Sleep,
			Trial_Count
		);
		if (!Result) {return;};
		
		//If successful then convert the received data from big endian to host endian.
		Lepton_Endian_be16tohv (sizeof (struct Lepton_I2C_Shutter_Mode) / sizeof (uint16_t), (uint16_t *) &Mode);
		
		Log ("%-40s : %010zu", "Shutter_Mode", (uint32_t) Mode.Shutter_Mode);
		Log ("%-40s : %010zu", "Temp_Lockout_State", (uint32_t) Mode.Temp_Lockout_State);
		Log ("%-40s : %010zu", "Video_Freeze_During_FFC", (uint32_t) Mode.Video_Freeze_During_FFC);
		Log ("%-40s : %010zu", "FFC_Desired", (uint32_t) Mode.FFC_Desired);
		Log ("%-40s : %010zu", "Elapsed_Time_Since_Last_FFC", (uint32_t) Mode.Elapsed_Time_Since_Last_FFC);
		Log ("%-40s : %010zu", "Desired_FFC_Period", (uint32_t) Mode.Desired_FFC_Period);
		Log ("%-40s : %010zu", "Explicit_Command_To_Open", (uint32_t) Mode.Explicit_Command_To_Open);
		Log ("%-40s : %010i", "Desired_FFC_Temp_Delta", (uint16_t) Mode.Desired_FFC_Temp_Delta);
		Log ("%-40s : %010i", "Imminent_Delay", (uint16_t) Mode.Imminent_Delay);
		
		//Disable shutter
		Mode.Shutter_Mode = 0;
		
		//Convert the data from host endian to big endian
		Lepton_Endian_htobe16v (sizeof (struct Lepton_I2C_Shutter_Mode) / sizeof (uint16_t), (uint16_t *) &Mode);
		
		Result = Lep_Execute 
		(
			Device, 
			Lepton_I2C_Command_FFC_Mode_Set,
			(void *) &Mode,
			sizeof (Mode),
			Micro_Sleep,
			Trial_Count
		);
		if (!Result) {return;};
		
		
		Result = Lep_Execute 
		(
			Device, 
			Lepton_I2C_Command_FFC_Mode_Get,
			(void *) &Mode,
			sizeof (Mode),
			Micro_Sleep,
			Trial_Count
		);
		if (!Result) {return;};
		
		//If successful then convert the received data from big endian to host endian.
		Lepton_Endian_be16tohv (sizeof (struct Lepton_I2C_Shutter_Mode) / sizeof (uint16_t), (uint16_t *) &Mode);
		
		Log ("%-40s : %010zu", "Shutter_Mode", (uint32_t) Mode.Shutter_Mode);
		Log ("%-40s : %010zu", "Temp_Lockout_State", (uint32_t) Mode.Temp_Lockout_State);
		Log ("%-40s : %010zu", "Video_Freeze_During_FFC", (uint32_t) Mode.Video_Freeze_During_FFC);
		Log ("%-40s : %010zu", "FFC_Desired", (uint32_t) Mode.FFC_Desired);
		Log ("%-40s : %010zu", "Elapsed_Time_Since_Last_FFC", (uint32_t) Mode.Elapsed_Time_Since_Last_FFC);
		Log ("%-40s : %010zu", "Desired_FFC_Period", (uint32_t) Mode.Desired_FFC_Period);
		Log ("%-40s : %010zu", "Explicit_Command_To_Open", (uint32_t) Mode.Explicit_Command_To_Open);
		Log ("%-40s : %010i", "Desired_FFC_Temp_Delta", (uint16_t) Mode.Desired_FFC_Temp_Delta);
		Log ("%-40s : %010i", "Imminent_Delay", (uint16_t) Mode.Imminent_Delay);
	}
}


int I2C_Device;
int SPI_Device;
struct Lepton_Pixel_Grayscale16 Pixmap [Lepton3_Width * Lepton3_Height];
_Atomic size_t Safe_Counter = 0;


void Interrupt_Handle ()
{
   int32_t Result;
   Result = Lepton_Stream_Accept (SPI_Device, Pixmap);  
   //By then, all 4 segment of the pixmap frame should be received.
   if (Result != 4) {return;}
   
   //Frame received succesfully so reset Safe_Counter.
   Safe_Counter = 0;
   
   Result = write (STDOUT_FILENO, Pixmap, sizeof (Pixmap));
   
   //Ignore SIGPIPE errors.
   if (Result == -1 && errno == EPIPE) {return;}
   Assert (Result == sizeof (Pixmap), "wrote %i of %i to STDOUT_FILENO", Result, sizeof (Pixmap));
}


//wiringPi handling the interrupt of GPIO3 vsync of FLIR Lepton.
void Setup_wiringPi ()
{
   int Result;
   Result = wiringPiSetup ();
   Assert (Result >= 0, "wiringPiSetup returned %i", Result);
   piHiPri (99);
   int Pin = 0;
   int Edge = INT_EDGE_RISING;
   wiringPiISR (Pin, Edge, &Interrupt_Handle);
}


/*
void Signal_Handler (int Number)
{
	Log ("SIGPIP %d", Number);
}
*/


int main (int argc, char * argv [])
{ 
	//No argument is used currently.
	Assert (argc == 1, "argc = %i", argc);
	Assert (argv != NULL, "argv = %p", argv);


	//Ignore SIGPIPE interrupt.
	//SIGPIPE interrupt can happen when the reading end of the pipe closes while pipe is being written to.
	signal (SIGPIPE, SIG_IGN);
	//signal (SIGPIPE, Signal_Handler);

	//I2C comm is used for setting/getting the camera registers.
	Log ("Open %s", "/dev/i2c-1");
	I2C_Device = Lepton_I2C_Open ("/dev/i2c-1");
	Reboot (I2C_Device);
	
	//SPI comm is only used for receiving frames.
	//SPI should only receive on v-sync interrupt.
	//wiringPi is used to setup interrupt from FLIR Lepton GPIO3 v-sync.
	Log ("Open %s", "/dev/spidev0.0");
	SPI_Device = Lepton_SPI_Open ("/dev/spidev0.0");
	Setup_wiringPi ();
	
	while (1)
	{
		//How frequent to check Safe_Counter.
		sleep (1);
		
		//The camera is not recieving frames if the Safe_Counter starts to increase.
		//If it goes too high then reboot the camera as it has probobly gone haywire.
		Safe_Counter ++;
		if (Safe_Counter > 3)
		{
			Safe_Counter = 0;
			Log ("Lepton device (%d): Camera has gone haywire! Rebooting now to fix the problem", I2C_Device);
			Reboot (I2C_Device);
		}
	}
}



