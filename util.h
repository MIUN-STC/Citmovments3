#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


//Stringyfier for c preproccessor
#define ANSIC_Str1(X) #X
#define ANSIC_Str(X) ANSIC_Str1 (X)


//ANSI text color escape string formatter
#define ANSIC(Text_Attribute, Foreground_Color, Background_Color) \
"\x1B[" ANSIC_Str (Text_Attribute) ";3" ANSIC_Str (Foreground_Color) ";4" ANSIC_Str (Background_Color) "m"


//Used for text attributes
#define ANSIC_Normal       0
#define ANSIC_Bold         1
#define ANSIC_Underscore   4
#define ANSIC_Blink        5
#define ANSIC_Reverse      7
#define ANSIC_Conceal      8


//Used for foreground and background color
#define ANSIC_Black   0
#define ANSIC_Red     1
#define ANSIC_Green   2
#define ANSIC_Yellow  3
#define ANSIC_Blue    4
#define ANSIC_Magenta 5
#define ANSIC_Cyan    6
#define ANSIC_White   7


//Default text color used in terminals
#define ANSIC_Default ANSIC (ANSIC_Normal, ANSIC_White, ANSIC_Black)


//Call this from macro function Log ()
//Log () is math. Rename to Info ()?
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
	fprintf (stderr, "" ANSIC (ANSIC_Bold  , ANSIC_Yellow , ANSIC_Black) "%04d. "      ANSIC_Default ""  , Uniqid);
	fprintf (stderr, "" ANSIC (ANSIC_Bold  , ANSIC_White  , ANSIC_Black) "%s:%04d"       ANSIC_Default ": ", File_Name, Line);
	fprintf (stderr, "" ANSIC (ANSIC_Normal, ANSIC_Magenta, ANSIC_Black) "runtime log: " ANSIC_Default "");
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
	fprintf (stderr, "Assertion         : " ANSIC (ANSIC_Bold, ANSIC_Red, ANSIC_Black) "%s" ANSIC_Default "\n", Assertion);
	fprintf (stderr, "Function_Name     : %s\n", Function_Name);
	fprintf (stderr, "File_Name         : %s\n", File_Name);
	fprintf (stderr, "Line              : %i\n", Line);
	fprintf (stderr, "errno             : %i = ", errno);
	if (errno == 0)
	{
		fprintf (stderr, ANSIC (ANSIC_Bold, ANSIC_Green, ANSIC_Black) "%s" ANSIC_Default "\n", strerror (errno));
	}
	else
	{
		fprintf (stderr, ANSIC (ANSIC_Bold, ANSIC_Red, ANSIC_Black) "%s" ANSIC_Default "\n", strerror (errno));
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
if (!(A)) {Util_Assert (__COUNTER__, Code, #Code, #A, __func__, __FILE__, __LINE__, Message, __VA_ARGS__); }


//Use this for normal assertion.
#define Assert(A, Message, ...) \
if (!(A)) {Assert_C (A, Util_Default_Assertion_Code, Message, __VA_ARGS__)}


//Use this to show important information.
#define Log(Message, ...) \
Util_Log (__COUNTER__, __FILE__, __LINE__, Message, __VA_ARGS__)
