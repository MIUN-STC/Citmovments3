#include "util.h"

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/signalfd.h>
#include <signal.h>
#include <netinet/in.h>


void Read_And_Print (int FD)
{
	size_t const Buffer_Size = 100;
	char Buffer [Buffer_Size];
	ssize_t R = read (FD, &Buffer, Buffer_Size - 1);
	Assert (R != -1, "Read (%d) error", FD);
	Buffer [R] = '\0';
	printf ("%s", Buffer);
}


void Handle_Incomming_Client (int Listener, int Eventpoll)
{
	struct sockaddr Address;
	socklen_t Address_Length;
	int Client = accept (Listener, &Address, &Address_Length);
	Assert (Client != -1, "accept (%d) error", Listener);
	{
		struct epoll_event Event;
		Event.data.fd = Client;
		Event.events = EPOLLIN | EPOLLET;
		int R = epoll_ctl (Eventpoll, EPOLL_CTL_ADD, Client, &Event);
		Assert (R != -1, "epoll_ctl (%d) error", Eventpoll);
	}
}


void Handle_Incomming_Signal (int FD)
{
	struct signalfd_siginfo Info;
	ssize_t R = read (FD, &Info, sizeof (Info));
	Assert (R != -1, "Read (%d) error", FD);
	printf ("Signal: %d\n", Info.ssi_signo);
}


int main (int argc, char * argv [])
{ 
	int Socket = socket (AF_INET, SOCK_STREAM, 0);
	Assert (Socket != -1, "socket error%s", "");
	
	int Eventpoll = epoll_create1 (0);
	Assert (Eventpoll != -1, "epoll_create error%s", "");
	
	int Signalfd;
	
	{
		struct sockaddr_in Address;
		Address.sin_family = AF_INET;
		Address.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
		Address.sin_port = htons (8000);
		int R = bind (Socket, (struct sockaddr *) &Address, sizeof (Address));
		Assert (R == 0, "Bind (%d) error", Socket);
	}
	
	{
		int R = listen (Socket, SOMAXCONN);
		Assert (R == 0, "Listen (%d) error", Socket);
	}
	
	{
		struct epoll_event Event;
		Event.data.fd = Socket;
		Event.events = EPOLLIN | EPOLLET;
		int R = epoll_ctl (Eventpoll, EPOLL_CTL_ADD, Socket, &Event);
		Assert (R != -1, "epoll_ctl (%d) error", Eventpoll);
	}
	
	{
		sigset_t Sigset;
		int R;
		R = sigemptyset (&Sigset);
		Assert (R == 0, "sigemptyset error%s", "");
		R = sigaddset (&Sigset, SIGQUIT);
		Assert (R == 0, "sigaddset error%s", "");
		R = sigprocmask (SIG_BLOCK, &Sigset, NULL);
		Assert (R == 0, "sigprocmask error%s", "");
		Signalfd = signalfd (-1, &Sigset, 0);
		printf ("Signalfd: %d\n", Signalfd);
		Assert (Signalfd != -1, "sigaddset error%s", "");
		struct epoll_event Event;
		Event.data.fd = Signalfd;
		Event.events = EPOLLIN | EPOLLET;
		R = epoll_ctl (Eventpoll, EPOLL_CTL_ADD, Signalfd, &Event);
		Assert (R != -1, "epoll_ctl (%d) error", Eventpoll);
	}
	
	while (true)
	{
		size_t const Event_List_Size = 100;
		struct epoll_event Event_List [Event_List_Size];
		int N = epoll_wait (Eventpoll, Event_List, Event_List_Size, -1);
		Assert (N != -1, "epoll_wait (%d) error", Eventpoll);
		for (int I = 0; I < N; I = I + 1)
		{
			printf ("#%03d : %03d : ", I, Event_List [I].data.fd);
			if (Event_List [I].data.fd == Signalfd)
			{
				printf ("Signal\n");
				Handle_Incomming_Signal (Event_List [I].data.fd);
			}
			else if (Event_List [I].data.fd == Socket)
			{
				printf ("New client\n");
				Handle_Incomming_Client (Socket, Eventpoll);
			}
			else
			{
				printf ("Client message : ");
				Read_And_Print (Event_List [I].data.fd);
				printf ("\n");
				close (Event_List [I].data.fd);
			}
			
		}
	}
	
	
	return 0;
}
