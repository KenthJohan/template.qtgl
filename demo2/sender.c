#include "csc/csc_crossos.h"
#include "csc/csc_basic.h"
#include "csc/csc_debug.h"
#include "csc/csc_math.h"
#include "csc/experiment/netgl.h"

#include <SDL2/SDL_net.h>
#include <stdio.h>




int main (int argc, char * argv[])
{
	csc_crossos_enable_ansi_color ();
	ASSERT (argc);
	ASSERT (argv);

	if (SDLNet_Init() == -1)
	{
		printf("SDLNet_Init: %s\n", SDLNet_GetError());
		exit(2);
	}


	IPaddress ip;
	TCPsocket client;

	if (SDLNet_ResolveHost (&ip, "localhost", 9999) == -1)
	{
		printf("SDLNet_ResolveHost: %s\n", SDLNet_GetError());
		exit(1);
	}

	client = SDLNet_TCP_Open (&ip);

	if (!client)
	{
		printf ("SDLNet_TCP_Open: %s\n", SDLNet_GetError());
		exit (2);
	}


	int len, result;
	char *msg = "Hello!";
	len = strlen (msg) + 1;
	result = SDLNet_TCP_Send (client, msg, len);
	if (result < len)
	{
		printf("SDLNet_TCP_Send: %s\n", SDLNet_GetError());
		// It may be good to disconnect sock because it is likely invalid now.
	}

	SDLNet_TCP_Close (client);

	SDLNet_Quit();
	return 0;
}
