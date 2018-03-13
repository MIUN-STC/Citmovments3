#include "util.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/epoll.h>

int main (int argc, char * argv [])
{ 
	Log ("%s () function entered\n", __func__);
	
	//No argument is used currently.
	Assert (argc == 1, "argc = %i", argc);
	Assert (argv != NULL, "argv = %p", argv);
	
	fprintf (stderr, ANSIC (ANSIC_Bold, ANSIC_White, ANSIC_Black) "Hello" ANSIC_Default "\n");

	
	int A;
	//int B;
	//int C;
	
	A = open ("A", O_RDONLY);
	printf ("open A: %d\n", A);
	//B = open ("B", O_RDONLY);
	//C = open ("C", O_RDONLY);
	printf ("ABC\n");
	
	/*
	while (1)
	{
		usleep (100000);
		char Buffer [10];
		int R;
		R = read (A, Buffer, 10);
		if (R > 0)
		{
			printf ("A: %s\n", Buffer);
		}
		R = read (B, Buffer, 10);
		if (R > 0)
		{
			printf ("B: %s\n", Buffer);
		}
		R = read (C, Buffer, 10);
		if (R > 0)
		{
			printf ("C: %s\n", Buffer);
		}
		printf (".");
		fflush (stdout);
	}
	*/
	
	int F;
	F = epoll_create1 (0);
	Assert (F != -1, "epoll_create1 failed%s", "");
	
	struct epoll_event Event;
	Event.data.fd = A;
	Event.events = EPOLLIN | EPOLLET;
	Assert (epoll_ctl (F, EPOLL_CTL_ADD, A, &Event) != -1, "epoll_ctl (%d) failed", F);
	
	
	size_t const List_Size = 4;
	struct epoll_event List [List_Size];
	
	while (1)
	{
		int N;
		N = epoll_wait (F, List, List_Size, -1);
		for (int I = 0; I < N; I = I + 1)
		{
			printf ("%d : ", List [I].data.fd);
			char Buffer [10];
			int R;
			R = read (List [I].data.fd, Buffer, 10);
			if (R > 0)
			{
				printf ("%s", Buffer);
			}
			printf ("\n");
		}
	}
	
	
	return 0;
}
