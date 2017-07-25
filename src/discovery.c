/*

Copyright (c) 2017 Airbus DS Limited

*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <net/if.h>

#include "./dlep_iana.h"
#include "./check.h"
#include "./util.h"

static int send_peer_discovery_signal(int s, const struct sockaddr* address, socklen_t address_len)
{
	char str_address[FORMATADDRESS_LEN] = {0};
	uint8_t msg[100];
	uint8_t* p;
	uint16_t msg_len = 0;
	size_t peer_type_len = 0;
	uint8_t flags = 0x00;

	/* All DLEP signals start with the 4 characters 'DLEP' */
	msg[0] = 'D';
	msg[1] = 'L';
	msg[2] = 'E';
	msg[3] = 'P';

	/* Octets 4 and 5 are the signal number */
	p = write_uint16(DLEP_PEER_DISCOVERY,msg+4);

	/* Octets 6 and 7 are the signal length, initialize to 0 */
	p = write_uint16(0,p);

	/* Write out Peer Type, with safety check! */
	peer_type_len = strlen(PEER_TYPE);
	if (peer_type_len > sizeof(msg) - 13)
		peer_type_len = sizeof(msg) - 13;

	p = write_uint16(DLEP_PEER_TYPE_DATA_ITEM,p);
	p = write_uint16(1 + peer_type_len,p); /* includes length of the flag + description fields */

	/* add flags field */
	*p++ = flags;

	if (peer_type_len)
	{
		memcpy(p,PEER_TYPE,peer_type_len);
		p += peer_type_len;
	}

	msg_len = p - msg;

	/* Octet 6 and 7 are the signal length, minus the length of the header */
	write_uint16(msg_len - 8,msg + 6);

	printf("Sending Peer Discovery signal to %s\n",formatAddress(address,str_address,sizeof(str_address)));

	if (sendto(s,msg,msg_len,0,address,address_len) != msg_len)
	{
		printf("Failed to send Peer Discovery signal: %s\n",strerror(errno));
		return 0;
	}

	return 1;
}

static ssize_t recv_peer_offer(int s, unsigned int secs, uint8_t msg[1500])
{
	struct timeval timeout = {0};
	fd_set readfds;
	ssize_t received;
	struct sockaddr_storage recv_address = {0};
	socklen_t recv_address_len = sizeof(recv_address);
	char str_address[FORMATADDRESS_LEN] = {0};

	FD_ZERO(&readfds);
	FD_SET(s,&readfds);

	printf("Waiting for Peer Offer signal\n");

	/* Use select() to wait for secs seconds */
	timeout.tv_sec = secs;
	if (select(s+1,&readfds,NULL,NULL,&timeout) == -1)
	{
		printf("Failed to wait for peer discovery signal: %s\n",strerror(errno));
		return -1;
	}
	if (!FD_ISSET(s,&readfds))
		return 0;

	/* Receive the signal */
	received = recvfrom(s,msg,1500,0,(struct sockaddr*)&recv_address,&recv_address_len);
	if (received == -1)
	{
		printf("Failed to receive from UDP socket: %s\n",strerror(errno));
		return -1;
	}

	printf("Received possible Peer Offer signal (%u bytes) from %s\n",(unsigned int)received,formatAddress((struct sockaddr*)&recv_address,str_address,sizeof(str_address)));

	return received;
}

static int get_peer_offer(int s, const struct sockaddr* dest_addr, socklen_t dest_addr_len, struct sockaddr_storage* modem_address, socklen_t* modem_address_length)
{
	uint8_t msg[1500];
	ssize_t len = 0;
	const uint8_t* data_item;
	char peer_address[INET6_ADDRSTRLEN] = {0};
	uint16_t port = 0;

	/* Loop until we have a Peer Offer signal or an error */
	while (len == 0)
	{
		/* Send the message to the well-known multicast address */
		if (!send_peer_discovery_signal(s,dest_addr,dest_addr_len))
			return 0;

		/* Now wait for a response */
		len = recv_peer_offer(s,DEFAULT_DISCOVERY_RETRY,msg);
		if (len == -1)
			return 0;

		/* Validate the signal */
		if (len && check_peer_offer_signal(msg,len))
			len = 0;
	}

	printf("Valid Peer Offer signal from modem\n");

	/* The signal has been validated so just scan for the relevant data_items */
	modem_address->ss_family = 0;

	data_item = msg + 8;
	while (data_item < msg + len)
	{
		/* Octets 0 and 1 are the data item type */
		enum dlep_data_item item_id = read_uint16(data_item);

		/* Octets 2 and 3 are the data item length */
		uint16_t item_len = read_uint16(data_item + 2);

		/* Increment data_item to point to the data */
		data_item += 4;

		switch (item_id)
		{
		case DLEP_PEER_TYPE_DATA_ITEM:
			printf("  Peer Type: '%.*s'%s\n",(int)item_len-1,data_item + 1,data_item[0] ? " - Secured Medium" : "");
			break;

		case DLEP_IPV4_CONN_POINT_DATA_ITEM:
			modem_address->ss_family = AF_INET;
			memcpy(&((struct sockaddr_in*)modem_address)->sin_addr,data_item + 1,4);
			*modem_address_length = sizeof(struct sockaddr_in);
			printf("  IPv4 address: (TLS %s) %s\n",(data_item[0] ? "Required" : "optional"),inet_ntop(AF_INET,data_item + 1,peer_address,sizeof(peer_address)));
			if (item_len == 7)
				port = read_uint16(data_item + 5);
			else
				port = DLEP_WELL_KNOWN_PORT;
			((struct sockaddr_in*)modem_address)->sin_port = htons(port);
			break;

		case DLEP_IPV6_CONN_POINT_DATA_ITEM:
			modem_address->ss_family = AF_INET6;
			memcpy(&((struct sockaddr_in6*)modem_address)->sin6_addr,data_item + 1,16);
			*modem_address_length = sizeof(struct sockaddr_in6);
			printf("  IPv6 address: (TLS %s) %s\n",(data_item[0] ? "Required" : "optional"),inet_ntop(AF_INET6,data_item + 1,peer_address,sizeof(peer_address)));
			if (item_len == 19)
				port = read_uint16(data_item + 17);
			else
				port = DLEP_WELL_KNOWN_PORT;
			((struct sockaddr_in*)modem_address)->sin_port = htons(port);
			break;

		default:
			/* Others will be caught by the check function */
			break;
		}

		/* Increment data_item to point to the next data item */
		data_item += item_len;
	}

	if (!modem_address->ss_family)
	{
		/* If we did not find an address with a compatible family, report */
		printf("Failed to find an IP address in Peer Offer signal\n");
		return 0;
	}

	if (!port)
	{
		/* If we did not find a port, report */
		printf("Failed to find an port in Peer Offer signal\n");
		return 0;
	}

	/* Set the port on the selected modem address */
	if (modem_address->ss_family == AF_INET)
		((struct sockaddr_in*)modem_address)->sin_port = htons(port);
	else
		((struct sockaddr_in6*)modem_address)->sin6_port = htons(port);

	return 1;
}

static int discover_ipv4(int s, struct sockaddr_storage* modem_address, socklen_t* modem_address_length)
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
			discovery_address.sin_port = htons(DLEP_WELL_KNOWN_PORT);
			inet_pton(AF_INET,DLEP_WELL_KNOWN_MULTICAST_ADDRESS,&discovery_address.sin_addr);

			/* Do the discovery */
			if (get_peer_offer(s,(struct sockaddr*)&discovery_address,sizeof(discovery_address),modem_address,modem_address_length))
				ret = 1;
		}
	}

	return ret;
}

static int discover_ipv6(int s, const char* iface, struct sockaddr_storage* modem_address, socklen_t* modem_address_length)
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
			discovery_address.sin6_port = htons(DLEP_WELL_KNOWN_PORT);
			inet_pton(AF_INET6,DLEP_WELL_KNOWN_MULTICAST_ADDRESS_6,&discovery_address.sin6_addr);
			if (iface)
				discovery_address.sin6_scope_id = if_nametoindex(iface);

			/* Do the discovery */
			if (get_peer_offer(s,(struct sockaddr*)&discovery_address,sizeof(discovery_address),modem_address,modem_address_length))
			{
				((struct sockaddr_in6*)modem_address)->sin6_scope_id = discovery_address.sin6_scope_id;
				ret = 1;
			}
		}
	}

	return ret;
}

int discover(int use_ipv6, const char* iface, struct sockaddr_storage* modem_address, socklen_t* modem_address_length)
{
	int ret = 0;

	/* Create a UDP socket */
	int s = socket(use_ipv6 ? AF_INET6 : AF_INET,SOCK_DGRAM,0);
	if (s == -1)
	{
		printf("Failed to create socket: %s\n",strerror(errno));
	}
	else
	{
		ret = 1;

		if (iface)
		{
			/* Bind the socket to the specified interface */
			if (geteuid() != 0)
				printf("Not binding multicast discovery socket to interface %s as not root\n",iface);
			else if (setsockopt(s,SOL_SOCKET,SO_BINDTODEVICE, iface, strlen(iface)+1) != 0)
			{
				printf("Failed to bind socket to interface %s: %s\n",iface,strerror(errno));
				ret = 0;
			}
		}

		if (ret)
		{
			if (use_ipv6)
				ret = discover_ipv6(s,iface,modem_address,modem_address_length);
			else
				ret = discover_ipv4(s,modem_address,modem_address_length);
		}

		close(s);
	}

	return ret;
}
