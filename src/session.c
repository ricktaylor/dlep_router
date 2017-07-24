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
#include <unistd.h>

#include "./dlep_iana.h"
#include "./check.h"

static uint8_t* write_message_header(uint8_t* msg, uint16_t msg_type)
{
	/* Octet 0 and 1 are the message type */
	uint8_t* p = write_uint16(msg_type,msg);

	/* Octet 2 and 3 are the message length, set to 0 initially */
	return write_uint16(0,p);
}

static uint8_t* write_data_item(uint8_t* msg, uint16_t type, uint16_t length)
{
	/* Octet 0 and 1 are the type code */
	uint8_t* p = write_uint16(type,msg);

	/* Octet 2 and 3 are the length, this excludes the header length (4) */
	return write_uint16(length,p);
}

static uint8_t* write_status_code(uint8_t* msg, enum dlep_status_code sc)
{
	/* Write out Status Code header */
	msg = write_data_item(msg,DLEP_STATUS_DATA_ITEM,1);

	/* And the code */
	*msg++ = sc;

	return msg;
}

/* static int send_session_init_message(int s, uint16_t router_heartbeat_interval) */
static int send_session_init_message(int s, uint32_t router_heartbeat_interval)
{
	uint8_t msg[300];
	uint16_t msg_len = 0;
	size_t peer_type_len = 0;

	/* Write the message header */
	uint8_t* p = NULL;

	p = write_message_header(msg,DLEP_SESSION_INIT);

	/* Write out our Heartbeat Interval */
	p = write_data_item(p,DLEP_HEARTBEAT_INTERVAL_DATA_ITEM,4);
	/*p = write_uint16(router_heartbeat_interval,p); */
	p = write_uint32(router_heartbeat_interval,p); 

	/* Write out Peer Type */
	peer_type_len = strlen(PEER_TYPE);
	if (peer_type_len)
	{
                uint8_t flags = 0;

		p = write_data_item(p,DLEP_PEER_TYPE_DATA_ITEM, 1+ peer_type_len);  /* includes length of the flag + description fields */
                p[0] = flags;
                p++;

                memcpy(p, PEER_TYPE, peer_type_len);
                p+= peer_type_len;
	}

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Session Initialization message\n");

	if (send(s,msg,msg_len,0) != msg_len)
	{
		printf("Failed to send Session Initialization message: %s\n",strerror(errno));
		return 0;
	}

	return 1;
}

static void send_heartbeat(int s, uint16_t router_heartbeat_interval)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_PEER_HEARTBEAT);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Heartbeat message\n");
  
	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Heartbeat message: %s\n",strerror(errno));
}

static void send_session_term(int s, enum dlep_status_code sc)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_SESSION_TERM);

	/* Write out our Status Code */
	p = write_status_code(p,sc);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Session Termination message\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Session Termination message: %s\n",strerror(errno));
}

static void send_session_term_resp(int s, enum dlep_status_code sc)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_SESSION_TERM_RESP);

	/* Write out our Status code */
	p = write_status_code(p,sc);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Session Termination Response message\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Session Termination Response message: %s\n",strerror(errno));
}

static void send_destination_up_resp(int s, const uint8_t* mac, enum dlep_status_code sc)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_DEST_UP_RESP);

	/* Write out our MAC Address */
	p = write_data_item(p,DLEP_MAC_ADDRESS_DATA_ITEM,6);
	memcpy(p,mac,6);
	p += 6;

	/* Write out our Status code */
	p = write_status_code(p,sc);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Destination Up Response message\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Destination Up Response message: %s\n",strerror(errno));
}

static void send_destination_down_resp(int s, const uint8_t* mac, enum dlep_status_code sc)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_DEST_DOWN_RESP);

	/* Write out our MAC Address */
	p = write_data_item(p,DLEP_MAC_ADDRESS_DATA_ITEM,6);
	memcpy(p,mac,6);
	p += 6;

	/* Write out our Status code */
	p = write_status_code(p,sc);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Destination Down Response message\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Destination Down Response message: %s\n",strerror(errno));
}

static void send_link_char_request_resp(int s, const uint8_t* mac, enum dlep_status_code sc)
{
	uint8_t msg[30];
	uint16_t msg_len = 0;

	/* Write the message header */
	uint8_t* p = write_message_header(msg,DLEP_LINK_CHAR_RESP);

	/* Write out our MAC Address */
	p = write_data_item(p,DLEP_MAC_ADDRESS_DATA_ITEM,6);
	memcpy(p,mac,6);
	p += 6;

	/* Write out our Status code */
	p = write_status_code(p,sc);

	msg_len = p - msg;

	/* Octet 2 and 3 are the message length, minus the length of the header */
	write_uint16(msg_len - 4,msg + 2);

	printf("Sending Link Characteristics Response message\n");

	if (send(s,msg,msg_len,0) != msg_len)
		printf("Failed to send Link Characteristics Response message: %s\n",strerror(errno));
}

static void printf_status(enum dlep_status_code sc)
{
	switch (sc)
	{
	case DLEP_SC_SUCCESS:
		printf("  Status: %u - Success\n",sc);
		break;

	case DLEP_SC_UNKNOWN_MESSAGE:
		printf("  Status: %u - Unknown Signal\n",sc);
		break;

	case DLEP_SC_INVALID_DATA:
		printf("  Status: %u - Invalid Data\n",sc);
		break;

	case DLEP_SC_UNEXPECTED_MESSAGE:
		printf("  Status: %u - Unexpected Signal\n",sc);
		break;

	case DLEP_SC_INVALID_DEST:
		printf("  Status: %u - Invalid Destination\n",sc);
		break;

	case DLEP_SC_NOT_INTERESTED:
		printf("  Status: %u - Not Interested\n",sc);
		break;

	case DLEP_SC_REQUEST_DENIED:
		printf("  Status: %u - Request Denied\n",sc);
		break;

	case DLEP_SC_TIMEDOUT:
		printf("  Status: %u - Timed Out\n",sc);
		break;

	default:
		printf("  Status: %u - <Unknown Status Code!>\n",sc);
		break;
	}
}

static enum dlep_status_code parse_session_init_resp_message(const uint8_t* data_items, uint16_t len, uint16_t* heartbeat_interval, enum dlep_status_code* sc)
{
	const uint8_t* data_item = data_items;

	printf("Valid Session Initialization Response message from modem:\n");

	/* The message has been validated so just scan for the relevant data_items */
	while (data_item < data_items + len)
	{
		/* Octets 0 and 1 are the data item type */
		enum dlep_data_item item_id = read_uint16(data_item);

		/* Octets 2 and 3 are the data item length */
		uint16_t item_len = read_uint16(data_item + 2);

		/* Increment data_item to point to the data */
		data_item += 4;

		switch (item_id)
		{
		case DLEP_HEARTBEAT_INTERVAL_DATA_ITEM:
			*heartbeat_interval = read_uint16(data_item);
			printf("  Heartbeat Interval: %u\n",*heartbeat_interval);
			break;

		case DLEP_PEER_TYPE_DATA_ITEM:
			printf("  Peer Type: %.*s\n",(int)item_len,data_item);
			break;

		case DLEP_STATUS_DATA_ITEM:
			*sc = data_item[0];
			printf_status(*sc);
			break;

		case DLEP_MDRR_DATA_ITEM:
			printf("  Default MDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_MDRT_DATA_ITEM:
			printf("  Default MDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRR_DATA_ITEM:
			printf("  Default CDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRT_DATA_ITEM:
			printf("  Default CDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_LATENCY_DATA_ITEM:
			/*printf("  Default Latency: %"PRIu32"\x03\xBCs\n",read_uint32(data_item));*/
			printf("  Default Latency: %"PRIu64"us\n",read_uint64(data_item));
			break;

		case DLEP_RESR_DATA_ITEM:
			printf("  Default Resources (Receive): %u%%\n",data_item[0]);
			break;

		case DLEP_REST_DATA_ITEM:
			printf("  Default Resources (Transmit): %u%%\n",data_item[0]);
			break;

		case DLEP_RLQR_DATA_ITEM:
			printf("  Default RLQR: %u\n",data_item[0]);
			break;

		case DLEP_RLQT_DATA_ITEM:
			printf("  Default RLQT: %u\n",data_item[0]);
			break;

		case DLEP_EXTS_SUPP_DATA_ITEM:
			if (item_len > 0)
			{
				size_t i = 0;
				printf("  Extensions advertised by peer:\n");

				for (; i < item_len; i += 2)
				{
					uint16_t ext_id = read_uint16(data_item + i);
					switch (ext_id)
					{
					default:
						printf("Unknown DLEP extension %u\n",ext_id);
						break;
					}
				}
			}
			break;
                default:
                        printf("  Unknown\n");
                        break;
		}

		/* Increment data_item to point to the next data item */
		data_item += item_len;
	}
	return DLEP_SC_SUCCESS;
}

static void parse_address(const uint8_t* data_item, uint16_t item_len)
{
	char address[INET6_ADDRSTRLEN] = {0};

	if (data_item[0] == 1)
		printf("  Add ");
	else
		printf("  Drop ");

	if (item_len == 5)
		printf("IPv4 address: %s\n",inet_ntop(AF_INET,data_item+1,address,sizeof(address)));
#if 0
	else
		printf("IPv6 address: %s\n",inet_ntop(AF_INET6,data_item+1,address,sizeof(address)));
#endif
}

static void parse_attached_subnet(const uint8_t* data_item, uint16_t item_len)
{
	char address[INET6_ADDRSTRLEN] = {0};

	if (item_len == 5)
		printf("IPv4 attached subnet: %s/%u\n",inet_ntop(AF_INET,data_item,address,sizeof(address)),(unsigned int)data_item[4]);
#if 0
	else
		printf("IPv6 attached subnet: %s/%u\n",inet_ntop(AF_INET6,data_item,address,sizeof(address)),(unsigned int)data_item[16]);
#endif
}

static void parse_session_update_message(const uint8_t* data_items, uint16_t len)
{
	const uint8_t* data_item = data_items;

	printf("Received Session Update message from modem:\n");

	/* The message has been validated so just scan for the relevant data_items */
	while (data_item < data_items + len)
	{
		/* Octets 0 and 1 are the data item type */
		enum dlep_data_item item_id = read_uint16(data_item);

		/* Octets 2 and 3 are the data item length */
		uint16_t item_len = read_uint16(data_item + 2);

		/* Increment data_item to point to the data */
		data_item += 4;

		switch (item_id)
		{
		case DLEP_IPV4_ADDRESS_DATA_ITEM:
		case DLEP_IPV6_ADDRESS_DATA_ITEM:
			parse_address(data_item,item_len);
			break;

		case DLEP_MDRR_DATA_ITEM:
			printf("  Session MDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_MDRT_DATA_ITEM:
			printf("  Session MDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRR_DATA_ITEM:
			printf("  Session CDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRT_DATA_ITEM:
			printf("  Session CDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_LATENCY_DATA_ITEM:
			printf("  Session Latency: %"PRIu32"\x03\xBCs\n",read_uint32(data_item));
			break;

		case DLEP_RESR_DATA_ITEM:
			printf("  Session Resources (Receive): %u%%\n",data_item[0]);
			break;

		case DLEP_REST_DATA_ITEM:
			printf("  Session Resources (Transmit): %u%%\n",data_item[0]);
			break;

		case DLEP_RLQR_DATA_ITEM:
			printf("  Session RLQR: %u\n",data_item[0]);
			break;

		case DLEP_RLQT_DATA_ITEM:
			printf("  Session RLQT: %u\n",data_item[0]);
			break;
                default:
			printf("  Unknown\n");
                        break;
		}

		/* Increment data_item to point to the next data item */
		data_item += item_len;
	}
}

static void parse_destination_up_message(int s, const uint8_t* data_items, uint16_t len)
{
	const uint8_t* data_item = data_items;

	printf("Received Destination Up message from modem:\n");

	/* The message has been validated so just scan for the relevant data_items */
	while (data_item < data_items + len)
	{
		/* Octets 0 and 1 are the data item type */
		enum dlep_data_item item_id = read_uint16(data_item);

		/* Octets 2 and 3 are the data item length */
		uint16_t item_len = read_uint16(data_item + 2);

		/* Increment data_item to point to the data */
		data_item += 4;

		switch (item_id)
		{
		case DLEP_MAC_ADDRESS_DATA_ITEM:
			printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",data_item[0],data_item[1],data_item[2],data_item[3],data_item[4],data_item[5]);
			send_destination_up_resp(s,data_item,DLEP_SC_SUCCESS);
			break;

		case DLEP_IPV4_ADDRESS_DATA_ITEM:
		case DLEP_IPV6_ADDRESS_DATA_ITEM:
			parse_address(data_item,item_len);
			break;

		case DLEP_IPV4_ATT_SUBNET_DATA_ITEM:
		case DLEP_IPV6_ATT_SUBNET_DATA_ITEM:
			parse_attached_subnet(data_item,item_len);
			break;

		case DLEP_MDRR_DATA_ITEM:
			printf("  MDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_MDRT_DATA_ITEM:
			printf("  MDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRR_DATA_ITEM:
			printf("  CDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRT_DATA_ITEM:
			printf("  CDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_LATENCY_DATA_ITEM:
			printf("  Latency: %"PRIu32"\x03\xBCs\n",read_uint32(data_item));
			break;

		case DLEP_RESR_DATA_ITEM:
			printf("  Resources (Receive): %u%%\n",data_item[0]);
			break;

		case DLEP_REST_DATA_ITEM:
			printf("  Resources (Transmit): %u%%\n",data_item[0]);
			break;

		case DLEP_RLQR_DATA_ITEM:
			printf("  RLQR: %u\n",data_item[0]);
			break;

		case DLEP_RLQT_DATA_ITEM:
			printf("  RLQT: %u\n",data_item[0]);
			break;
                default:
                        printf("  Unknown\n");
                        break;
		}

		/* Increment data_item to point to the next data item */
		data_item += item_len;
	}
}

static void parse_destination_update_message(const uint8_t* data_items, uint16_t len)
{
	const uint8_t* data_item = data_items;

	printf("Received Destination Update message from modem:\n");

	/* The message has been validated so just scan for the relevant data_items */
	while (data_item < data_items + len)
	{
		/* Octets 0 and 1 are the data item type */
		enum dlep_data_item item_id = read_uint16(data_item);

		/* Octets 2 and 3 are the data item length */
		uint16_t item_len = read_uint16(data_item + 2);

		/* Increment data_item to point to the data */
		data_item += 4;

		switch (item_id)
		{
		case DLEP_MAC_ADDRESS_DATA_ITEM:
			printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",data_item[0],data_item[1],data_item[2],data_item[3],data_item[4],data_item[5]);
			break;

		case DLEP_IPV4_ADDRESS_DATA_ITEM:
		case DLEP_IPV6_ADDRESS_DATA_ITEM:
			parse_address(data_item,item_len);
			break;

		case DLEP_IPV4_ATT_SUBNET_DATA_ITEM:
		case DLEP_IPV6_ATT_SUBNET_DATA_ITEM:
			parse_attached_subnet(data_item,item_len);
			break;

		case DLEP_MDRR_DATA_ITEM:
			printf("  MDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_MDRT_DATA_ITEM:
			printf("  MDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRR_DATA_ITEM:
			printf("  CDDR: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_CDRT_DATA_ITEM:
			printf("  CDDT: %"PRIu64"bps\n",read_uint64(data_item));
			break;

		case DLEP_LATENCY_DATA_ITEM:
			printf("  Latency: %"PRIu32"\x03\xBCs\n",read_uint32(data_item));
			break;

		case DLEP_RESR_DATA_ITEM:
			printf("  Resources (Receive): %u%%\n",data_item[0]);
			break;

		case DLEP_REST_DATA_ITEM:
			printf("  Resources (Transmit): %u%%\n",data_item[0]);
			break;

		case DLEP_RLQR_DATA_ITEM:
			printf("  RLQR: %u\n",data_item[0]);
			break;

		case DLEP_RLQT_DATA_ITEM:
			printf("  RLQT: %u\n",data_item[0]);
			break;
                default:
                        printf("  Unknown\n");
                        break;
		}

		/* Increment data_item to point to the next data item */
		data_item += item_len;
	}
}

static int handle_message(int s, const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = DLEP_SC_SUCCESS;

	const uint8_t* mac;

	/* Octets 0 and 1 are the message type */
	enum dlep_message msg_id = read_uint16(msg);

	/* Octets 2 and 3 are the message length */
	uint16_t msg_len = read_uint16(msg + 2);

	/* Check the message type */
	switch (msg_id)
	{
	case DLEP_SESSION_INIT:
		printf("Unexpected Session Initialization message received during 'in session' state\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_SESSION_INIT_RESP:
		printf("Unexpected Session Initialization Response message received during 'in session' state\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_SESSION_TERM:
		sc = check_session_term_message(msg,len);
		if (sc != DLEP_SC_SUCCESS)
			send_session_term_resp(s,sc);
		else
		{
			printf("Received Session Termination message from modem\n");
			send_session_term_resp(s,DLEP_SC_SUCCESS);
		}
		break;

	case DLEP_SESSION_TERM_RESP:
		printf("Unexpected Session Termination Response message received during 'in session' state\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_SESSION_UPDATE:
		sc = check_session_update_message(msg,len);
		if (sc == DLEP_SC_SUCCESS)
			parse_session_update_message(msg+4,msg_len);
		break;

	case DLEP_SESSION_UPDATE_RESP:
		printf("Unexpected Session Update Response message received\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_DEST_UP:
		sc = check_destination_up_message(msg,len);
		if (sc == DLEP_SC_SUCCESS)
			parse_destination_up_message(s,msg+4,msg_len);
		break;

	case DLEP_DEST_UP_RESP:
		printf("Unexpected Destination Up Response message received\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_DEST_DOWN:
		sc = check_destination_down_message(msg,len,&mac);
		if (sc == DLEP_SC_SUCCESS)
		{
			printf("Received Destination Down message from modem:\n");

			/* The message has been validated so just scan for the relevant data_items */
			printf("  MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",msg[8],msg[9],msg[10],msg[11],msg[12],msg[13]);

			send_destination_down_resp(s,mac,0);
		}
		break;

	case DLEP_DEST_DOWN_RESP:
		printf("Unexpected Destination Up Response message received\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_DEST_UPDATE:
		/* sc = check_destination_update_message(msg,len,&mac); */
		sc = check_destination_update_message(msg,len);
		if (sc == DLEP_SC_SUCCESS)
			parse_destination_update_message(msg+4,msg_len);
		break;

	case DLEP_LINK_CHAR_REQ:
		sc = check_link_char_request_message(msg,len,&mac);
		if (sc == DLEP_SC_SUCCESS)
		{
			printf("Received Link Characteristics Request message from modem:\n");

			send_link_char_request_resp(s,mac,DLEP_SC_REQUEST_DENIED);
		}
		break;

	case DLEP_LINK_CHAR_RESP:
		printf("Unexpected Link Characteristics Response message received. We don't send them.\n");
		sc = DLEP_SC_UNEXPECTED_MESSAGE;
		break;

	case DLEP_PEER_HEARTBEAT:
		sc = check_heartbeat_message(msg,len);
		if (sc == DLEP_SC_SUCCESS)
		{
			printf("Received Heartbeat message from modem\n");
		}
		break;

	default:
		printf("Unrecognized message %u received\n",msg_id);
		sc = DLEP_SC_UNKNOWN_MESSAGE;
		break;
	}

	if (sc && sc < DLEP_SC_NOT_INTERESTED)
	{
		send_session_term(s,sc);
		return 0;
	}

	if (msg_id == DLEP_SESSION_TERM)
		return 0;

	return 1;
}

static ssize_t recv_message(int s, uint8_t** msg)
{
	ssize_t received = -1;

	/* Make sure we have room for the header */
	uint8_t* new_msg = realloc(*msg,4);
	if (!new_msg)
	{
		int err = errno;
		printf("Failed to allocate message buffer");
		errno = err;
		return -1;
	}

	*msg = new_msg;

	/* Receive the header */
	received = recv(s,*msg,4,0);
	if (received == 4)
	{
		/* Read the message length */
		uint16_t reported_len = read_uint16((*msg)+2);
		if (reported_len)
		{
			/* Make more room in the message buffer */
			if (!new_msg)
			{
				int err = errno;
				printf("Failed to allocate message buffer");
				errno = err;
				return -1;
			}

	                new_msg = realloc(*msg,reported_len+4);
			*msg = new_msg;

			/* Receive the rest of the message */
			received = recv(s,(*msg)+4,reported_len,0);
			if (received == -1)
				return -1;

			/* Include the header length in the return value */
			received += 4;
		}
	}

	return received;
}

static void in_session(int s, uint8_t* msg, uint16_t modem_heartbeat_interval, uint16_t router_heartbeat_interval)
{
	struct timespec last_recv_time = {0};
	struct timespec last_sent_time = {0};
	struct timespec now_time = {0};

	/* Remember when we started */
	clock_gettime(CLOCK_MONOTONIC,&last_recv_time);
	clock_gettime(CLOCK_MONOTONIC,&last_sent_time);

	/* Loop forever handling messages */
	for (;;)
	{
		ssize_t received;

		if (modem_heartbeat_interval || router_heartbeat_interval)
		{
			/* Wait for a message */
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
				printf("Failed to wait for message: %s\n",strerror(errno));
				return;
			}

			clock_gettime(CLOCK_MONOTONIC,&now_time);

			/* Check for our heartbeat interval */
			if (router_heartbeat_interval && interval_compare(&last_sent_time,&now_time,router_heartbeat_interval) > 0)
			{
				/* Send out a heartbeat if the 'timer' has expired */
				send_heartbeat(s,router_heartbeat_interval);

				last_sent_time = now_time;
			}

			if (!FD_ISSET(s,&readfds))
			{
				/* Timeout */

				/* Check Modem heartbeat interval, check for 2 missed intervals */
				if (modem_heartbeat_interval && interval_compare(&last_recv_time,&now_time,modem_heartbeat_interval * 2) > 0)
				{
					printf("No heartbeat from modem within %u seconds, terminating session\n",modem_heartbeat_interval * 2);
					break;
				}

				/* Wait again */
				continue;
			}
		}

		/* Receive a message */
		received = recv_message(s,&msg);
		if (received == -1)
		{
			printf("Failed to receive from TCP socket: %s\n",strerror(errno));
			break;
		}
		if (received < 4)
		{
			printf("Modem disconnected TCP session\n");
			break;
		}

		/* Update the last received time */
		clock_gettime(CLOCK_MONOTONIC,&last_recv_time);

		/* Handle the message */
		if (!handle_message(s,msg,received))
			break;
	}
}

void session(/* [in] */ const struct sockaddr* modem_address, /* [int] */ socklen_t modem_address_length, /* [int] */ uint16_t modem_heartbeat_interval, /* [int] */ uint32_t router_heartbeat_interval)
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
exit(1);
	}
	else if (send_session_init_message(s,router_heartbeat_interval))
	{
		uint8_t* msg = malloc(4);
		ssize_t received;

		printf("Waiting for Session Initialization Response message\n");

		/* Receive a Session Initialization Response message */
		received = recv_message(s,&msg);
		if (received == -1)
			printf("Failed to receive from TCP socket: %s\n",strerror(errno));
		else if (received == 0)
			printf("Modem disconnected TCP session\n");
		else
		{
                        int terminate = 0;

			printf("Received possible Session Initialization Response message (%u bytes)\n",(unsigned int)received);

			/* Check it's a valid Session Initialization Response message */
			if (!check_session_init_resp_message(msg,received, &terminate))
			{
				enum dlep_status_code init_sc = DLEP_SC_SUCCESS;
				enum dlep_status_code sc = parse_session_init_resp_message(msg+4,received-4,&modem_heartbeat_interval,&init_sc);
				if (sc != DLEP_SC_SUCCESS)
				{
					send_session_term(s,sc);
				}
				else if (init_sc != DLEP_SC_SUCCESS)
				{
					printf("Non-zero Status data item in Session Initialization Response message: %u, Terminating\n",init_sc);

					send_session_term(s,DLEP_SC_SUCCESS);
				}
				else
				{
					printf("Moving to 'in-session' state\n");

					in_session(s,msg,modem_heartbeat_interval,router_heartbeat_interval);
				}
			}
		}

		free(msg);
	}

	close(s);
}
