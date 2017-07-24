/*

Copyright (c) 2014 Airbus DS Limited

*/

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <stdlib.h>

#include "./dlep_iana.h"
#include "./util.h"

static enum dlep_status_code check_length(uint16_t item_len, unsigned int expected_len, const char* name)
{
	if (item_len != expected_len)
	{
		printf("Incorrect length in %s data item: %u, expected %u\n",name,item_len,expected_len);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_peer_type(const uint8_t* data_item, uint16_t item_len)
{
	size_t i = 0;

/* ignore the first field, as it's a Flags field */

/*	for (i=0; i < (item_len - 1); ++i) */
	for (i=1; i < (item_len); ++i)
	{
		/* Check for NUL */
		if (data_item[i] == 0)
		{
			printf("Warning: Suspicious NUL character in peer type string\n");
		}

		/* TODO: One should check for valid UTF8 characters here */
	}

	return DLEP_SC_SUCCESS;
}

/*static enum dlep_status_code check_heartbeat_interval(const uint8_t* data_item, uint16_t item_len) */
static enum dlep_status_code check_heartbeat_interval(const uint8_t* data_item, uint32_t item_len)
{
	return check_length(item_len,4,"Heartbeat Interval");
}

static enum dlep_status_code check_ipv4_connection_point(const uint8_t* data_item, uint16_t item_len)
{
/*printf("check_ipv4_connection_point...\n"); */
	if (item_len != 5 && item_len != 7)
	{
		printf("Incorrect length in IPv4 Connection Point data item: %u, expected 5 or 7\n",item_len);
		return DLEP_SC_INVALID_DATA;
	}
	else if (data_item[0] != 0 && data_item[0] != 1)
	{
		printf("Incorrect Use TLS indicator in IPv4 Connection Point data item: %u, expected 0 or 1\n",(unsigned int)data_item[0]);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_ipv6_connection_point(const uint8_t* data_item, uint16_t item_len)
{
	if (item_len != 17 && item_len != 19)
	{
		printf("Incorrect length in IPv6 Connection Point data item: %u, expected 17 or 19\n",item_len);
		return DLEP_SC_INVALID_DATA;
	}
	else if (data_item[0] != 0 && data_item[0] != 1)
	{
		printf("Incorrect Use TLS indicator in IPv6 Connection Point data item: %u, expected 0 or 1\n",(unsigned int)data_item[0]);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_ipv4_address(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,5,"IPv4 Address");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] != 0 && data_item[0] != 1)
		{
			printf("Incorrect add/drop indicator in IPv4 Address data item: %u, expected 0 or 1\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv6_address(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,17,"IPv6 Address");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] != 0 && data_item[0] != 1)
		{
			printf("Incorrect add/drop indicator in IPv6 Address data item: %u, expected 0 or 1\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv4_attached_subnet(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,5,"IPv4 Attached Subnet");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[4] < 1 || data_item[4] > 32)
		{
			printf("Incorrect prefix in IPv4 Address data item: %u, expected 1-32\n",(unsigned int)data_item[4]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv6_attached_subnet(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,17,"IPv6 Attached Subnet");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[16] > 128)
		{
			printf("Incorrect prefix in IPv4 Address data item: %u, expected 1-128\n",(unsigned int)data_item[16]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_mdrr(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,8,"Maximum Data Rate (Receive)");
}

static enum dlep_status_code check_mdrt(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,8,"Maximum Data Rate (Transmit)");
}

static enum dlep_status_code check_cdrr(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,8,"Current Data Rate (Receive)");
}

static enum dlep_status_code check_cdrt(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,8,"Current Data Rate (Transmit)");
}

static enum dlep_status_code check_latency(const uint8_t* data_item, uint16_t item_len)
{
	/*return check_length(item_len,4,"Latency");*/
	return check_length(item_len,8,"Latency");
}

static enum dlep_status_code check_resr(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,1,"Resources (Receive)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] > 100)
		{
			printf("Incorrect value in Resources (Receive) data item: %u, expected 0 to 100%%\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rest(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,1,"Resources (Transmit)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] > 100)
		{
			printf("Incorrect value in Resources (Transmit) data item: %u, expected 0 to 100%%\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rlqr(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,1,"Relative Link Quality (Receive)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] > 100)
		{
			printf("Incorrect value in Relative Link Quality (Receive) data item: %u, expected 1 to 100\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rlqt(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = check_length(item_len,1,"Relative Link Quality (Transmit)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (data_item[0] > 100)
		{
			printf("Incorrect value in Relative Link Quality (Transmit) data item: %u, expected 1 to 100\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_extensions_supported(const uint8_t* data_item, uint16_t item_len)
{
	enum dlep_status_code sc = DLEP_SC_SUCCESS;
	if (item_len == 0)
	{
		printf("Warning: Empty DLEP Extensions Supported data item.\n");
	}
	else
	{
		size_t i = 0;
		for (; i < item_len; i += 2)
		{
			uint16_t ext_id = read_uint16(data_item + i);
			switch (ext_id)
			{
			default:
				printf("Unknown DLEP extension %u\n",ext_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}
	}
	return sc;
}

static enum dlep_status_code check_status(const uint8_t* data_item, uint16_t item_len, int* terminate)
{
	/* enum dlep_status_code sc = check_length(item_len,1,"Status data item"); */
	enum dlep_status_code sc = check_length(item_len,item_len,"Status data item"); /* variable length, so can't be determined if correct or not */

	*terminate = 1;

	if (sc == DLEP_SC_SUCCESS)
	{
		switch (data_item[0])
		{
		case DLEP_SC_SUCCESS:
		case DLEP_SC_UNKNOWN_MESSAGE:
		case DLEP_SC_UNEXPECTED_MESSAGE:
		case DLEP_SC_INVALID_DATA:
		case DLEP_SC_INVALID_DEST:
			break;

		case DLEP_SC_NOT_INTERESTED:
		case DLEP_SC_REQUEST_DENIED:
		case DLEP_SC_TIMEDOUT:
			*terminate = 0;
			break;

		default:
			printf("Unknown Status Code %u in Status data item.\n",(unsigned int)data_item[0]);
			sc = DLEP_SC_INVALID_DATA;
			break;
		}
	}
	return sc;
}

static enum dlep_status_code check_mac_address(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,6,"MAC Address data item");
}

static enum dlep_status_code check_lcr_timer(const uint8_t* data_item, uint16_t item_len)
{
	return check_length(item_len,1,"Link Characteristics Response Timer data item");
}

static enum dlep_status_code check_message(const uint8_t* msg, size_t len, unsigned int id, const char* name)
{
	enum dlep_status_code sc = DLEP_SC_SUCCESS;
	if (len < 4)
	{
		printf("Packet too short for %s message: %lu bytes\n",name,(unsigned long)len);
		sc = DLEP_SC_INVALID_DATA;
	}
	else
	{
		enum dlep_message msg_id = read_uint16(msg);
		if (msg_id != id)
		{
			printf("%s message expected, but message %u received\n",name,msg_id);
			sc = DLEP_SC_UNEXPECTED_MESSAGE;
		}
		else
		{
			uint16_t reported_len = read_uint16(msg+2);
			if (reported_len != len - 4)
			{
				printf("%s message length %u + header length does not match received packet length %lu\n",name,reported_len,(unsigned long)len);
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

static void printf_unexpected_data_item(const char* name, enum dlep_data_item item_id)
{
	const char* data_item_text = NULL;
	switch (item_id)
	{
	case DLEP_STATUS_DATA_ITEM:
		data_item_text = "Status";
		break;

	case DLEP_IPV4_CONN_POINT_DATA_ITEM:
		data_item_text = "IPv4 Connection Point";
		break;

	case DLEP_IPV6_CONN_POINT_DATA_ITEM:
		data_item_text = "IPv6 Connection Point";
		break;

	case DLEP_PEER_TYPE_DATA_ITEM:
		data_item_text = "Peer Type";
		break;

	case DLEP_HEARTBEAT_INTERVAL_DATA_ITEM:
		data_item_text = "Heartbeat Interval";
		break;

	case DLEP_EXTS_SUPP_DATA_ITEM:
		data_item_text = "Extensions Supported";
		break;

	case DLEP_MAC_ADDRESS_DATA_ITEM:
		data_item_text = "MAC Address";
		break;

	case DLEP_IPV4_ADDRESS_DATA_ITEM:
		data_item_text = "IPv4 Address";
		break;

	case DLEP_IPV6_ADDRESS_DATA_ITEM:
		data_item_text = "IPv6 Address";
		break;

	case DLEP_IPV4_ATT_SUBNET_DATA_ITEM:
		data_item_text = "IPv4 Attached Subnet";
		break;

	case DLEP_IPV6_ATT_SUBNET_DATA_ITEM:
		data_item_text = "IPv6 Attached Subnet";
		break;

	case DLEP_MDRR_DATA_ITEM:
		data_item_text = "Maximum Data Rate (Receive) (MDRR)";
		break;

	case DLEP_MDRT_DATA_ITEM:
		data_item_text = "Maximum Data Rate (Transmit) (MDRT)";
		break;

	case DLEP_CDRR_DATA_ITEM:
		data_item_text = "Current Data Rate (Receive) (CDRR)";
		break;

	case DLEP_CDRT_DATA_ITEM:
		data_item_text = "Current Data Rate (Transmit) (CDRT)";
		break;

	case DLEP_LATENCY_DATA_ITEM:
		data_item_text = "Latency";
		break;

	case DLEP_RESR_DATA_ITEM:
		data_item_text = "Resources (Receive) (RESR)";
		break;

	case DLEP_REST_DATA_ITEM:
		data_item_text = "Resources (Transmit) (REST)";
		break;

	case DLEP_RLQR_DATA_ITEM:
		data_item_text = "Relative Link Quality (Receive) (RLQR)";
		break;

	case DLEP_RLQT_DATA_ITEM:
		data_item_text = "Relative Link Quality (Transmit) (RLQT)";
		break;

	case DLEP_LINK_CHAR_RESP_TIMER_DATA_ITEM:
		data_item_text = "Link Characteristics Response Timer";
		break;
        default:
             break;
	}

	if (data_item_text)
		printf("Unexpected %s data item in %s message\n",data_item_text,name);
	else
		printf("Unexpected data item %u in %s message\n",item_id,name);
}

enum dlep_status_code check_peer_offer_signal(const uint8_t* msg, size_t len)
{
	/* Validate the signal */
	enum dlep_status_code sc = DLEP_SC_SUCCESS;
	if (len < 8)
	{
		printf("Packet too short for Peer Offer signal: %u bytes\n",(unsigned int)len);
		sc = DLEP_SC_INVALID_DATA;
	}
	else if (memcmp(msg,"DLEP",4) != 0)
	{
		printf("DLEP signal expected, but something else received, check for 'DLEP' in packet header\n");
		sc = DLEP_SC_INVALID_DATA;
	}
	else
	{
		uint16_t id = read_uint16(msg+4);
		if (id != DLEP_PEER_OFFER)
		{
			printf("Peer Offer signal expected, but signal %u received\n",id);
			sc = DLEP_SC_INVALID_DATA;
		}
		else
		{
			uint16_t reported_len = read_uint16(msg+6);
			if (reported_len + 8 != len)
			{
				printf("Peer Offer signal length %u + header length does not match received packet length %u\n",reported_len,(unsigned int)len);
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}

	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_ip_conn_pt = 0;
		int seen_peer_type = 0;
		//int seen_version = 0;

		/* Check for mandatory data items */
		/*const uint8_t* data_item = msg + 4; */
		const uint8_t* data_item = msg + 8;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
		{
			/* Octets 0 and 1 are the data item type */
			enum dlep_data_item item_id = read_uint16(data_item);

			/* Octets 2 and 3 are the data item length */
			uint16_t item_len = read_uint16(data_item + 2);

			/* Increment data_item to point to the data */
			data_item += 4;
			/*data_item += 1; flag field */

			switch (item_id)
			{
			case DLEP_PEER_TYPE_DATA_ITEM:
				if (seen_peer_type)
				{
					printf("Multiple Peer Type data items in Peer Offer signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_peer_type(data_item,item_len);
					seen_peer_type = 1;
				}
				break;

			case DLEP_IPV4_CONN_POINT_DATA_ITEM:
				sc = check_ipv4_connection_point(data_item,item_len);
				seen_ip_conn_pt = 1;
				break;

			case DLEP_IPV6_CONN_POINT_DATA_ITEM:
				sc = check_ipv6_connection_point(data_item,item_len);
				seen_ip_conn_pt = 1;
				break;

			default:
				printf_unexpected_data_item("Peer Offer",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (data_item != msg + len)
			{
	     			printf("Signal length does not equal sum of data item lengths in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
/*
			else if (!seen_version)
			{
				printf("Missing mandatory DLEP Version data item in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
*/
			else if (!seen_ip_conn_pt)
			{
				printf("Missing IPv4 or IPv6 connection point data item in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_session_init_resp_message(const uint8_t* msg, size_t len, int* terminate)
{
	/* Validate the message */
	enum dlep_status_code sc = check_message(msg,len,DLEP_SESSION_INIT_RESP,"Session Initialization Response");

	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_heartbeat = 0;
		int seen_mdrr = 0;
		int seen_mdrt = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;
		int seen_resr = 0;
		int seen_rest = 0;
		int seen_rlqr = 0;
		int seen_rlqt = 0;
		int seen_status = 0;
		int seen_peer_type = 0;
		//int seen_version = 0;
		int seen_exts_supported = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;

		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				if (seen_heartbeat)
				{
					printf("Multiple Heartbeat Interval data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_heartbeat_interval(data_item,item_len);
					seen_heartbeat = 1;
				}
				break;

			case DLEP_MDRR_DATA_ITEM:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(data_item,item_len);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_DATA_ITEM:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(data_item,item_len);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_DATA_ITEM:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(data_item,item_len);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_DATA_ITEM:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(data_item,item_len);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_DATA_ITEM:
				if (seen_latency)
				{
					printf("Multiple Latency data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(data_item,item_len);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_DATA_ITEM:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(data_item,item_len);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_DATA_ITEM:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(data_item,item_len);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_DATA_ITEM:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(data_item,item_len);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_DATA_ITEM:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(data_item,item_len);
					seen_rlqt = 1;
				}
				break;

			case DLEP_STATUS_DATA_ITEM:
				if (seen_status)
				{
					printf("Multiple Status data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_status(data_item,item_len,terminate);
					seen_status = 1;
				}
				break;

			case DLEP_PEER_TYPE_DATA_ITEM:
				if (seen_peer_type)
				{
					printf("Multiple Peer Type data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_peer_type(data_item,item_len);
					seen_peer_type = 1;
				}
				break;

			case DLEP_EXTS_SUPP_DATA_ITEM:
				if (seen_exts_supported)
				{
					printf("Multiple Extensions Supported data items in Session Initialization Response message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_extensions_supported(data_item,item_len);
					seen_exts_supported = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Session Initialization Response",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (data_item != msg + len)
			{
				printf("Signal length does not equal sum of data item lengths in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
/*
			else if (!seen_version)
			{
				printf("Missing mandatory DLEP Version data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
*/
			else if (!seen_heartbeat)
			{
				printf("Missing mandatory Heartbeat Interval data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_mdrr)
			{
				printf("Missing mandatory Maximum Data Rate (Receive) data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_mdrt)
			{
				printf("Missing mandatory Maximum Data Rate (Transmit) data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_cdrr)
			{
				printf("Missing mandatory Current Data Rate (Receive) data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_cdrt)
			{
				printf("Missing mandatory Current Data Rate (Transmit) data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_latency)
			{
				printf("Missing mandatory Latency data item in Session Initialization Response message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_heartbeat_message(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_PEER_HEARTBEAT,"Session Heartbeat");
	if (sc == DLEP_SC_SUCCESS)
	{
		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
		{
			/* Octets 0 and 1 are the data item type */
			enum dlep_data_item item_id = read_uint16(data_item);

			/* Octets 2 and 3 are the data item length */
			uint16_t item_len = read_uint16(data_item + 2);

			/* Increment data_item to point to the data */
			data_item += 4;

			switch (item_id)
			{
			default:
				printf_unexpected_data_item("Session Heartbeat",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}
	}
	return sc;
}

enum dlep_status_code check_session_term_message(const uint8_t* msg, size_t len, int* terminate)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_SESSION_TERM,"Session Termination");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_status = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
		{
			/* Octets 0 and 1 are the data item type */
			enum dlep_data_item item_id = read_uint16(data_item);

			/* Octets 2 and 3 are the data item length */
			uint16_t item_len = read_uint16(data_item + 2);

			/* Increment data_item to point to the data */
			data_item += 4;

			switch (item_id)
			{
			case DLEP_STATUS_DATA_ITEM:
				if (seen_status)
				{
					printf("Multiple Status data items in Session Termination message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_status(data_item,item_len,terminate);
					seen_status = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Session Termination",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_status)
			{
				printf("Missing mandatory Status data item in Session Termination message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_session_update_message(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_SESSION_UPDATE,"Session Update");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mdrr = 0;
		int seen_mdrt = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;
		int seen_resr = 0;
		int seen_rest = 0;
		int seen_rlqr = 0;
		int seen_rlqt = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				sc = check_ipv4_address(data_item,item_len);
				break;

			case DLEP_IPV6_ADDRESS_DATA_ITEM:
				sc = check_ipv6_address(data_item,item_len);
				break;

			case DLEP_MDRR_DATA_ITEM:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(data_item,item_len);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_DATA_ITEM:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(data_item,item_len);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_DATA_ITEM:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(data_item,item_len);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_DATA_ITEM:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(data_item,item_len);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_DATA_ITEM:
				if (seen_latency)
				{
					printf("Multiple Latency data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(data_item,item_len);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_DATA_ITEM:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(data_item,item_len);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_DATA_ITEM:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(data_item,item_len);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_DATA_ITEM:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(data_item,item_len);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_DATA_ITEM:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) data items in Session Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(data_item,item_len);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Session Update",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}
	}
	return sc;
}

enum dlep_status_code check_destination_up_message(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_DEST_UP,"Destination Up");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;
		int seen_mdrr = 0;
		int seen_mdrt = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;
		int seen_resr = 0;
		int seen_rest = 0;
		int seen_rlqr = 0;
		int seen_rlqt = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				if (seen_mac)
				{
					printf("Multiple MAC Address data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(data_item,item_len);
					seen_mac = 1;
				}
				break;

			case DLEP_IPV4_ADDRESS_DATA_ITEM:
				sc = check_ipv4_address(data_item,item_len);
				break;

			case DLEP_IPV6_ADDRESS_DATA_ITEM:
				sc = check_ipv6_address(data_item,item_len);
				break;

			case DLEP_IPV4_ATT_SUBNET_DATA_ITEM:
				sc = check_ipv4_attached_subnet(data_item,item_len);
				break;

			case DLEP_IPV6_ATT_SUBNET_DATA_ITEM:
				sc = check_ipv6_attached_subnet(data_item,item_len);
				break;

			case DLEP_MDRR_DATA_ITEM:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(data_item,item_len);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_DATA_ITEM:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(data_item,item_len);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_DATA_ITEM:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(data_item,item_len);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_DATA_ITEM:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(data_item,item_len);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_DATA_ITEM:
				if (seen_latency)
				{
					printf("Multiple Latency data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(data_item,item_len);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_DATA_ITEM:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(data_item,item_len);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_DATA_ITEM:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(data_item,item_len);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_DATA_ITEM:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(data_item,item_len);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_DATA_ITEM:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) data items in Destination Up message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(data_item,item_len);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Destination Up",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address data item in Destination Up message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_destination_update_message(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_DEST_UPDATE,"Destination Update");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;
		int seen_mdrr = 0;
		int seen_mdrt = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;
		int seen_resr = 0;
		int seen_rest = 0;
		int seen_rlqr = 0;
		int seen_rlqt = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				if (seen_mac)
				{
					printf("Multiple MAC Address data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(data_item,item_len);
					seen_mac = 1;
				}
				break;

			case DLEP_IPV4_ADDRESS_DATA_ITEM:
				sc = check_ipv4_address(data_item,item_len);
				break;

			case DLEP_IPV6_ADDRESS_DATA_ITEM:
				sc = check_ipv6_address(data_item,item_len);
				break;

			case DLEP_IPV4_ATT_SUBNET_DATA_ITEM:
				sc = check_ipv4_attached_subnet(data_item,item_len);
				break;

			case DLEP_IPV6_ATT_SUBNET_DATA_ITEM:
				sc = check_ipv6_attached_subnet(data_item,item_len);
				break;

			case DLEP_MDRR_DATA_ITEM:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(data_item,item_len);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_DATA_ITEM:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(data_item,item_len);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_DATA_ITEM:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(data_item,item_len);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_DATA_ITEM:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(data_item,item_len);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_DATA_ITEM:
				if (seen_latency)
				{
					printf("Multiple Latency data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(data_item,item_len);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_DATA_ITEM:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(data_item,item_len);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_DATA_ITEM:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(data_item,item_len);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_DATA_ITEM:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(data_item,item_len);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_DATA_ITEM:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) data items in Destination Update message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(data_item,item_len);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Destination Update",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address data item in Destination Update message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_destination_down_message(const uint8_t* msg, size_t len, const uint8_t** mac)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_DEST_DOWN,"Destination Down");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				if (seen_mac)
				{
					printf("Multiple MAC Address data items in Destination Down message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(data_item,item_len);
					if (sc == DLEP_SC_SUCCESS)
						*mac = data_item;
					seen_mac = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Destination Down",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address data item in Destination Down message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_link_char_request_message(const uint8_t* msg, size_t len, const uint8_t** mac)
{
	enum dlep_status_code sc = check_message(msg,len,DLEP_LINK_CHAR_REQ,"Link Characteristics Request");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;
		int seen_timer = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;

		/* Check for mandatory data items */
		const uint8_t* data_item = msg + 4;
		while (data_item < msg + len && sc == DLEP_SC_SUCCESS)
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
				if (seen_mac)
				{
					printf("Multiple MAC Address data items in Link Characteristics Request message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(data_item,item_len);
					if (sc == DLEP_SC_SUCCESS)
						*mac = data_item;
					seen_mac = 1;
				}
				break;

			case DLEP_LINK_CHAR_RESP_TIMER_DATA_ITEM:
				if (seen_timer)
				{
					printf("Multiple Link Characteristics Response Timer data items in Link Characteristics Request message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_lcr_timer(data_item,item_len);
					seen_timer = 1;
				}
				break;

			case DLEP_CDRR_DATA_ITEM:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) data items in Link Characteristics Request message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(data_item,item_len);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_DATA_ITEM:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) data items in Link Characteristics Request message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(data_item,item_len);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_DATA_ITEM:
				if (seen_latency)
				{
					printf("Multiple Latency data items in Link Characteristics Request message\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(data_item,item_len);
					seen_latency = 1;
				}
				break;

			default:
				printf_unexpected_data_item("Link Characteristics Request",item_id);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}

			/* Increment data_item to point to the next data item */
			data_item += item_len;
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address data item in Link Characteristics Request message\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}
