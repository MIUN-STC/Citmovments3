#include "util.h"


#include "../Lepton/Lepton_I2C.h"
#include "../Lepton/Lepton_Endian.h"
#include "../Lepton/Lepton_Strings.h"


void Get_Shutter 
(
	int Device, 
	struct Lepton_I2C_Shutter_Mode * Mode,
	int Micro_Sleep, 
	size_t Trial_Count
)
{
	int Stage = 0;
	uint16_t Status;
	for (size_t I = 0; I < Trial_Count; I = I + 1)
	{
		Log ("Lepton_I2C_Execute Lepton_I2C_Command_FFC_Mode_Get. Try %i\n", (int) I);
		Lepton_I2C_Execute 
		(
			Device, 
			Lepton_I2C_Command_FFC_Mode_Get, 
			(void *) Mode,
			sizeof (struct Lepton_I2C_Shutter_Mode),
			&(Status),
			&(Stage)
		);
		char Buffer [17] = {'\0'};
		Lepton_Strings_Base_Converter (be16toh (Status), Buffer, 16, 2);
		Log ("Lepton device (%d): status = %16s", Device, Buffer);
		if (Stage == 0) 
		{
			Lepton_Endian_be16tohv (sizeof (struct Lepton_I2C_Shutter_Mode) / sizeof (uint16_t), (uint16_t *) Mode);
			break;
		}
		usleep (Micro_Sleep);
	}
}


void Set_Shutter 
(
	int Device, 
	struct Lepton_I2C_Shutter_Mode * Mode,
	int Micro_Sleep, 
	size_t Trial_Count
)
{
	int Stage = 0;
	uint16_t Status;
	for (size_t I = 0; I < Trial_Count; I = I + 1)
	{
		Log ("Lepton_I2C_Execute Lepton_I2C_Command_FFC_Mode_Set. Try %i\n", (int) I);
		Lepton_I2C_Execute 
		(
			Device, 
			Lepton_I2C_Command_FFC_Mode_Set, 
			(void *) Mode,
			sizeof (struct Lepton_I2C_Shutter_Mode),
			&(Status),
			&(Stage)
		);
		char Buffer [17] = {'\0'};
		Lepton_Strings_Base_Converter (be16toh (Status), Buffer, 16, 2);
		Log ("Lepton device (%d): status = %16s", Device, Buffer);
		if (Stage == 0) 
		{
			break;
		}
		usleep (Micro_Sleep);
	}
}


void Print_Shutter (struct Lepton_I2C_Shutter_Mode * Item)
{
	printf ("%40s : %010zu\n", "Shutter_Mode", (uint32_t) Item->Shutter_Mode);
	printf ("%40s : %010zu\n", "Temp_Lockout_State", (uint32_t) Item->Temp_Lockout_State);
	printf ("%40s : %010zu\n", "Video_Freeze_During_FFC", (uint32_t) Item->Video_Freeze_During_FFC);
	printf ("%40s : %010zu\n", "FFC_Desired", (uint32_t) Item->FFC_Desired);
	printf ("%40s : %010zu\n", "Elapsed_Time_Since_Last_FFC", (uint32_t) Item->Elapsed_Time_Since_Last_FFC);
	printf ("%40s : %010zu\n", "Desired_FFC_Period", (uint32_t) Item->Desired_FFC_Period);
	printf ("%40s : %010zu\n", "Explicit_Command_To_Open", (uint32_t) Item->Explicit_Command_To_Open);
	printf ("%40s : %010i\n", "Desired_FFC_Temp_Delta", (uint16_t) Item->Desired_FFC_Temp_Delta);
	printf ("%40s : %010i\n", "Imminent_Delay", (uint16_t) Item->Imminent_Delay);
}


int main (int argc, char * argv [])
{ 
	Log ("%s () function entered\n", __func__);
	
	//No argument is used currently.
	Assert (argc == 1, "argc = %i", argc);
	Assert (argv != NULL, "argv = %p", argv);
	
	fprintf (stderr, ANSIC (ANSIC_Bold, ANSIC_White, ANSIC_Black) "Hello" ANSIC_Default "\n");


	//I2C port is used for setting/getting camera registers.
	Log ("Open %s", "/dev/i2c-1");
	int I2C_Device = Lepton_I2C_Open ("/dev/i2c-1");
	
	struct Lepton_I2C_Shutter_Mode Mode;
	Log ("Mode size: %i", sizeof (Mode));
	
	//while (1)
	{
		Get_Shutter (I2C_Device, &Mode, 10, 10);
		Print_Shutter (&Mode);
		usleep (1000000);
	}
	
	//return 0;
	
	{
		Mode.Desired_FFC_Period = 500000;
		Mode.Shutter_Mode = 0;
		Lepton_Endian_htobe16v (sizeof (struct Lepton_I2C_Shutter_Mode) / sizeof (uint16_t), (uint16_t *) &Mode);
		Set_Shutter (I2C_Device, &Mode, 10, 10);
		Print_Shutter (&Mode);
		usleep (1000000);
	}
	
	{
		Get_Shutter (I2C_Device, &Mode, 10, 10);
		Print_Shutter (&Mode);
		usleep (1000000);
	}
	
	return 0;
}
