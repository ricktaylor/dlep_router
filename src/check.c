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
#include <arpa/inet.h>

#include "./dlep_iana.h"

uint16_t get_uint16(const char* p)
{
	/* Avoid ntohs() due to unaligned access issues on some architectures */
	return (p[0] << 8 | p[1]);
}

void set_uint16(uint16_t v, char* p)
{
	/* Avoid htons() due to unaligned access issues on some architectures */
	p[0] = v >> 8;
	p[1] = v & 0xFF;
}

int check_port(const char* tlv)
{
	if (tlv[1] != 2)
	{
		printf("Incorrect length in DLEP Port TLV: %u, expected 2\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

int check_peer_type(const char* tlv)
{
	size_t i = 0;
	for (; i < tlv[1]; ++i)
	{
		/* Check for NUL */
		if (tlv[i + 2] == 0)
		{
			printf("Invalid NUL character in peer type string\n");
			return 0;
		}

		/* TODO: One should check for valid UTF8 characters here */
	}

	return 1;
}

int check_heartbeat_interval(const char* tlv)
{
	if (tlv[1] != 2)
	{
		printf("Incorrect length in Heartbeat Interval TLV: %u, expected 2\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

int check_ipv4_address(const char* tlv)
{
	if (tlv[1] != 6)
	{
		printf("Incorrect length in IPv4 Address TLV: %u, expected 6\n",(unsigned int)tlv[1]);
		return 0;
	}

	if (tlv[2] != 1 && tlv[2] != 2)
	{
		printf("Incorrect add/drop indicator in IPv4 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
		return 0;
	}

	if (tlv[7] > 32)
	{
		printf("Incorrect subnet mask in IPv4 Address TLV: %u, expected 0..32\n",(unsigned int)tlv[7]);
		return 0;
	}

	return 1;
}

int check_ipv6_address(const char* tlv)
{
	if (tlv[1] != 18)
	{
		printf("Incorrect length in IPv6 Address TLV: %u, expected 18\n",(unsigned int)tlv[1]);
		return 0;
	}

	if (tlv[2] != 1 && tlv[2] != 2)
	{
		printf("Incorrect add/drop indicator in IPv6 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
		return 0;
	}

	if (tlv[19] > 128)
	{
		printf("Incorrect subnet mask in IPv6 Address TLV: %u, expected 0..128\n",(unsigned int)tlv[19]);
		return 0;
	}

	return 1;
}

int check_peer_offer_signal(const char* msg, size_t len)
{
	int ret = 1;
	const char* tlv;
	int seen_heartbeat = 0;
	int seen_ip_address = 0;
	int seen_port = 0;
	int seen_peer_type = 0;

	/* Validate the signal */
	if (len < 3)
	{
		printf("Packet too short for Peer Offer signal: %d bytes\n",(unsigned int)len);
		return 0;
	}
	else if (msg[0] != DLEP_PEER_OFFER)
	{
		printf("Peer Offer signal expected, but signal %d received\n",msg[0]);
		return 0;
	}
	else
	{
		uint16_t reported_len = get_uint16(msg+1);
		if (reported_len != len)
		{
			printf("Peer Offer signal length %u does not match received packet length %d\n",reported_len,(unsigned int)len);
			return 0;
		}
	}

	/* Check for mandatory TLV's */
	for (tlv = msg + 3; tlv < msg + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch (tlv[0])
		{
		case DLEP_PORT_TLV:
			if (seen_port)
			{
				printf("Multiple DLEP port TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_port(tlv))
				ret = 0;
			else
				seen_port = 1;
			break;

		case DLEP_HEARTBEAT_INTERVAL_TLV:
			if (seen_heartbeat)
			{
				printf("Multiple Heartbeat Interval TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_heartbeat_interval(tlv))
				ret = 0;
			else
				seen_heartbeat = 1;
			break;

		case DLEP_IPV4_ADDRESS_TLV:
			if (!check_ipv4_address(tlv))
				ret = 0;
			else if (tlv[2] != 1)
			{
				printf("IPv4 address TLV in Peer Offer signal marks address as dropped!\n");
				ret = 0;
			}
			else if (tlv[2] != 1)
			{
				printf("IPv4 address TLV in Peer Offer signal marks address as dropped!\n");
				ret = 0;
			}
			else if (tlv[7] != 32)
			{
				printf("IPv4 address TLV in Peer Offer signal has a subnet!\n");
				ret = 0;
			}
			else
				seen_ip_address = 1;
			break;

		case DLEP_IPV6_ADDRESS_TLV:
			if (!check_ipv6_address(tlv))
				ret = 0;
			else if (tlv[2] != 1)
			{
				printf("IPv6 address TLV in Peer Offer signal marks address as dropped!\n");
				ret = 0;
			}
			else if (tlv[19] != 32)
			{
				printf("IPv6 address TLV in Peer Offer signal has a subnet!\n");
				ret = 0;
			}
			else
				seen_ip_address = 1;
			break;

		case DLEP_PEER_TYPE_TLV:
			if (seen_peer_type)
			{
				printf("Multiple Peer Type TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_peer_type(tlv))
				ret = 0;
			else
				seen_peer_type = 1;
			break;

		default:
			printf("Unexpected TLV %d in Peer Offer signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_ip_address)
	{
		printf("Missing IPv4 or IPv6 address TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (!seen_port)
	{
		printf("Missing DLEP Port TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (!seen_heartbeat)
	{
		printf("Missing Heartbeat Interval TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (tlv != msg + len)
	{
		printf("Signal length does not equal sum of TLV lengths in Peer Offer signal\n");
		ret = 0;
	}

	return ret;
}
