#include "util.h"

int main (int argc, char * argv [])
{ 
	Log ("%s () function entered\n", __func__);
	
	//No argument is used currently.
	Assert (argc == 1, "argc = %i", argc);
	Assert (argv != NULL, "argv = %p", argv);
	
	fprintf (stderr, ANSIC (ANSIC_Bold, ANSIC_White, ANSIC_Black) "Hello" ANSIC_Default "\n");

	
	return 0;
}
