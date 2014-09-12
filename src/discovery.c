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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "./dlep_iana.h"
#include "./check.h"
#include "./util.h"

static int send_peer_discovery_signal(int s, const struct sockaddr* address, socklen_t address_len)
{
	char str_address[PRINTFADDRESS_LEN] = {0};
	char msg[3];
	uint16_t msg_len = 3;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_PEER_DISCOVERY;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	set_uint16(msg_len,msg+1);

	printf("Sending Peer Discovery signal to %s\n",printfAddress(address,str_address,sizeof(str_address)));

	if (sendto(s,msg,msg_len,0,address,address_len) != msg_len)
	{
		printf("Failed to send Peer Discovery signal: %s\n",strerror(errno));
		return -1;
	}

	return 0;
}

static ssize_t recv_peer_offer(int s, unsigned int secs, char msg[1500])
{
	struct timeval timeout = {0};
	fd_set readfds;
	ssize_t received;
	struct sockaddr_storage recv_address = {0};
	socklen_t recv_address_len = sizeof(recv_address);
	char str_address[PRINTFADDRESS_LEN] = {0};

	FD_ZERO(&readfds);
	FD_SET(s,&readfds);

	printf("Waiting for Peer Offer signal\n");

	/* Use select() to wait for secs seconds */
	timeout.tv_sec = secs;
	if (select(1,&readfds,NULL,NULL,&timeout) == -1)
	{
		printf("Failed to wait for peer discovery signal: %s\n",strerror(errno));
		return -1;
	}
	if (!FD_ISSET(s,&readfds))
		return 0;

	/* Receive the signal */
	received = recvfrom(s,msg,sizeof(msg),MSG_DONTWAIT,(struct sockaddr*)&recv_address,&recv_address_len);
	if (received == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return 0;

		printf("Failed to receive from UDP socket: %s\n",strerror(errno));
		return -1;
	}

	printf("Received possible Peer Offer signal (%u bytes) from %s\n",(unsigned int)received,printfAddress((struct sockaddr*)&recv_address,str_address,sizeof(str_address)));

	return received;
}

static int get_peer_offer(int s, const struct sockaddr* dest_addr, socklen_t dest_addr_len, struct sockaddr_storage* modem_address, socklen_t* modem_address_length, uint16_t* heartbeat_interval)
{
	char msg[1500];
	ssize_t len = 0;
	const char* tlv;
	char peer_type[257] = {0};
	char peer_address[PRINTFADDRESS_LEN] = {0};
	uint16_t port;

	/* Loop until we have a Peer Offer signal or an error */
	while (len == 0)
	{
		/* Send the message to the well-known multicast address */
		if (send_peer_discovery_signal(s,dest_addr,dest_addr_len) != 0)
			return 0;

		/* Now wait for a response */
		len = recv_peer_offer(s,DEFAULT_DISCOVERY_RETRY,msg);
		if (len == -1)
			return 0;

		/* Validate the signal */
		if (len && !check_peer_offer_signal(msg,len))
			len = 0;
	}

	/* The signal has been validated so just scan for the relevant tlvs */
	modem_address->ss_family = 0;
	for (tlv = msg + 3; tlv < msg + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch (tlv[0])
		{
		case DLEP_PORT_TLV:
			port = get_uint16(tlv+2);
			if (dest_addr->sa_family == AF_INET)
				((struct sockaddr_in*)&modem_address)->sin_port = htons(port);
			else
				((struct sockaddr_in6*)&modem_address)->sin6_port = htons(port);
			break;

		case DLEP_HEARTBEAT_INTERVAL_TLV:
			*heartbeat_interval = get_uint16(tlv+2);
			break;

		case DLEP_IPV4_ADDRESS_TLV:
			if (dest_addr->sa_family == AF_INET)
			{
				modem_address->ss_family = AF_INET;
				memcpy(&((struct sockaddr_in*)&modem_address)->sin_addr,tlv+3,4);
				*modem_address_length = sizeof(struct sockaddr_in);
			}
			break;

		case DLEP_IPV6_ADDRESS_TLV:
			if (dest_addr->sa_family == AF_INET6)
			{
				modem_address->ss_family = AF_INET6;
				memcpy(&((struct sockaddr_in6*)&modem_address)->sin6_addr,tlv+3,16);
				*modem_address_length = sizeof(struct sockaddr_in6);
			}
			break;

		case DLEP_PEER_TYPE_TLV:
			strncpy(peer_type,tlv+2,tlv[1]);
			break;

		case DLEP_STATUS_TLV:
			if (tlv[2] != 0)
			{
				printf("Received Peer Offer signal has status %u, not connecting\n",tlv[2]);
				return 0;
			}
			break;

		default:
			break;
		}
	}

	if (!modem_address->ss_family)
	{
		/* If we did not find an address with a compatible family, report */
		if (dest_addr->sa_family == AF_INET)
			printf("Failed to find an IPv4 address in Peer Offer signal\n");
		else
			printf("Failed to find an IPv6 address in Peer Offer signal\n");
		return 0;
	}

	printf("Received Peer Offer signal from modem ");

	if (*peer_type)
		printf("%s ",peer_type);

	printf("offering connection %s\n",printfAddress((struct sockaddr*)&modem_address,peer_address,sizeof(peer_address)));

	return 1;
}

static int discover_ipv4(int s, struct sockaddr_storage* modem_address, socklen_t* modem_address_length, uint16_t* heartbeat_interval)
{
	int ret = 0;

	/* Bind the socket to a system assigned local address */
	struct sockaddr_in local_address = {0};
	local_address.sin_family = AF_INET;
	local_address.sin_port = 0;
	local_address.sin_addr.s_addr = INADDR_ANY;

	if (bind(s,(struct sockaddr*)&local_address,sizeof(local_address)) != 0)
	{
		printf("Failed to bind socket: %s\n",strerror(errno));
	}
	else
	{
		/* Set multicast loopback socket options so we can run the router
		 * process and the modem process on the same machine */
		int on = 1;
		if (setsockopt(s,IPPROTO_IP,IP_MULTICAST_LOOP,&on,sizeof(on)) != 0)
		{
			printf("Failed to set multicast loopback option: %s\n",strerror(errno));
		}
		else
		{
			struct sockaddr_in discovery_address = {0};
			discovery_address.sin_family = AF_INET;
			discovery_address.sin_port = htons(DLEP_WELL_KNOWN_MULTICAST_PORT);
			inet_pton(AF_INET,DLEP_WELL_KNOWN_MULTICAST_ADDRESS,&discovery_address.sin_addr);

			/* Do the discovery */
			if (get_peer_offer(s,(struct sockaddr*)&discovery_address,sizeof(discovery_address),modem_address,modem_address_length,heartbeat_interval))
				ret = 1;
		}
	}

	return ret;
}

int discover_ipv6(int s, struct sockaddr_storage* modem_address, socklen_t* modem_address_length, uint16_t* heartbeat_interval)
{
	int ret = 0;

	/* Bind the socket to a system assigned local address */
	struct sockaddr_in6 local_address = {0};
	local_address.sin6_family = AF_INET6;
	local_address.sin6_port = 0;
	local_address.sin6_addr = in6addr_any;

	if (bind(s,(struct sockaddr*)&local_address,sizeof(local_address)) != 0)
	{
		printf("Failed to bind socket: %s\n",strerror(errno));
	}
	else
	{
		/* Set multicast loopback socket options so we can run the router
		 * process and the modem process on the same machine */
		int on = 1;
		if (setsockopt(s,IPPROTO_IPV6,IPV6_MULTICAST_LOOP,&on,sizeof(on)) != 0)
		{
			printf("Failed to set multicast loopback option: %s\n",strerror(errno));
		}
		else
		{
			struct sockaddr_in6 discovery_address = {0};
			discovery_address.sin6_family = AF_INET6;
			discovery_address.sin6_port = htons(DLEP_WELL_KNOWN_MULTICAST_PORT);
			inet_pton(AF_INET6,DLEP_WELL_KNOWN_MULTICAST_ADDRESS_6,&discovery_address.sin6_addr);

			/* Do the discovery */
			if (get_peer_offer(s,(struct sockaddr*)&discovery_address,sizeof(discovery_address),modem_address,modem_address_length,heartbeat_interval))
				ret = 1;
		}
	}

	return ret;
}

int discover(/* [in] */ int use_ipv6, /* [out] */ struct sockaddr_storage* modem_address, /* [out] */ socklen_t* modem_address_length, /* [out] */ uint16_t* heartbeat_interval)
{
	int ret = 0;

	/* Create a UDP socket */
	int s = socket(AF_INET,SOCK_DGRAM,0);
	if (s == -1)
	{
		printf("Failed to create socket: %s\n",strerror(errno));
	}
	else
	{
		if (use_ipv6)
			ret = discover_ipv6(s,modem_address,modem_address_length,heartbeat_interval);
		else
			ret = discover_ipv4(s,modem_address,modem_address_length,heartbeat_interval);

		close(s);
	}

	return ret;
}
