/*

The MIT License (MIT)

Copyright (c) 2014 Airbus DS Limited

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>

#include "./util.h"

void session(/* [in] */ const struct sockaddr* modem_address, /* [int] */ socklen_t modem_address_length, /* [int] */ uint16_t heartbeat_interval)
{
	char str_address[PRINTFADDRESS_LEN] = {0};

	/* First we must initialise, draft section 7.2 */
	int s = socket(modem_address->sa_family,SOCK_STREAM,0);
	if (s == -1)
	{
		printf("Failed to create socket: %s\n",strerror(errno));
		return;
	}

	printf("Connecting to modem at %s\n",printfAddress(modem_address,str_address,sizeof(str_address)));

	/* Connect to the modem */
	if (connect(s,modem_address,modem_address_length) == -1)
	{
		printf("Failed to connect socket: %s\n",strerror(errno));
	}
	else
	{
		char msg[1500];
		ssize_t received;

		printf("Waiting for Peer Initialization signal from modem...\n");

		/* Receive a Peer Initialization signal */
		received = recv(s,msg,sizeof(msg),0);
		if (received == -1)
			printf("Failed to receive from TCP socket: %s\n",strerror(errno));
		else if (received == -1)
			printf("Modem disconnected TCP session\n");
		else
		{
			printf("Received possible Peer Initialization signal (%u bytes) from modem...\n",(unsigned int)received);

			/* Check it's a valid Peer Initialization signal */
			if (check_peer_init_signal(msg,received))
			{


			}
		}
	}

	close(s);
}
