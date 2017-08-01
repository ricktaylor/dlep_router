/*

Copyright (c) 2017 Airbus DS Limited

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <net/if.h>

#include "./dlep_iana.h"

/* Defined in discovery.c */
int discover(int use_ipv6, const char* iface, struct sockaddr_storage* modem_address, socklen_t* modem_address_length);

/* Defined in session.c */
int session(const struct sockaddr* modem_address, socklen_t modem_address_length, uint32_t router_heartbeat_interval);

static void help()
{
    printf(
	"dlep_router - A logging DLEP router\n"
        "  Version 0.1.2\n"
        "  Copyright (c) 2017 Airbus DS Limited\n\n"

        "Usage: dlep_router [options] [modem IP address [port]]\n"
        "Options:\n"
        "  -6 or --ipv6          Use IPv6 (default is IPv4)\n"
        "  -I or --interface <I> Bind the discovery to interface I, requires root\n"
        "  -H or --heartbeat <N> Use Heartbeat Interval N (default is 0)\n"
        "  -h or --help          Show this text\n");
}

int main(int argc, char* argv[])
{
	const struct option options[] =
	{
		{ "heartbeat",1,NULL,'H' },
		{ "interface",1,NULL,'I' },
		{ "help",0,NULL,'h' },
		{ "ipv6",0,NULL,'6' },
		{ 0 }
	};

	int c;
	int longindex = -1;
	int use_ipv6 = 0;
	struct sockaddr_storage address = {0};
	socklen_t address_length = 0;
	uint32_t router_heartbeat_interval = DEFAULT_HEARTBEAT_INTERVAL;
	const char* iface = NULL;

	/* Disable getopt's error messages */
	opterr = 0;

	/* Parse command line arguments */
	while ((c = getopt_long(argc, argv, ":h6H:I:", options, &longindex)) != -1)
	{
		switch (c)
		{
		case 'I':
			iface = optarg;
			break;

		case '6':
			use_ipv6 = 1;
			break;

		case 'H':
			router_heartbeat_interval = strtoul(optarg,NULL,10);
			router_heartbeat_interval *= 1000;
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
		 * This is section 7 in RFC 8175, jumping to 7.2 */

		char address_buffer[16] = {0};
		in_port_t port = DLEP_WELL_KNOWN_PORT;

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
		if (use_ipv6 || inet_pton(AF_INET6,argv[optind],&address_buffer))
		{
			((struct sockaddr_in6*)&address)->sin6_family = AF_INET6;
			((struct sockaddr_in6*)&address)->sin6_port = htons(port);
			address_length = sizeof(struct sockaddr_in6);
			memcpy(&((struct sockaddr_in6*)&address)->sin6_addr,address_buffer,16);

			/* IPv6 Link-local requires and interface index */
			if (IN6_IS_ADDR_MULTICAST(&((struct sockaddr_in6*)&address)->sin6_addr.s6_addr))
			{
				if (!iface)
				{
					printf("Interface name required with IPv6 link-local modem address, use -I\n");
					return EXIT_FAILURE;
				}

				((struct sockaddr_in6*)&address)->sin6_scope_id = if_nametoindex(iface);
			}

			use_ipv6 = 1;
		}

		if (!use_ipv6 || inet_pton(AF_INET,argv[optind],&address_buffer))
		{
			((struct sockaddr_in*)&address)->sin_family = AF_INET;
			((struct sockaddr_in*)&address)->sin_port = htons(port);
			address_length = sizeof(struct sockaddr_in);
			memcpy(&((struct sockaddr_in*)&address)->sin_addr,address_buffer,4);

			use_ipv6 = 0;
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
	        "  Version 0.1.2\n"
	        "  Copyright (c) 2017 Airbus DS Limited\n\n");

	/* Loop forever */
	for (;;)
	{
		if (optind == argc)
		{
			/* If no address was supplied on the command line, perform discovery
			 * This is section 7.1 in RFC 8175 */

			if (!discover(use_ipv6,iface,&address,&address_length))
				return EXIT_FAILURE;
		}

		if (session((const struct sockaddr*)&address,address_length,router_heartbeat_interval) != 0)
			return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
