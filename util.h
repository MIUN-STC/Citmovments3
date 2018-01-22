#pragma once

#include <stdlib.h>

#define KWHT "\x1B[1;37;40m"
#define KCYA "\x1B[1;35;40m"
#define KNRM "\x1B[0m"

#define Log_Error(A, M, ...) fprintf (stderr, KWHT"%s:%d"KNRM": "KCYA"runtime assertion error"KNRM" '"#A"' not true!. [errno: %d, %s]. " M "\n", __FILE__, __LINE__, errno, strerror (errno), ##__VA_ARGS__);
#define Log_Assert(A, M, ...) if (!(A)) {Log_Error (A, M, ##__VA_ARGS__); abort ();}
#define Log(M, ...) fprintf (stderr, KWHT"%s:%d"KNRM": "KCYA"runtime log"KNRM". " M "\n", __FILE__, __LINE__, ##__VA_ARGS__);
