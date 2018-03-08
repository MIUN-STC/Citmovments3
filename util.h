#pragma once

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#define KWHT "\x1B[1;37;40m"
#define KMAG "\x1B[1;35;40m"
#define KYEL "\x1B[1;33;40m"
#define KNRM "\x1B[0m"

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
	fprintf (stderr, "Assertion         : %s\n", Assertion);
	fprintf (stderr, "Function_Name     : %s\n", Function_Name);
	fprintf (stderr, "File_Name         : %s\n", File_Name);
	fprintf (stderr, "Line              : %i\n", Line);
	fprintf (stderr, "errno             : %i\n", errno);
	fprintf (stderr, "strerror          : %s\n", strerror (errno));
	vfprintf (stderr, Format, Ap);
	fprintf (stderr, "\n\n");
	abort ();
	va_end (Ap);
}


#define Util_Assertion 0

#define Assert_C(A, Code, Message, ...) \
if (!(A)) {Util_Assert(__COUNTER__, Code, #Code, #A, __func__, __FILE__, __LINE__, Message, __VA_ARGS__); }

#define Assert(A, Message, ...) \
if (!(A)) {Assert_C (A, Util_Assertion, Message, __VA_ARGS__)}

#define Log(Message, ...) \
Util_Log(__COUNTER__, __FILE__, __LINE__, Message, __VA_ARGS__)
