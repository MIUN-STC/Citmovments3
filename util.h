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


/*
#define Log_Error(A, M, ...) fprintf (stderr, KWHT"%s:%d"KNRM": "KCYA"runtime assertion error"KNRM" '"#A"' not true!. [errno: %d, %s]. " M "\n", __FILE__, __LINE__, errno, strerror (errno), ##__VA_ARGS__);
#define Log_Assert(A, M, ...) if (!(A)) {Log_Error (A, M, ##__VA_ARGS__); abort ();}
#define Log(M, ...) fprintf (stderr, KWHT"%s:%d"KNRM": "KCYA"runtime log"KNRM". " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);
*/

enum Util_Importance
{
   Util_Ignore = 0,
   Util_Info = 1,
   Util_Abort = 2
};

enum Util_Code
{
   Util_Normal = __COUNTER__
};

static void Util_Logger 
(
   int const Uniqid, 
   int const Code,
   char const * Code_String, 
   int const Importance,
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
   switch (Importance)
   {
      case Util_Ignore:
         break;
      
      case Util_Info:
         fprintf (stderr, "" KYEL "%04d. " KNRM "" KWHT "%s:%d" KNRM ": " KMAG "runtime log" KNRM ". ", Uniqid, File_Name, Line);
         vfprintf (stderr, Format, Ap);
         fprintf (stderr, "\n");
         break;
      
      case Util_Abort:
         fprintf (stderr, "Uniq Id           : %i\n", Uniqid);
         fprintf (stderr, "Code              : %i = %s\n", Code, Code_String);
         fprintf (stderr, "Importance        : %i\n", Importance);
         fprintf (stderr, "Assertion         : %s\n", Assertion);
         fprintf (stderr, "Function_Name     : %s\n", Function_Name);
         fprintf (stderr, "File_Name         : %s\n", File_Name);
         fprintf (stderr, "Line              : %i\n", Line);
         fprintf (stderr, "errno             : %i\n", errno);
         fprintf (stderr, "strerror          : %s\n", strerror (errno));
         vfprintf (stderr, Format, Ap);
         fprintf (stderr, "\n\n");
         abort ();
         break;
      
      default:
         break;
   };
   va_end (Ap);
}

#define Assert3arg(A, Code, Message, ...) \
if (!(A)) {Util_Logger(__COUNTER__, Code, #Code, Util_Abort, #A, __func__, __FILE__, __LINE__, Message, __VA_ARGS__); }

#define Assert(A, Message, ...) \
if (!(A)) {Util_Logger(__COUNTER__, Util_Normal, "Util_Normal", Util_Abort, #A, __func__, __FILE__, __LINE__, Message, __VA_ARGS__); }

#define Log(Message, ...) \
Util_Logger(__COUNTER__, Util_Normal, "Util_Normal", Util_Info, "", __func__, __FILE__, __LINE__, Message, __VA_ARGS__)



