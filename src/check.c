/*

Copyright (c) 2014 Airbus DS Limited

*/

#include <stdio.h>
#include <arpa/inet.h>

#include "./dlep_iana.h"

static enum dlep_status_code check_length(const uint8_t* tlv, unsigned int len, const char* name)
{
	if (tlv[1] != len)
	{
		printf("Incorrect length in %s TLV: %u, expected %u\n",name,(unsigned int)tlv[1],len);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_version(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,4,"DLEP Version");
	if (sc == DLEP_SC_SUCCESS)
	{
		uint16_t major = get_uint16(tlv+2);
		uint16_t minor = get_uint16(tlv+4);

		if (major == 1 || minor == 0)
		{
			printf("DLEP version 1.0 advertised, but this version does not know the IANA registered numbers, use 0.14\n");
			sc = DLEP_SC_INVALID_DATA;
		}
		else if (major != 0 || minor != 14)
		{
			printf("Only DLEP version 0.14 supported, discovered %u.%u\n",major,minor);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_peer_type(const uint8_t* tlv)
{
	size_t i = 0;
	for (; i < tlv[1]; ++i)
	{
		/* Check for NUL */
		if (tlv[i + 2] == 0)
		{
			printf("Warning: Suspicious NUL character in peer type string\n");
		}

		/* TODO: One should check for valid UTF8 characters here */
	}

	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_heartbeat_interval(const uint8_t* tlv)
{
	return check_length(tlv,2,"Heartbeat Interval");
}

static enum dlep_status_code check_ipv4_connection_point(const uint8_t* tlv)
{
	if (tlv[1] != 4 && tlv[1] != 6)
	{
		printf("Incorrect length in IPv4 Connection Point TLV: %u, expected 4 or 6\n",(unsigned int)tlv[1]);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_ipv6_connection_point(const uint8_t* tlv)
{
	if (tlv[1] != 16 && tlv[1] != 18)
	{
		printf("Incorrect length in IPv6 Connection Point TLV: %u, expected 16 or 18\n",(unsigned int)tlv[1]);
		return DLEP_SC_INVALID_DATA;
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_ipv4_address(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,5,"IPv4 Address");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[2] != 1 && tlv[2] != 2)
		{
			printf("Incorrect add/drop indicator in IPv4 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv6_address(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,17,"IPv6 Address");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[2] != 1 && tlv[2] != 2)
		{
			printf("Incorrect add/drop indicator in IPv6 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv4_attached_subnet(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,5,"IPv4 Attached Subnet");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[6] < 1 || tlv[6] > 32)
		{
			printf("Incorrect prefix in IPv4 Address TLV: %u, expected 1-32\n",(unsigned int)tlv[6]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_ipv6_attached_subnet(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,17,"IPv6 Attached Subnet");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[18] > 128)
		{
			printf("Incorrect prefix in IPv4 Address TLV: %u, expected 1-128\n",(unsigned int)tlv[18]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_mdrr(const uint8_t* tlv)
{
	return check_length(tlv,8,"Maximum Data Rate (Receive)");
}

static enum dlep_status_code check_mdrt(const uint8_t* tlv)
{
	return check_length(tlv,8,"Maximum Data Rate (Transmit)");
}

static enum dlep_status_code check_cdrr(const uint8_t* tlv)
{
	return check_length(tlv,8,"Current Data Rate (Receive)");
}

static enum dlep_status_code check_cdrt(const uint8_t* tlv)
{
	return check_length(tlv,8,"Current Data Rate (Transmit)");
}

static enum dlep_status_code check_latency(const uint8_t* tlv)
{
	return check_length(tlv,4,"Latency");
}

static enum dlep_status_code check_resr(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,1,"Resources (Receive)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[1] > 100)
		{
			printf("Incorrect value in Resources (Receive) TLV: %u, expected 0 to 100%%\n",(unsigned int)tlv[1]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rest(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,1,"Resources (Transmit)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[1] > 100)
		{
			printf("Incorrect value in Resources (Transmit) TLV: %u, expected 0 to 100%%\n",(unsigned int)tlv[1]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rlqr(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,1,"Relative Link Quality (Receive)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[1] > 100)
		{
			printf("Incorrect value in Relative Link Quality (Receive) TLV: %u, expected 1 to 100\n",(unsigned int)tlv[1]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_rlqt(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,1,"Relative Link Quality (Transmit)");
	if (sc == DLEP_SC_SUCCESS)
	{
		if (tlv[1] > 100)
		{
			printf("Incorrect value in Relative Link Quality (Transmit) TLV: %u, expected 1 to 100\n",(unsigned int)tlv[1]);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static enum dlep_status_code check_extensions_supported(const uint8_t* tlv)
{
	enum dlep_status_code sc = DLEP_SC_SUCCESS;
	if (tlv[1] == 0)
	{
		printf("Warning: Empty DLEP Extensions Supported TLV.\n");
	}
	else
	{
		size_t i = 0;
		for (; i < tlv[1]; ++i)
		{
			switch (tlv[2 + i])
			{
			default:
				printf("Unknown DLEP extension %u\n",tlv[2 + i]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}
	}
	return sc;
}

static enum dlep_status_code check_experimental_definition(const uint8_t* tlv)
{
	if (tlv[1] == 0)
	{
		printf("Warning: Empty DLEP Experimental Definition TLV.\n");
	}
	else
	{
		size_t i = 0;
		for (; i < tlv[1]; ++i)
		{
			/* Check for NUL */
			if (tlv[2 + i] == 0)
			{
				printf("Warning: Suspicious NUL character in experimental definition type string\n");
			}

			/* TODO: One should check for valid UTF8 characters here */
		}
	}
	return DLEP_SC_SUCCESS;
}

static enum dlep_status_code check_status(const uint8_t* tlv)
{
	enum dlep_status_code sc = check_length(tlv,1,"Status TLV");
	if (sc == DLEP_SC_SUCCESS)
	{
		switch (tlv[2])
		{
		case DLEP_SC_SUCCESS:
		case DLEP_SC_UNKNOWN_SIGNAL:
		case DLEP_SC_INVALID_DATA:
		case DLEP_SC_UNEXPECTED_SIGNAL:
		case DLEP_SC_REQUEST_DENIED:
		case DLEP_SC_TIMEDOUT:
		case DLEP_SC_INVALID_DEST:
			break;

		default:
			printf("Unknown Status Code in Status TLV.\n");
			sc = DLEP_SC_INVALID_DATA;
			break;
		}
	}
	return sc;
}

static enum dlep_status_code check_mac_address(const uint8_t* tlv)
{
	return check_length(tlv,6,"MAC Address TLV");
}

static enum dlep_status_code check_lcr_timer(const uint8_t* tlv)
{
	return check_length(tlv,1,"Link Characteristics ACK Timer TLV");
}

static enum dlep_status_code check_signal(const uint8_t* msg, size_t len, unsigned int id, const char* name)
{
	enum dlep_status_code sc = DLEP_SC_SUCCESS;
	if (len < 3)
	{
		printf("Packet too short for %s signal: %u bytes\n",name,(unsigned int)len);
		sc = DLEP_SC_INVALID_DATA;
	}
	else if (msg[0] != id)
	{
		printf("%s signal expected, but signal %u received\n",name,msg[0]);
		sc = DLEP_SC_UNEXPECTED_SIGNAL;
	}
	else
	{
		uint16_t reported_len = get_uint16(msg+1);
		if (reported_len != len)
		{
			printf("%s signal length %u does not match received packet length %u\n",name,reported_len,(unsigned int)len);
			sc = DLEP_SC_INVALID_DATA;
		}
	}
	return sc;
}

static void printf_unexpected_tlv(const char* name, uint8_t tlv)
{
	const char* tlv_text = NULL;
	switch (tlv)
	{
	case DLEP_VERSION_TLV:
		tlv_text = "DLEP Version";
		break;

	case DLEP_STATUS_TLV:
		tlv_text = "Status";
		break;

	case DLEP_IPV4_CONN_POINT_TLV:
		tlv_text = "IPv4 Connection Point";
		break;

	case DLEP_IPV6_CONN_POINT_TLV:
		tlv_text = "IPv6 Connection Point";
		break;

	case DLEP_PEER_TYPE_TLV:
		tlv_text = "Peer Type";
		break;

	case DLEP_PEER_HEARTBEAT_INTERVAL_TLV:
		tlv_text = "Heartbeat Interval";
		break;

	case DLEP_EXTS_SUPP_TLV:
		tlv_text = "Extensions Supported";
		break;

	case DLEP_EXP_DEFNS_TLV:
		tlv_text = "Experimental Definition";
		break;

	case DLEP_MAC_ADDRESS_TLV:
		tlv_text = "MAC Address";
		break;

	case DLEP_IPV4_ADDRESS_TLV:
		tlv_text = "IPv4 Address";
		break;

	case DLEP_IPV6_ADDRESS_TLV:
		tlv_text = "IPv6 Address";
		break;

	case DLEP_IPV4_ATT_SUBNET_TLV:
		tlv_text = "IPv4 Attached Subnet";
		break;

	case DLEP_IPV6_ATT_SUBNET_TLV:
		tlv_text = "IPv6 Attached Subnet";
		break;

	case DLEP_MDRR_TLV:
		tlv_text = "Maximum Data Rate (Receive) (MDRR)";
		break;

	case DLEP_MDRT_TLV:
		tlv_text = "Maximum Data Rate (Transmit) (MDRT)";
		break;

	case DLEP_CDRR_TLV:
		tlv_text = "Current Data Rate (Receive) (CDRR)";
		break;

	case DLEP_CDRT_TLV:
		tlv_text = "Current Data Rate (Transmit) (CDRT)";
		break;

	case DLEP_LATENCY_TLV:
		tlv_text = "Latency";
		break;

	case DLEP_RESR_TLV:
		tlv_text = "Resources (Receive) (RESR)";
		break;

	case DLEP_REST_TLV:
		tlv_text = "Resources (Transmit) (REST)";
		break;

	case DLEP_RLQR_TLV:
		tlv_text = "Relative Link Quality (Receive) (RLQR)";
		break;

	case DLEP_RLQT_TLV:
		tlv_text = "Relative Link Quality (Transmit) (RLQT)";
		break;

	case DLEP_LINK_CHAR_ACK_TIMER_TLV:
		tlv_text = "Link Characteristics ACK Timer";
		break;
	}

	if (tlv_text)
		printf("Unexpected %s TLV in %s signal\n",tlv_text,name);
	else
		printf("Unexpected TLV %u in %s signal\n",(unsigned int)tlv,name);
}

enum dlep_status_code check_peer_offer_signal(const uint8_t* msg, size_t len)
{
	/* Validate the signal */
	enum dlep_status_code sc = check_signal(msg,len,DLEP_PEER_OFFER,"Peer Offer");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_ip_conn_pt = 0;
		int seen_peer_type = 0;
		int seen_version = 0;

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_VERSION_TLV:
				if (seen_version)
				{
					printf("Multiple DLEP version TLVs in Peer Offer signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_version(tlv);
					seen_version = 1;
				}
				break;

			case DLEP_PEER_TYPE_TLV:
				if (seen_peer_type)
				{
					printf("Multiple Peer Type TLVs in Peer Offer signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_peer_type(tlv);
					seen_peer_type = 1;
				}
				break;

			case DLEP_IPV4_CONN_POINT_TLV:
				sc = check_ipv4_connection_point(tlv);
				seen_ip_conn_pt = 1;
				break;

			case DLEP_IPV6_CONN_POINT_TLV:
				sc = check_ipv6_connection_point(tlv);
				seen_ip_conn_pt = 1;
				break;

			default:
				printf_unexpected_tlv("Peer Offer",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (tlv != msg + len)
			{
				printf("Signal length does not equal sum of TLV lengths in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_version)
			{
				printf("Missing mandatory DLEP Version TLV in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_ip_conn_pt)
			{
				printf("Missing IPv4 or IPv6 connection point TLV in Peer Offer signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_peer_init_ack_signal(const uint8_t* msg, size_t len)
{
	/* Validate the signal */
	enum dlep_status_code sc = check_signal(msg,len,DLEP_PEER_INIT_ACK,"Peer Initialization ACK");
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
		int seen_version = 0;
		int seen_exts_supported = 0;

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_VERSION_TLV:
				if (seen_version)
				{
					printf("Multiple DLEP version TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_version(tlv);
					seen_version = 1;
				}
				break;

			case DLEP_PEER_HEARTBEAT_INTERVAL_TLV:
				if (seen_heartbeat)
				{
					printf("Multiple Heartbeat Interval TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_heartbeat_interval(tlv);
					seen_heartbeat = 1;
				}
				break;

			case DLEP_MDRR_TLV:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(tlv);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_TLV:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(tlv);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_TLV:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(tlv);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_TLV:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(tlv);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_TLV:
				if (seen_latency)
				{
					printf("Multiple Latency TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(tlv);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_TLV:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(tlv);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_TLV:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(tlv);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_TLV:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(tlv);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_TLV:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(tlv);
					seen_rlqt = 1;
				}
				break;

			case DLEP_STATUS_TLV:
				if (seen_status)
				{
					printf("Multiple Status TLVs in Peer Offer signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_status(tlv);
					seen_status = 1;
				}
				break;

			case DLEP_PEER_TYPE_TLV:
				if (seen_peer_type)
				{
					printf("Multiple Peer Type TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_peer_type(tlv);
					seen_peer_type = 1;
				}
				break;

			case DLEP_EXTS_SUPP_TLV:
				if (seen_exts_supported)
				{
					printf("Multiple Extensions Supported TLVs in Peer Initialization ACK signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_extensions_supported(tlv);
					seen_exts_supported = 1;
				}
				break;

			case DLEP_EXP_DEFNS_TLV:
				sc = check_experimental_definition(tlv);
				break;

			default:
				printf_unexpected_tlv("Peer Initialization ACK",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (tlv != msg + len)
			{
				printf("Signal length does not equal sum of TLV lengths in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_version)
			{
				printf("Missing mandatory DLEP Version TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_heartbeat)
			{
				printf("Missing mandatory Heartbeat Interval TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_mdrr)
			{
				printf("Missing mandatory Maximum Data Rate (Receive) TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_mdrt)
			{
				printf("Missing mandatory Maximum Data Rate (Transmit) TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_cdrr)
			{
				printf("Missing mandatory Current Data Rate (Receive) TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_cdrt)
			{
				printf("Missing mandatory Current Data Rate (Transmit) TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
			else if (!seen_latency)
			{
				printf("Missing mandatory Latency TLV in Peer Initialization ACK signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_heartbeat_signal(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_PEER_HEARTBEAT,"Peer Heartbeat");
	if (sc == DLEP_SC_SUCCESS)
	{
		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			default:
				printf_unexpected_tlv("Peer Heartbeat",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_peer_term_signal(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_PEER_TERM,"Peer Termination");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_status = 0;

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_STATUS_TLV:
				if (seen_status)
				{
					printf("Multiple Status TLVs in Peer Termination signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_status(tlv);
					seen_status = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Peer Termination",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_status)
			{
				printf("Missing mandatory Status TLV in Peer Termination signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_peer_update_signal(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_PEER_UPDATE,"Peer Update");
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

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg+3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_IPV4_ADDRESS_TLV:
				sc = check_ipv4_address(tlv);
				break;

			case DLEP_IPV6_ADDRESS_TLV:
				sc = check_ipv6_address(tlv);
				break;

			case DLEP_MDRR_TLV:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(tlv);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_TLV:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(tlv);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_TLV:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(tlv);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_TLV:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(tlv);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_TLV:
				if (seen_latency)
				{
					printf("Multiple Latency TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(tlv);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_TLV:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(tlv);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_TLV:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(tlv);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_TLV:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(tlv);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_TLV:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) TLVs in Peer Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(tlv);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Peer Update",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_destination_up_signal(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_DEST_UP,"Destination Up");
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

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg+3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_MAC_ADDRESS_TLV:
				if (seen_mac)
				{
					printf("Multiple MAC Address TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(tlv);
					seen_mac = 1;
				}
				break;

			case DLEP_IPV4_ADDRESS_TLV:
				sc = check_ipv4_address(tlv);
				break;

			case DLEP_IPV6_ADDRESS_TLV:
				sc = check_ipv6_address(tlv);
				break;

			case DLEP_IPV4_ATT_SUBNET_TLV:
				sc = check_ipv4_attached_subnet(tlv);
				break;

			case DLEP_IPV6_ATT_SUBNET_TLV:
				sc = check_ipv6_attached_subnet(tlv);
				break;

			case DLEP_MDRR_TLV:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(tlv);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_TLV:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(tlv);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_TLV:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(tlv);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_TLV:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(tlv);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_TLV:
				if (seen_latency)
				{
					printf("Multiple Latency TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(tlv);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_TLV:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(tlv);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_TLV:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(tlv);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_TLV:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(tlv);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_TLV:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) TLVs in Destination Up signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(tlv);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Destination Up",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address TLV in Destination Up signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_destination_update_signal(const uint8_t* msg, size_t len)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_DEST_UPDATE,"Destination Update");
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

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_MAC_ADDRESS_TLV:
				if (seen_mac)
				{
					printf("Multiple MAC Address TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(tlv);
					seen_mac = 1;
				}
				break;

			case DLEP_IPV4_ADDRESS_TLV:
				sc = check_ipv4_address(tlv);
				break;

			case DLEP_IPV6_ADDRESS_TLV:
				sc = check_ipv6_address(tlv);
				break;

			case DLEP_IPV4_ATT_SUBNET_TLV:
				sc = check_ipv4_attached_subnet(tlv);
				break;

			case DLEP_IPV6_ATT_SUBNET_TLV:
				sc = check_ipv6_attached_subnet(tlv);
				break;

			case DLEP_MDRR_TLV:
				if (seen_mdrr)
				{
					printf("Multiple Maximum Data Rate (Receive) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrr(tlv);
					seen_mdrr = 1;
				}
				break;

			case DLEP_MDRT_TLV:
				if (seen_mdrt)
				{
					printf("Multiple Maximum Data Rate (Transmit) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mdrt(tlv);
					seen_mdrt = 1;
				}
				break;

			case DLEP_CDRR_TLV:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(tlv);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_TLV:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(tlv);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_TLV:
				if (seen_latency)
				{
					printf("Multiple Latency TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(tlv);
					seen_latency = 1;
				}
				break;

			case DLEP_RESR_TLV:
				if (seen_resr)
				{
					printf("Multiple Resources (Receive) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_resr(tlv);
					seen_resr = 1;
				}
				break;

			case DLEP_REST_TLV:
				if (seen_rest)
				{
					printf("Multiple Resources (Transmit) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rest(tlv);
					seen_rest = 1;
				}
				break;

			case DLEP_RLQR_TLV:
				if (seen_rlqr)
				{
					printf("Multiple Relative Link Quality (Receive) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqr(tlv);
					seen_rlqr = 1;
				}
				break;

			case DLEP_RLQT_TLV:
				if (seen_rlqt)
				{
					printf("Multiple Relative Link Quality (Transmit) TLVs in Destination Update signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_rlqt(tlv);
					seen_rlqt = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Destination Update",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address TLV in Destination Update signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_destination_down_signal(const uint8_t* msg, size_t len, const uint8_t** mac)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_DEST_DOWN,"Destination Down");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_MAC_ADDRESS_TLV:
				if (seen_mac)
				{
					printf("Multiple MAC Address TLVs in Destination Down signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(tlv);
					if (sc == DLEP_SC_SUCCESS)
						*mac = tlv + 2;
					seen_mac = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Destination Down",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address TLV in Destination Down signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}

enum dlep_status_code check_link_char_request_signal(const uint8_t* msg, size_t len, const uint8_t** mac)
{
	enum dlep_status_code sc = check_signal(msg,len,DLEP_LINK_CHAR_REQ,"Link Characteristics Request");
	if (sc == DLEP_SC_SUCCESS)
	{
		int seen_mac = 0;
		int seen_timer = 0;
		int seen_cdrr = 0;
		int seen_cdrt = 0;
		int seen_latency = 0;

		/* Check for mandatory TLV's */
		const uint8_t* tlv;
		for (tlv = msg + 3; tlv < msg + len && sc == DLEP_SC_SUCCESS; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
		{
			/* Octet 0 is the TLV type */
			switch ((enum dlep_tlvs)tlv[0])
			{
			case DLEP_MAC_ADDRESS_TLV:
				if (seen_mac)
				{
					printf("Multiple MAC Address TLVs in Link Characteristics Request signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_mac_address(tlv);
					if (sc == DLEP_SC_SUCCESS)
						*mac = tlv + 2;
					seen_mac = 1;
				}
				break;

			case DLEP_LINK_CHAR_ACK_TIMER_TLV:
				if (seen_timer)
				{
					printf("Multiple Link Characteristics ACK Timer TLVs in Link Characteristics Request signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_lcr_timer(tlv);
					seen_timer = 1;
				}
				break;

			case DLEP_CDRR_TLV:
				if (seen_cdrr)
				{
					printf("Multiple Current Data Rate (Receive) TLVs in Link Characteristics Request signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrr(tlv);
					seen_cdrr = 1;
				}
				break;

			case DLEP_CDRT_TLV:
				if (seen_cdrt)
				{
					printf("Multiple Current Data Rate (Transmit) TLVs in Link Characteristics Request signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_cdrt(tlv);
					seen_cdrt = 1;
				}
				break;

			case DLEP_LATENCY_TLV:
				if (seen_latency)
				{
					printf("Multiple Latency TLVs in Link Characteristics Request signal\n");
					sc = DLEP_SC_INVALID_DATA;
				}
				else
				{
					sc = check_latency(tlv);
					seen_latency = 1;
				}
				break;

			default:
				printf_unexpected_tlv("Link Characteristics Request",tlv[0]);
				sc = DLEP_SC_INVALID_DATA;
				break;
			}
		}

		if (sc == DLEP_SC_SUCCESS)
		{
			if (!seen_mac)
			{
				printf("Missing mandatory MAC Address TLV in Link Characteristics Request signal\n");
				sc = DLEP_SC_INVALID_DATA;
			}
		}
	}
	return sc;
}
