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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "./dlep_iana.h"

/* Defined in discovery.c */
int discover(/* [in] */ int use_ipv6, /* [out] */ struct sockaddr_storage* modem_address, /* [out] */ socklen_t* modem_address_length, /* [out] */ uint16_t* heartbeat_interval);

/* Defined in session.c */
int session(/* [in] */ const struct sockaddr* modem_address, /* [int] */ socklen_t modem_address_length, /* [int] */ uint16_t modem_heartbeat_interval, /* [int] */ uint16_t router_heartbeat_interval);

static void help()
{
    printf(
		"dlep_router - A logging DLEP router\n"
        "  Version 0.1.0\n"
        "  Copyright (c) 2014 Airbus DS Limited\n\n"

        "Usage: dlep_router [options] [modem IP address [port]]\n"
        "Options:\n"
    	"  -6 or --ipv6          Use IPv6 (default is IPv4)\n"
    	"  -H or --heartbeat <N> Use Heartbeat Interval N (default is 0)\n"
		"  -h or --help          Show this text\n");
}

int main(int argc, char* argv[])
{
	const struct option options[] =
	{
		{ "heartbeat",0,NULL,'H' },
		{ "help",0,NULL,'h' },
		{ "ipv6",0,NULL,'6' },
		{ 0 }
	};

	int c;
	int longindex = -1;
	int use_ipv6 = 0;
	struct sockaddr_storage address = {0};
	socklen_t address_length = 0;
	uint16_t router_heartbeat_interval = 0;

	/* Disable getopt's error messages */
	opterr = 0;

	/* Parse command line arguments */
	while ((c = getopt_long(argc, argv, ":h6H:", options, &longindex)) != -1)
	{
		switch (c)
		{
		case '6':
			use_ipv6 = 1;
			break;

		case 'H':
			router_heartbeat_interval = strtoul(optarg,NULL,10);
			break;

		case 'h':
			help();
			return EXIT_SUCCESS;

		case ':':
			if (longindex >= 0 && options[longindex].name)
				printf("Missing argument to %s\n", options[longindex].name);
			else
				printf("Missing argument to -%c\n", optopt);
			help();
			return EXIT_FAILURE;

		case '?':
		default:
			printf("Unknown option '-%c'\n", optopt);
			help();
			return EXIT_FAILURE;
		}
	}

	if (argc > optind + 2)
	{
		printf("Too many arguments\n");
		help();
		return EXIT_FAILURE;
	}

	if (optind < argc)
	{
		/* The modem address is on the command line
		 * This is section 7.1 in the draft */

		char address_buffer[16] = {0};
		in_port_t port = DLEP_WELL_KNOWN_MULTICAST_PORT;

		if (optind + 1 < argc)
		{
			/* The port is supplied */
			port = strtoul(argv[optind+1],NULL,10);
			if (errno)
			{
				printf("Failed to parse modem port number %s\n",argv[optind+1]);
				return EXIT_FAILURE;
			}
		}

		/* Try to parse */
		if (!use_ipv6 && inet_pton(AF_INET,argv[optind],&address))
		{
			((struct sockaddr_in*)&address)->sin_family = AF_INET;
			((struct sockaddr_in*)&address)->sin_port = htons(port);
			memcpy(&((struct sockaddr_in*)&address)->sin_addr,address_buffer,4);
		}
		else if (use_ipv6 && inet_pton(AF_INET6,argv[optind],&address))
		{
			((struct sockaddr_in6*)&address)->sin6_family = AF_INET6;
			((struct sockaddr_in6*)&address)->sin6_port = htons(port);

			memcpy(&((struct sockaddr_in6*)&address)->sin6_addr,address_buffer,16);
		}
		else
		{
			printf("Modem address %s not recognised\n",argv[optind]);
			return EXIT_FAILURE;
		}
	}

	/* Seed the prng */
	srand(time(NULL) ^ getpid());

	printf(	"dlep_router - A logging DLEP router\n"
	        "  Version 0.1.0\n"
	        "  Copyright (c) 2014 Airbus DS Limited\n\n");

	/* Loop forever */
	for (;;)
	{
		uint16_t modem_heartbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;

		if (optind == argc)
		{
			/* If no address was supplied on the command line, perform discovery
			 * This is section 7.2 in the draft */
			if (!discover(use_ipv6,&address,&address_length,&modem_heartbeat_interval))
				return EXIT_FAILURE;
		}

		if (session((const struct sockaddr*)&address,address_length,modem_heartbeat_interval,router_heartbeat_interval) != 0)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
