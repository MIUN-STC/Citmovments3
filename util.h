#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define ANSIC_Str1(X) #X
#define ANSIC_Str(X) ANSIC_Str1 (X)

#define ANSIC(Text_Attribute, Foreground_Color, Background_Color) \
"\x1B[" ANSIC_Str (Text_Attribute) ";3" ANSIC_Str (Foreground_Color) ";4" ANSIC_Str (Background_Color) "m"


#define ANSIC_Black   0
#define ANSIC_Red     1
#define ANSIC_Green   2
#define ANSIC_Yellow  3
#define ANSIC_Blue    4
#define ANSIC_Magenta 5
#define ANSIC_Cyan    6
#define ANSIC_White   7

#define ANSIC_Default ANSIC (0, ANSIC_White, ANSIC_Black)

//TODO: Rename color names to better names
#define KWHT "\x1B[1;37;40m"
#define KMAG "\x1B[1;35;40m"
#define KYEL "\x1B[1;33;40m"
#define KNRM "\x1B[0m"

#define TNONE "\033[0m"

#define FBLACK   "\033[30;"
#define FRED     "\033[31;"
#define FGREEN   "\033[32;"
#define FYELLOW  "\033[33;"
#define FBLUE    "\033[34;"
#define FPURPLE  "\033[35;"
#define D_FGREEN "\033[6;"
#define FWHITE   "\033[7;"
#define FCYAN    "\x1b[36m"

#define BBLACK  "40m"
#define BRED    "41m"
#define BGREEN  "42m"
#define BYELLOW "43m"
#define BBLUE   "44m"
#define BPURPLE "45m"
#define D_BGREEN "46m"
#define BWHITE  "47m"


//Call this from macro function Log
//Log is math. Rename to Util_Info?
static inline void Util_Log
(
   int const Uniqid, 
   char const * File_Name, 
   int const Line,
   char const * Format,
   ...
)
{
	va_list Ap;
	va_start (Ap, Format);
	fprintf (stderr, "" KYEL "%04d. " KNRM "" KWHT "%s:%d" KNRM ": " KMAG "runtime log" KNRM ". ", Uniqid, File_Name, Line);
	vfprintf (stderr, Format, Ap);
	fprintf (stderr, "\n");
	va_end (Ap);
}


//Call this from macro function Assert_C or Assert
static inline void Util_Assert 
(
   int const Uniqid, 
   int const Code,
   char const * Code_String, 
   char const * Assertion,
   char const * Function_Name, 
   char const * File_Name, 
   int const Line,
   char const * Format,
   ...
)
{
	va_list Ap;
	va_start (Ap, Format);
	fprintf (stderr, "Uniq Id           : %i\n", Uniqid);
	fprintf (stderr, "Code              : %i = %s\n", Code, Code_String);
	fprintf (stderr, "Assertion         : " ANSIC (1, ANSIC_Red, ANSIC_Black) "%s" ANSIC_Default "\n", Assertion);
	fprintf (stderr, "Function_Name     : %s\n", Function_Name);
	fprintf (stderr, "File_Name         : %s\n", File_Name);
	fprintf (stderr, "Line              : %i\n", Line);
	fprintf (stderr, "errno             : %i = ", errno);
	if (errno == 0)
	{
		fprintf (stderr, ANSIC (1, ANSIC_Green, ANSIC_Black) "%s" ANSIC_Default "\n", strerror (errno));
	}
	else
	{
		fprintf (stderr, ANSIC (1, ANSIC_Red, ANSIC_Black) "%s" ANSIC_Default "\n", strerror (errno));
	}
	
	fprintf (stderr, "message           :\n");
	vfprintf (stderr, Format, Ap);
	fprintf (stderr, "\n\n");
	abort ();
	va_end (Ap);
}


//Not important. This is just a placeholder.
#define Util_Default_Assertion_Code 0


//Use this if assertion has a code attached to it.
#define Assert_C(A, Code, Message, ...) \
if (!(A)) {Util_Assert(__COUNTER__, Code, #Code, #A, __func__, __FILE__, __LINE__, Message, __VA_ARGS__); }


//Use this for normal assertion.
#define Assert(A, Message, ...) \
if (!(A)) {Assert_C (A, Util_Default_Assertion_Code, Message, __VA_ARGS__)}


//Use this to show important information.
#define Log(Message, ...) \
Util_Log(__COUNTER__, __FILE__, __LINE__, Message, __VA_ARGS__)
