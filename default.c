#include "util.h"

int main (int argc, char * argv [])
{ 
	//No argument is used currently.
	Assert (argc == 1, "argc = %i", argc);
	Assert (argv != NULL, "argv = %p", argv);
	
	fprintf (stderr, ANSIC (1, ANSIC_White, ANSIC_Black) "Hello" ANSIC (0, ANSIC_White, ANSIC_Black));
	fprintf (stderr, "\n");
	
	return 0;
}
