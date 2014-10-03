/*

Copyright (c) 2014 Airbus DS Limited

*/

#include "./util.h"

#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include <inttypes.h>

#include "./dlep_iana.h"

static int send_peer_init_signal(int s, uint16_t router_heartbeat_interval)
{
	char msg[300];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_PEER_INITIALIZATION;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our version */
	tlv[0] = DLEP_VERSION_TLV;
	tlv[1] = 4;
	set_uint16(0,tlv + 2);
	set_uint16(7,tlv + 4);
	tlv += tlv[1] + 2;

	/* Write out our Heartbeat Interval */
	tlv[0] = DLEP_HEARTBEAT_INTERVAL_TLV;
	set_uint16(router_heartbeat_interval,tlv+2);
	tlv[1] = 2;
	tlv += tlv[1] + 2;

	/* Write out Optional Signals Supported */
	tlv[0] = DLEP_OPTIONAL_SIGNALS_TLV;
	tlv[2] = DLEP_PEER_UPDATE_ACK;
	tlv[1] = 1;
	tlv += tlv[1] + 2;

	/* Write out Optional Data Items Supported */
	tlv[0] = DLEP_OPTIONAL_DATA_ITEMS_TLV;
	tlv[2] = DLEP_RESR_TLV;
	tlv[3] = DLEP_REST_TLV;
	tlv[4] = DLEP_RLQR_TLV;
	tlv[5] = DLEP_RLQT_TLV;
	tlv[6] = DLEP_VENDOR_EXTENSION_TLV;
	tlv[1] = 5;
	tlv += tlv[1] + 2;

	/* Write out Peer Type */
	tlv[0] = DLEP_PEER_TYPE_TLV;
	strcpy(tlv+2,"dlep_router: Simple example DLEP router");
	tlv[1] = strlen(tlv+2);
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Peer Initialization signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
	{
		printf("Failed to send Peer Initialization signal: %s\n",strerror(errno));
		return 0;
	}

	return 1;
}

static void send_heartbeat(int s, uint16_t router_heartbeat_interval)
{
	char msg[30];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_HEARTBEAT;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our Heartbeat Interval */
	tlv[0] = DLEP_HEARTBEAT_INTERVAL_TLV;
	set_uint16(router_heartbeat_interval,tlv+2);
	tlv[1] = 2;
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Heartbeat signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Heartbeat signal: %s\n",strerror(errno));
}

static void send_peer_term(int s, unsigned int status)
{
	char msg[30];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_PEER_TERMINATION;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our Status */
	tlv[0] = DLEP_STATUS_TLV;
	tlv[1] = 1;
	tlv[2] = status;
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Peer Termination signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Peer Termination signal: %s\n",strerror(errno));
}

static void send_peer_term_ack(int s, unsigned int status)
{
	char msg[30];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_PEER_TERMINATION_ACK;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our Status */
	tlv[0] = DLEP_STATUS_TLV;
	tlv[1] = 1;
	tlv[2] = status;
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Peer Termination ACK signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Peer Termination ACK signal: %s\n",strerror(errno));
}

static void send_destination_up_ack(int s, const char* mac)
{
	char msg[30];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_DESTINATION_UP_ACK;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our MAC Address */
	tlv[0] = DLEP_MAC_ADDRESS_TLV;
	tlv[1] = 6;
	memcpy(tlv+2,mac,6);
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Destination Up ACK signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Destination Up ACK signal: %s\n",strerror(errno));
}

static void send_destination_down_ack(int s, const char* mac, int status)
{
	char msg[30];
	char* tlv;
	uint16_t msg_len = 0;

	/* Octet 0 is the signal number */
	msg[0] = DLEP_DESTINATION_DOWN_ACK;

	/* Data items start at octet 3 */
	tlv = msg + 3;

	/* Write out our MAC Address */
	tlv[0] = DLEP_MAC_ADDRESS_TLV;
	tlv[1] = 6;
	memcpy(tlv+2,mac,6);
	tlv += tlv[1] + 2;

	/* Write out our Status */
	tlv[0] = DLEP_STATUS_TLV;
	tlv[1] = 1;
	tlv[2] = status;
	tlv += tlv[1] + 2;

	/* Octet 1 and 2 are the 16bit length of the signal in network byte order */
	msg_len = tlv - msg;
	set_uint16(msg_len,msg+1);

	printf("Sending Destination Down ACK signal\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Destination Down ACK signal: %s\n",strerror(errno));
}

static void parse_vendor_extension_tlv(const char* tlv)
{
	printf("    OUI: ");
	printfBytes(tlv+3,tlv[2],':');
	printf("\n    Device type %u\n    Payload: ",get_uint16(tlv+4));
	if (tlv[7] != tlv[1] - 6 || tlv[7] <= 2)
		printfBytes(tlv+6,tlv[1] - 6,' ');
	else
	{
		printf("\n      Type %u\n      Length %u\n      Value: ",tlv[6],tlv[7]);
		printfBytes(tlv+8,tlv[7] - 2,' ');
	}
	printf("\n");
}

static unsigned int parse_peer_init_ack_signal(const char* tlvs, size_t len, uint16_t* heartbeat_interval)
{
	const char* tlv;
	unsigned int status = 0;

	printf("Valid Peer Initialization ACK signal from modem:\n");

	/* The signal has been validated so just scan for the relevant tlvs */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_HEARTBEAT_INTERVAL_TLV:
			*heartbeat_interval = get_uint16(tlv+2);
			printf("  Heartbeat Interval: %u\n",*heartbeat_interval);
			break;

		case DLEP_PEER_TYPE_TLV:
			printf("  Peer Type: %.*s\n",(int)tlv[1],tlv+2);
			break;

		case DLEP_STATUS_TLV:
			status = tlv[2];
			printf("  Status: %u\n",status);
			break;

		case DLEP_MDRR_TLV:
			printf("  Default MDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_MDRT_TLV:
			printf("  Default MDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRR_TLV:
			printf("  Default CDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRT_TLV:
			printf("  Default CDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_LATENCY_TLV:
			printf("  Default Latency: %"PRIu32"\x03\xBCs\n",get_uint32(tlv+2));
			break;

		case DLEP_RESR_TLV:
			printf("  Default Resources (Receive): %u%%\n",tlv[2]);
			break;

		case DLEP_REST_TLV:
			printf("  Default Resources (Transmit): %u%%\n",tlv[2]);
			break;

		case DLEP_RLQR_TLV:
			printf("  Default RLQR: %u\n",tlv[2]);
			break;

		case DLEP_RLQT_TLV:
			printf("  Default RLQT: %u\n",tlv[2]);
			break;

		case DLEP_OPTIONAL_SIGNALS_TLV:
			{
				const char* tlv_type = tlv + 2;
				const char* end = tlv + tlv[1];

				while (tlv_type < end)
				{
					switch ((enum dlep_tlvs)*tlv_type++)
					{
					case DLEP_PEER_UPDATE:
						printf("  Peer Update signal supported\n");
						break;

					case DLEP_PEER_UPDATE_ACK:
						printf("  Peer Update ACK signal supported\n");
						break;

					case DLEP_LINK_CHARACTERISTICS_ACK:
						printf("  Link Characteristics Request ACK signal supported\n");
						break;
					}
				}
			}
			break;

		case DLEP_OPTIONAL_DATA_ITEMS_TLV:
			{
				const char* tlv_type = tlv + 2;
				const char* end = tlv + tlv[1];

				while (tlv_type < end)
				{
					switch ((enum dlep_tlvs)*tlv_type++)
					{
					case DLEP_RESR_TLV:
						printf("  Resources (Receive) TLV supported\n");
						break;

					case DLEP_REST_TLV:
						printf("  Resources (Transmit) TLV supported\n");
						break;

					case DLEP_RLQR_TLV:
						printf("  RLQR TLV supported\n");
						break;

					case DLEP_RLQT_TLV:
						printf("  RLQT TLV supported\n");
						break;

					case DLEP_VENDOR_EXTENSION_TLV:
						printf("  Vendor Extension TLV supported\n");
						break;
					}
				}
			}
			break;

		case DLEP_VENDOR_EXTENSION_TLV:
			printf("  Default Vendor extension:\n");
			parse_vendor_extension_tlv(tlv);
			break;
		}
	}

	return status;
}

static void parse_address(const char* tlv)
{
	char address[INET6_ADDRSTRLEN] = {0};

	if (tlv[2] == 1)
		printf("  Add ");
	else
		printf("  Drop ");

	if (tlv[1] == 5)
		printf("IPv4 address: %s\n",inet_ntop(AF_INET,tlv+3,address,sizeof(address)));
	else
		printf("IPv6 address: %s\n",inet_ntop(AF_INET6,tlv+3,address,sizeof(address)));
}

static void parse_peer_update_signal(const char* tlvs, size_t len)
{
	const char* tlv;
	unsigned int status = 0;

	printf("Received Peer Update signal from modem:\n");

	/* The signal has been validated so just scan for the relevant tlvs */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_IPV4_ADDRESS_TLV:
		case DLEP_IPV6_ADDRESS_TLV:
			parse_address(tlv);
			break;

		case DLEP_MDRR_TLV:
			printf("  Peer MDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_MDRT_TLV:
			printf("  Peer MDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRR_TLV:
			printf("  Peer CDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRT_TLV:
			printf("  Peer CDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_LATENCY_TLV:
			printf("  Peer Latency: %"PRIu32"\x03\xBCs\n",get_uint32(tlv+2));
			break;

		case DLEP_RESR_TLV:
			printf("  Peer Resources (Receive): %u%%\n",tlv[2]);
			break;

		case DLEP_REST_TLV:
			printf("  Peer Resources (Transmit): %u%%\n",tlv[2]);
			break;

		case DLEP_RLQR_TLV:
			printf("  Peer RLQR: %u\n",tlv[2]);
			break;

		case DLEP_RLQT_TLV:
			printf("  Peer RLQT: %u\n",tlv[2]);
			break;

		case DLEP_VENDOR_EXTENSION_TLV:
			printf("  Peer Vendor extension:\n");
			parse_vendor_extension_tlv(tlv);
			break;
		}
	}
}

static void parse_destination_up_signal(int s, const char* tlvs, size_t len)
{
	const char* tlv;
	unsigned int status = 0;

	printf("Received Destination Up signal from modem:\n");

	/* The signal has been validated so just scan for the relevant tlvs */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_MAC_ADDRESS_TLV:
			printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",tlv[2],tlv[3],tlv[4],tlv[5],tlv[6],tlv[7]);

			send_destination_up_ack(s,tlv+2);
			break;

		case DLEP_IPV4_ADDRESS_TLV:
		case DLEP_IPV6_ADDRESS_TLV:
			parse_address(tlv);
			break;

		case DLEP_MDRR_TLV:
			printf("  MDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_MDRT_TLV:
			printf("  MDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRR_TLV:
			printf("  CDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRT_TLV:
			printf("  CDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_LATENCY_TLV:
			printf("  Latency: %"PRIu32"\x03\xBCs\n",get_uint32(tlv+2));
			break;

		case DLEP_RESR_TLV:
			printf("  Resources (Receive): %u%%\n",tlv[2]);
			break;

		case DLEP_REST_TLV:
			printf("  Resources (Transmit): %u%%\n",tlv[2]);
			break;

		case DLEP_RLQR_TLV:
			printf("  RLQR: %u\n",tlv[2]);
			break;

		case DLEP_RLQT_TLV:
			printf("  RLQT: %u\n",tlv[2]);
			break;
		}
	}
}

static void parse_destination_update_signal(const char* tlvs, size_t len)
{
	const char* tlv;
	unsigned int status = 0;

	printf("Received Destination Update signal from modem:\n");

	/* The signal has been validated so just scan for the relevant tlvs */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_MAC_ADDRESS_TLV:
			printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",tlv[2],tlv[3],tlv[4],tlv[5],tlv[6],tlv[7]);
			break;

		case DLEP_IPV4_ADDRESS_TLV:
		case DLEP_IPV6_ADDRESS_TLV:
			parse_address(tlv);
			break;

		case DLEP_MDRR_TLV:
			printf("  MDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_MDRT_TLV:
			printf("  MDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRR_TLV:
			printf("  CDDR: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_CDRT_TLV:
			printf("  CDDT: %"PRIu64"bps\n",get_uint64(tlv+2));
			break;

		case DLEP_LATENCY_TLV:
			printf("  Latency: %"PRIu32"\x03\xBCs\n",get_uint32(tlv+2));
			break;

		case DLEP_RESR_TLV:
			printf("  Resources (Receive): %u%%\n",tlv[2]);
			break;

		case DLEP_REST_TLV:
			printf("  Resources (Transmit): %u%%\n",tlv[2]);
			break;

		case DLEP_RLQR_TLV:
			printf("  RLQR: %u\n",tlv[2]);
			break;

		case DLEP_RLQT_TLV:
			printf("  RLQT: %u\n",tlv[2]);
			break;
		}
	}
}

static int handle_signal(int s, const char* msg, size_t len, uint16_t* modem_heartbeat_interval)
{
	int ret = 0;
	if (len < 3)
	{
		printf("Packet too short for a DLEP signal: %u bytes\n",(unsigned int)len);

		send_peer_term(s,1);
	}
	else
	{
		uint16_t reported_len = get_uint16(msg+1);
		if (reported_len != len)
		{
			printf("Received signal length %u does not match received packet length %u\n",reported_len,(unsigned int)len);

			send_peer_term(s,1);
		}
		else
		{
			int status = 0;

			ret = 1;

			/* Check the signal type */
			switch ((enum dlep_signals)msg[0])
			{
			case DLEP_PEER_DISCOVERY:
				printf("Unexpected Peer Discovery signal received during 'in session' state\n");
				status = 1;
				break;

			case DLEP_PEER_OFFER:
				printf("Unexpected Peer Offer signal received during 'in session' state\n");
				status = 1;
				break;

			case DLEP_PEER_INITIALIZATION:
				printf("Unexpected Peer Initialization signal received during 'in session' state\n");
				status = 1;
				break;

			case DLEP_PEER_INITIALIZATION_ACK:
				printf("Unexpected Peer Initialization ACK signal received during 'in session' state\n");
				status = 1;
				break;

			case DLEP_PEER_TERMINATION:
				if (!check_peer_term_signal(msg+3,len-3))
					send_peer_term_ack(s,1);
				else
				{
					printf("Received Peer Termination signal from modem\n");
					send_peer_term_ack(s,0);
				}
				break;

			case DLEP_PEER_TERMINATION_ACK:
				printf("Unexpected Peer Termination Ack signal received during 'in session' state\n");
				status = 1;
				break;

			case DLEP_PEER_UPDATE:
				if (!check_peer_update_signal(msg+3,len-3))
					status = 1;
				else
					parse_peer_update_signal(msg+3,len-3);
				break;

			case DLEP_PEER_UPDATE_ACK:
				printf("Unexpected Peer Update Ack signal received\n");
				status = 1;
				break;

			case DLEP_DESTINATION_UP:
				if (!check_destination_up_signal(msg+3,len-3))
					status = 1;
				else
					parse_destination_up_signal(s,msg+3,len-3);
				break;

			case DLEP_DESTINATION_UP_ACK:
				printf("Unexpected Destination Up Ack signal received\n");
				status = 1;
				break;

			case DLEP_DESTINATION_DOWN:
				if (!check_destination_down_signal(msg+3,len-3))
					status = 1;
				else
				{
					printf("Received Destination Update signal from modem:\n");

					/* The signal has been validated so just scan for the relevant tlvs */
					printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",msg[5],msg[6],msg[7],msg[8],msg[9],msg[10]);

					send_destination_down_ack(s,msg+5,0);
				}
				break;

			case DLEP_DESTINATION_DOWN_ACK:
				printf("Unexpected Destination Up Ack signal received\n");
				status = 1;
				break;

			case DLEP_DESTINATION_UPDATE:
				if (!check_destination_update_signal(msg+3,len-3))
					status = 1;
				else
					parse_destination_update_signal(msg+3,len-3);
				break;

			case DLEP_LINK_CHARACTERISTICS_REQUEST:
				printf("Unsupported Link Characteristics Request signal received\n");
				status = 1;
				break;

			case DLEP_LINK_CHARACTERISTICS_ACK:
				printf("Unsupported Link Characteristics Ack signal received\n");
				status = 1;
				break;

			case DLEP_HEARTBEAT:
				if (!check_heartbeat_signal(msg+3,len-3))
					status = 1;
				else
				{
					uint16_t hb = get_uint16(msg+5);

					printf("Received Heartbeat signal from modem\n");

					if (hb != *modem_heartbeat_interval)
					{
						printf("Modem Heartbeat Interval changed to %u secs\n",hb);
						*modem_heartbeat_interval = hb;
					}
				}
				break;

			default:
				printf("Unrecognized signal %u received\n",msg[0]);
				status = 1;
				break;
			}

			if (status)
			{
				send_peer_term(s,status);
				ret = 0;
			}
		}
	}

	return ret;
}

static void in_session(int s, uint16_t modem_heartbeat_interval, uint16_t router_heartbeat_interval)
{
	char msg[1500] = {0};
	struct timespec last_recv_time = {0};
	struct timespec last_sent_time = {0};
	struct timespec now_time = {0};

	/* Remember when we started */
	clock_gettime(CLOCK_MONOTONIC,&last_recv_time);
	clock_gettime(CLOCK_MONOTONIC,&last_sent_time);

	/* Loop forever */
	for (;;)
	{
		ssize_t received;

		if (modem_heartbeat_interval || router_heartbeat_interval)
		{
			/* Wait for a signal */
			struct timeval timeout = {0};
			fd_set readfds;

			FD_ZERO(&readfds);
			FD_SET(s,&readfds);

			/* Use the lower interval, not 0 */
			timeout.tv_sec = (modem_heartbeat_interval < router_heartbeat_interval ? modem_heartbeat_interval : router_heartbeat_interval);
			if (!timeout.tv_sec)
				timeout.tv_sec = (modem_heartbeat_interval > router_heartbeat_interval ? modem_heartbeat_interval : router_heartbeat_interval);;

			if (select(s+1,&readfds,NULL,NULL,&timeout) == -1)
			{
				printf("Failed to wait for signal: %s\n",strerror(errno));
				return;
			}

			if (!FD_ISSET(s,&readfds))
			{
				/* Timeout */
				clock_gettime(CLOCK_MONOTONIC,&now_time);

				/* Check Modem heartbeat interval, check for 2 missed intervals */
				if (modem_heartbeat_interval && interval_compare(&last_recv_time,&now_time,modem_heartbeat_interval * 2) > 0)
				{
					printf("No heartbeat from modem within %u seconds, terminating session\n",modem_heartbeat_interval * 2);
					break;
				}

				/* Check for our heartbeat interval */
				if (router_heartbeat_interval && interval_compare(&last_sent_time,&now_time,router_heartbeat_interval) > 0)
				{
					/* Send out a heartbeat if the 'timer' has expired */
					send_heartbeat(s,router_heartbeat_interval);

					last_sent_time = now_time;
				}

				/* Wait again */
				continue;
			}
		}

		/* Receive a signal */
		received = recv(s,msg,sizeof(msg),0);
		if (received == -1)
		{
			printf("Failed to receive from TCP socket: %s\n",strerror(errno));
			break;
		}
		if (received == 0)
		{
			printf("Modem disconnected TCP session\n");
			break;
		}

		/* Update the last received time */
		clock_gettime(CLOCK_MONOTONIC,&last_recv_time);

		/* Handle the signal */
		if (!handle_signal(s,msg,received,&modem_heartbeat_interval))
			break;
	}
}

void session(/* [in] */ const struct sockaddr* modem_address, /* [int] */ socklen_t modem_address_length, /* [int] */ uint16_t modem_heartbeat_interval, /* [int] */ uint16_t router_heartbeat_interval)
{
	char str_address[FORMATADDRESS_LEN] = {0};

	/* First we must initialise, draft section 7.2 */
	int s = socket(modem_address->sa_family,SOCK_STREAM,0);
	if (s == -1)
	{
		printf("Failed to create socket: %s\n",strerror(errno));
		return;
	}

	printf("Connecting to modem at %s\n",formatAddress(modem_address,str_address,sizeof(str_address)));

	/* Connect to the modem */
	if (connect(s,modem_address,modem_address_length) == -1)
	{
		printf("Failed to connect socket: %s\n",strerror(errno));
	}
	else if (send_peer_init_signal(s,router_heartbeat_interval))
	{
		char msg[1500];
		ssize_t received;

		printf("Waiting for Peer Initialization ACK signal\n");

		/* Receive a Peer Initialization ACK signal */
		received = recv(s,msg,sizeof(msg),0);
		if (received == -1)
			printf("Failed to receive from TCP socket: %s\n",strerror(errno));
		else if (received == 0)
			printf("Modem disconnected TCP session\n");
		else
		{
			printf("Received possible Peer Initialization ACK signal (%u bytes)\n",(unsigned int)received);

			/* Check it's a valid Peer Initialization ACK signal */
			if (check_peer_init_ack_signal(msg,received))
			{
				unsigned int status = parse_peer_init_ack_signal(msg+3,received-3,&modem_heartbeat_interval);
				if (status != 0)
				{
					printf("Non-zero Status TLV in Peer Initialization ACK signal: %u, Terminating\n",status);
				}
				else
				{
					printf("Moving to 'in-session' state\n");

					in_session(s,modem_heartbeat_interval,router_heartbeat_interval);
				}
			}
		}
	}

	close(s);
}
