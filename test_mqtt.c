#include <mosquitto.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>
#include <string.h>


static bool Should_Run = true;

void Handle_Signal (int S)
{
	printf ("Handle_Signal %i\n", S);
	Should_Run = false;
}

void Connect_Callback (struct mosquitto * Mosq, void * Object, int Result)
{
	assert (Mosq != NULL);
	assert (Object != NULL);
	printf ("Connect_Callback. Result : %i\n", Result);
}

void Message_Callback (struct mosquitto * Mosq, void * Object, struct mosquitto_message const * Message)
{
	assert (Mosq != NULL);
	assert (Object != NULL);
	printf ("Message_Callback\n");
	printf ("%.*s", Message->payloadlen, (char *) Message->payload);
}


void Delegate (int From, struct mosquitto * Mosq, char const * Topic)
{
	size_t const Buffer_Size = 10;
	char Buffer [Buffer_Size];
    int R;
    R = read (From, Buffer, Buffer_Size);
    if (R < 0)
    {fprintf (stderr, "Read error.  %s:%d\n", __FILE__, __LINE__);}
    R = mosquitto_publish (Mosq, NULL, Topic, R, Buffer, 0, 0);
    if (R != MOSQ_ERR_SUCCESS)
    {fprintf (stderr, "mosquitto_publish %d.  %s:%d\n", R, __FILE__, __LINE__);}
}


int main (int argc, char * argv [argc])
{
	assert (argc > 0);
	for (size_t I = 0; I < (size_t) argc; I = I + 1)
	{
		printf ("%02i : %s\n", I, argv [I]);
	}
	
	signal (SIGINT, Handle_Signal);
	signal (SIGTERM, Handle_Signal);
	
	char Client_ID [24];
	memset (Client_ID, 0, 24);
	snprintf (Client_ID, 23, "Client_%d", getpid ());
	struct mosquitto * Mosq;
	mosquitto_lib_init ();
	Mosq = mosquitto_new (Client_ID, true, NULL);
	if (Mosq == NULL)
	{
		fprintf (stderr, "mosquitto_new fail\n");
		fflush (stderr);
		return 0;
	}
	
	mosquitto_connect_callback_set (Mosq, Connect_Callback);
	mosquitto_message_callback_set (Mosq, Message_Callback);
	
	int RC;
	mosquitto_username_pw_set (Mosq, argv [0], argv [1]);
	mosquitto_connect (Mosq, argv [2], argv [3], 60);
	mosquitto_subscribe (Mosq, NULL, "my_topic", 0);
	
	
	while (true)
	{
		if (!Should_Run) {break;};
		RC = mosquitto_loop (Mosq, -1, 1); 
		if (!Should_Run) {break;};
		if (RC)
		{
			printf ("Mosquitto connection error. Reconnecting in 10 seconds.\n");
			sleep (10);
			mosquitto_reconnect (Mosq);
		};
		Delegate (STDIN_FILENO, Mosq, "test");
	}
	
	
	return 1;
}
