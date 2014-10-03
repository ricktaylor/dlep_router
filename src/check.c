/*

Copyright (c) 2014 Airbus DS Limited

*/

#include <stdio.h>
#include <arpa/inet.h>

#include "./dlep_iana.h"

static int check_length(const char* tlv, unsigned int len, const char* name)
{
	if (tlv[1] != len)
	{
		printf("Incorrect length in %s TLV: %u, expected %u\n",name,(unsigned int)tlv[1],len);
		return 0;
	}
	return 1;
}

static int check_port(const char* tlv)
{
	return check_length(tlv,2,"DLEP Port");
}

static int check_version(const char* tlv)
{
	if (!check_length(tlv,4,"DLEP Version"))
		return 0;
	else
	{
		uint16_t major = get_uint16(tlv+2);
		uint16_t minor = get_uint16(tlv+2);

		if (major != 0 || minor != 7)
		{
			printf("Only DLEP version 0.7 supported, discovered %u.%u\n",major,minor);
			return 0;
		}

		return 1;
	}
}

static int check_peer_type(const char* tlv)
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

static int check_heartbeat_interval(const char* tlv)
{
	return check_length(tlv,2,"Heartbeat Interval");
}

static int check_ipv4_address(const char* tlv)
{
	if (!check_length(tlv,5,"IPv4 Address"))
		return 0;

	if (tlv[2] != 1 && tlv[2] != 2)
	{
		printf("Incorrect add/drop indicator in IPv4 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
		return 0;
	}

	return 1;
}

static int check_ipv6_address(const char* tlv)
{
	if (!check_length(tlv,17,"IPv6 Address"))
		return 0;

	if (tlv[2] != 1 && tlv[2] != 2)
	{
		printf("Incorrect add/drop indicator in IPv6 Address TLV: %u, expected 1 or 2\n",(unsigned int)tlv[2]);
		return 0;
	}

	return 1;
}

static int check_mdrr(const char* tlv)
{
	return check_length(tlv,8,"Maximum Data Rate (Receive)");
}

static int check_mdrt(const char* tlv)
{
	return check_length(tlv,8,"Maximum Data Rate (Transmit)");
}

static int check_cdrr(const char* tlv)
{
	return check_length(tlv,8,"Current Data Rate (Receive)");
}

static int check_cdrt(const char* tlv)
{
	return check_length(tlv,8,"Current Data Rate (Transmit)");
}

static int check_latency(const char* tlv)
{
	return check_length(tlv,4,"Latency");
}

static int check_resr(const char* tlv)
{
	if (!check_length(tlv,1,"Resources (Receive)"))
		return 0;

	if (tlv[1] > 100)
	{
		printf("Incorrect value in Resources (Receive) TLV: %u, expected 0 to 100%%\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

static int check_rest(const char* tlv)
{
	if (!check_length(tlv,1,"Resources (Transmit)"))
		return 0;

	if (tlv[1] > 100)
	{
		printf("Incorrect value in Resources (Transmit) TLV: %u, expected 0 to 100%%\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

static int check_rlqr(const char* tlv)
{
	if (!check_length(tlv,1,"Relative Link Quality (Receive)"))
		return 0;

	if (tlv[1] > 100)
	{
		printf("Incorrect value in Relative Link Quality (Receive) TLV: %u, expected 1 to 100\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

static int check_rlqt(const char* tlv)
{
	if (!check_length(tlv,1,"Relative Link Quality (Transmit)"))
		return 0;

	if (tlv[1] > 100)
	{
		printf("Incorrect value in Relative Link Quality (Transmit) TLV: %u, expected 1 to 100\n",(unsigned int)tlv[1]);
		return 0;
	}

	return 1;
}

static int check_vendor_extension(const char* tlv)
{
	if (tlv[1] < 4)
	{
		printf("Incorrect length in Vendor Extension TLV: %u, expected > 4\n",(unsigned int)tlv[1]);
		return 0;
	}

	/* Octet 2 is the OUI length, IEEE OUIs are 24-36 bits */
	if (tlv[2] < 3 || tlv[2] > 5)
	{
		printf("Unlikely OUI octet length in Vendor Extension TLV: %u, expected between 3 and 5\n",(unsigned int)tlv[1]);
		return 0;
	}

	/* Octet 7 is probably the Length of the payload TLV */
	if (tlv[7] != tlv[1] - 6 || tlv[7] <= 2)
	{
		printf("Vendor Extension TLV payload does not appear to be a TLV\n");
	}

	return 1;
}

static int check_status(const char* tlv)
{
	return check_length(tlv,1,"Status TLV");
}

static int check_optional_signals(const char* tlv)
{
	int ret = 1;
	if (tlv[1] < 2)
	{
		printf("Incorrect length in Optional Signals Supported TLV: %u, expected >=2 \n",(unsigned int)tlv[1]);
		ret = 0;
	}
	else
	{
		const char* end = tlv + tlv[1];
		tlv += 2;

		while (tlv < end)
		{
			switch ((enum dlep_signals)*tlv++)
			{
			/* Mandatory signals */
			case DLEP_PEER_DISCOVERY:
				printf("Unexpected mandatory Peer Discovery signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_PEER_OFFER:
				printf("Unexpected mandatory Peer Offer signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_PEER_INITIALIZATION:
				printf("Unexpected mandatory Peer Initialization signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_PEER_INITIALIZATION_ACK:
				printf("Unexpected mandatory Peer Initialization Ack signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_PEER_TERMINATION:
				printf("Unexpected mandatory Peer Termination signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_PEER_TERMINATION_ACK:
				printf("Unexpected mandatory Peer Initialization Ack signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_DESTINATION_UP:
				printf("Unexpected mandatory Destination Up signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_DESTINATION_UP_ACK:
				printf("Unexpected mandatory Destination Up Ack signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_DESTINATION_DOWN:
				printf("Unexpected mandatory Destination Down signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_DESTINATION_DOWN_ACK:
				printf("Unexpected mandatory Destination Down Ack signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_DESTINATION_UPDATE:
				printf("Unexpected mandatory Destination Update signal in Optional Signals Supported TLV\n");
				break;

			case DLEP_HEARTBEAT:
				printf("Unexpected mandatory Heartbeat signal in Optional Signals Supported TLV\n");
				break;

			/* Valid optional signals */
			case DLEP_PEER_UPDATE:
			case DLEP_PEER_UPDATE_ACK:
			case DLEP_LINK_CHARACTERISTICS_ACK:
				break;

			case DLEP_LINK_CHARACTERISTICS_REQUEST:
				printf("Unexpected Link Characteristics Request signal in Optional Signals Supported TLV\n");
				break;

			default:
				printf("Unrecognized signal %u in Optional Signals Supported TLV\n",*tlv);
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

static int check_optional_data_items(const char* tlv)
{
	int ret = 1;
	if (tlv[1] < 2)
	{
		printf("Incorrect length in Optional Data Items Supported TLV: %u, expected >=2 \n",(unsigned int)tlv[1]);
		ret = 0;
	}
	else
	{
		const char* end = tlv + tlv[1];
		tlv += 2;

		while (tlv < end)
		{
			switch ((enum dlep_tlvs)*tlv++)
			{
			/* Mandatory TLVs */
			case DLEP_PORT_TLV:
				printf("Unexpected mandatory DLEP Port TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_MAC_ADDRESS_TLV:
				printf("Unexpected mandatory MAC Address TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_MDRR_TLV:
				printf("Unexpected mandatory Maximum Data Rate (Receive) TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_MDRT_TLV:
				printf("Unexpected mandatory Maximum Data Rate (Transmit) TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_CDRR_TLV:
				printf("Unexpected mandatory Current Data Rate (Receive) TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_CDRT_TLV:
				printf("Unexpected mandatory Current Data Rate (Transmit) TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_LATENCY_TLV:
				printf("Unexpected mandatory Latency TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_STATUS_TLV:
				printf("Unexpected mandatory Status TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_HEARTBEAT_INTERVAL_TLV:
				printf("Unexpected mandatory Heartbeat Interval TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_OPTIONAL_SIGNALS_TLV:
				printf("Unexpected mandatory Optional Signals Supported TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_OPTIONAL_DATA_ITEMS_TLV:
				printf("Unexpected mandatory Optional Data Items Supported TLV in Optional Data Items Supported TLV\n");
				break;

			/* Optional, but non-Data Item TLVs */
			case DLEP_PEER_TYPE_TLV:
				printf("Unexpected Peer Type TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_IPV4_ADDRESS_TLV:
				printf("Unexpected IPv4 Address TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_IPV6_ADDRESS_TLV:
				printf("Unexpected IPv4 Address TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_LINK_CHAR_ACK_TIMER_TLV:
				printf("Unexpected Link Characteristics ACK Timer TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_CREDIT_WIN_STATUS_TLV:
				printf("Unexpected Credit Window Status TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_CREDIT_GRANT_REQ_TLV:
				printf("Unexpected Credit Grant Request TLV in Optional Data Items Supported TLV\n");
				break;

			case DLEP_CREDIT_REQUEST_TLV:
				printf("Unexpected Credit Request TLV in Optional Data Items Supported TLV\n");
				break;

			/* Optional and reportable */
			case DLEP_RESR_TLV:
			case DLEP_REST_TLV:
			case DLEP_RLQR_TLV:
			case DLEP_RLQT_TLV:
			case DLEP_VENDOR_EXTENSION_TLV:
				break;

			default:
				printf("Unrecognized TLV %u in Optional Data Items Supported TLV\n",*tlv);
				ret = 0;
				break;
			}
		}
	}

	return ret;
}

static int check_mac_address(const char* tlv)
{
	return check_length(tlv,6,"MAC Address TLV");
}

static int check_signal_length(const char* msg, size_t len, unsigned int id, const char* name)
{
	if (len < 3)
	{
		printf("Packet too short for %s signal: %u bytes\n",name,(unsigned int)len);
		return 0;
	}
	else if (msg[0] != id)
	{
		printf("%s signal expected, but signal %u received\n",name,msg[0]);
		return 0;
	}
	else
	{
		uint16_t reported_len = get_uint16(msg+1);
		if (reported_len != len)
		{
			printf("%s signal length %u does not match received packet length %u\n",name,reported_len,(unsigned int)len);
			return 0;
		}
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
	int seen_status = 0;
	int seen_version = 0;

	/* Validate the signal */
	if (!check_signal_length(msg,len,DLEP_PEER_OFFER,"Peer Offer"))
		return 0;

	/* Check for mandatory TLV's */
	for (tlv = msg + 3; tlv < msg + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_VERSION_TLV:
			if (seen_version)
			{
				printf("Multiple DLEP version TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_version(tlv))
				ret = 0;
			else
				seen_version = 1;
			break;

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

		case DLEP_STATUS_TLV:
			if (seen_status)
			{
				printf("Multiple Status TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_status(tlv))
				ret = 0;
			else
				seen_status = 1;
			break;

		default:
			printf("Unexpected TLV %u in Peer Offer signal\n",tlv[0]);
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
		printf("Missing mandatory DLEP Port TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (!seen_version)
	{
		printf("Missing mandatory DLEP Version TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (!seen_heartbeat)
	{
		printf("Missing mandatory Heartbeat Interval TLV in Peer Offer signal\n");
		ret = 0;
	}

	if (tlv != msg + len)
	{
		printf("Signal length does not equal sum of TLV lengths in Peer Offer signal\n");
		ret = 0;
	}

	return ret;
}

int check_peer_init_ack_signal(const char* msg, size_t len)
{
	int ret = 1;
	const char* tlv;
	int seen_heartbeat = 0;
	int seen_opt_signals = 0;
	int seen_opt_data_items = 0;
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

	/* Validate the signal */
	if (!check_signal_length(msg,len,DLEP_PEER_INITIALIZATION_ACK,"Peer Initialization ACK"))
		return 0;

	/* Check for mandatory TLV's */
	for (tlv = msg + 3; tlv < msg + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_VERSION_TLV:
			if (seen_version)
			{
				printf("Multiple DLEP version TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_version(tlv))
				ret = 0;
			else
				seen_version = 1;
			break;

		case DLEP_HEARTBEAT_INTERVAL_TLV:
			if (seen_heartbeat)
			{
				printf("Multiple Heartbeat Interval TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_heartbeat_interval(tlv))
				ret = 0;
			else
				seen_heartbeat = 1;
			break;

		case DLEP_MDRR_TLV:
			if (seen_mdrr)
			{
				printf("Multiple Maximum Data Rate (Receive) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_mdrr(tlv))
				ret = 0;
			else
				seen_mdrr = 1;
			break;

		case DLEP_MDRT_TLV:
			if (seen_mdrt)
			{
				printf("Multiple Maximum Data Rate (Transmit) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_mdrt(tlv))
				ret = 0;
			else
				seen_mdrt = 1;
			break;

		case DLEP_CDRR_TLV:
			if (seen_cdrr)
			{
				printf("Multiple Current Data Rate (Receive) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_cdrr(tlv))
				ret = 0;
			else
				seen_cdrr = 1;
			break;

		case DLEP_CDRT_TLV:
			if (seen_cdrt)
			{
				printf("Multiple Current Data Rate (Transmit) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_cdrt(tlv))
				ret = 0;
			else
				seen_cdrt = 1;
			break;

		case DLEP_LATENCY_TLV:
			if (seen_latency)
			{
				printf("Multiple Latency TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_latency(tlv))
				ret = 0;
			else
				seen_latency = 1;
			break;

		case DLEP_RESR_TLV:
			if (seen_resr)
			{
				printf("Multiple Resources (Receive) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_resr(tlv))
				ret = 0;
			else
				seen_resr = 1;
			break;

		case DLEP_REST_TLV:
			if (seen_rest)
			{
				printf("Multiple Resources (Transmit) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_rest(tlv))
				ret = 0;
			else
				seen_rest = 1;
			break;

		case DLEP_RLQR_TLV:
			if (seen_rlqr)
			{
				printf("Multiple Relative Link Quality (Receive) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_rlqr(tlv))
				ret = 0;
			else
				seen_rlqr = 1;
			break;

		case DLEP_RLQT_TLV:
			if (seen_rlqt)
			{
				printf("Multiple Relative Link Quality (Transmit) TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_rlqt(tlv))
				ret = 0;
			else
				seen_rlqt = 1;
			break;

		case DLEP_OPTIONAL_SIGNALS_TLV:
			if (seen_opt_signals)
			{
				printf("Multiple Optional Signals Supported TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_optional_signals(tlv))
				ret = 0;
			else
				seen_opt_signals = 1;
			break;

		case DLEP_OPTIONAL_DATA_ITEMS_TLV:
			if (seen_opt_data_items)
			{
				printf("Multiple Optional Data Items Supported TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_optional_data_items(tlv))
				ret = 0;
			else
				seen_opt_data_items = 1;
			break;

		case DLEP_STATUS_TLV:
			if (seen_status)
			{
				printf("Multiple Status TLVs in Peer Offer signal\n");
				ret = 0;
			}
			else if (!check_status(tlv))
				ret = 0;
			else
				seen_status = 1;
			break;

		case DLEP_PEER_TYPE_TLV:
			if (seen_peer_type)
			{
				printf("Multiple Peer Type TLVs in Peer Initialization ACK signal\n");
				ret = 0;
			}
			else if (!check_peer_type(tlv))
				ret = 0;
			else
				seen_peer_type = 1;
			break;

		case DLEP_VENDOR_EXTENSION_TLV:
			/* TODO: One should check for duplicates here */
			if (!check_vendor_extension(tlv))
				ret = 0;
			break;

		default:
			printf("Unexpected TLV %u in Peer Initialization ACK signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_heartbeat)
	{
		printf("Missing mandatory Heartbeat Interval TLV in Peer Initialization ACK signal\n");
		ret = 0;
	}

	if (!seen_status)
	{
		printf("Missing mandatory Status TLV in Peer Initialization ACK signal\n");
		ret = 0;
	}

	if (!seen_version)
	{
		printf("Missing mandatory DLEP Version TLV in Peer Initialization ACK signal\n");
		ret = 0;
	}

	if (!seen_opt_signals)
	{
		printf("Missing mandatory Optional Signals Supported TLV in Peer Initialization ACK signal\n");
		ret = 0;
	}

	if (!seen_opt_data_items)
	{
		printf("Missing mandatory Optional data Items Supported TLV in Peer Initialization ACK signal\n");
		ret = 0;
	}

	if (tlv != msg + len)
	{
		printf("Signal length does not equal sum of TLV lengths in Peer Initialization ACK signal\n");
		ret = 0;
	}

	return ret;
}

int check_heartbeat_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
	int seen_heartbeat = 0;

	/* Check for mandatory TLV's */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_HEARTBEAT_INTERVAL_TLV:
			if (seen_heartbeat)
			{
				printf("Multiple Heartbeat Interval TLVs in Heartbeat signal\n");
				ret = 0;
			}
			else if (!check_heartbeat_interval(tlv))
				ret = 0;
			else
				seen_heartbeat = 1;
			break;

		default:
			printf("Unexpected TLV %u in Heartbeat signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_heartbeat)
	{
		printf("Missing mandatory Heartbeat Interval TLV in Heartbeat signal\n");
		ret = 0;
	}

	return ret;
}

int check_peer_term_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
	int seen_status = 0;

	/* Check for mandatory TLV's */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_STATUS_TLV:
			if (seen_status)
			{
				printf("Multiple Status TLVs in Peer Termination signal\n");
				ret = 0;
			}
			else if (!check_status(tlv))
				ret = 0;
			else
				seen_status = 1;
			break;

		default:
			printf("Unexpected TLV %u in Peer Termination signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_status)
	{
		printf("Missing mandatory Status TLV in Peer Termination signal\n");
		ret = 0;
	}

	return ret;
}

int check_peer_update_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
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
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_IPV4_ADDRESS_TLV:
			if (!check_ipv4_address(tlv))
				ret = 0;
			break;

		case DLEP_IPV6_ADDRESS_TLV:
			if (!check_ipv6_address(tlv))
				ret = 0;
			break;

		case DLEP_MDRR_TLV:
			if (seen_mdrr)
			{
				printf("Multiple Maximum Data Rate (Receive) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_mdrr(tlv))
				ret = 0;
			else
				seen_mdrr = 1;
			break;

		case DLEP_MDRT_TLV:
			if (seen_mdrt)
			{
				printf("Multiple Maximum Data Rate (Transmit) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_mdrt(tlv))
				ret = 0;
			else
				seen_mdrt = 1;
			break;

		case DLEP_CDRR_TLV:
			if (seen_cdrr)
			{
				printf("Multiple Current Data Rate (Receive) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_cdrr(tlv))
				ret = 0;
			else
				seen_cdrr = 1;
			break;

		case DLEP_CDRT_TLV:
			if (seen_cdrt)
			{
				printf("Multiple Current Data Rate (Transmit) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_cdrt(tlv))
				ret = 0;
			else
				seen_cdrt = 1;
			break;

		case DLEP_LATENCY_TLV:
			if (seen_latency)
			{
				printf("Multiple Latency TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_latency(tlv))
				ret = 0;
			else
				seen_latency = 1;
			break;

		case DLEP_RESR_TLV:
			if (seen_resr)
			{
				printf("Multiple Resources (Receive) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_resr(tlv))
				ret = 0;
			else
				seen_resr = 1;
			break;

		case DLEP_REST_TLV:
			if (seen_rest)
			{
				printf("Multiple Resources (Transmit) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_rest(tlv))
				ret = 0;
			else
				seen_rest = 1;
			break;

		case DLEP_RLQR_TLV:
			if (seen_rlqr)
			{
				printf("Multiple Relative Link Quality (Receive) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_rlqr(tlv))
				ret = 0;
			else
				seen_rlqr = 1;
			break;

		case DLEP_RLQT_TLV:
			if (seen_rlqt)
			{
				printf("Multiple Relative Link Quality (Transmit) TLVs in Peer Update signal\n");
				ret = 0;
			}
			else if (!check_rlqt(tlv))
				ret = 0;
			else
				seen_rlqt = 1;
			break;

		case DLEP_VENDOR_EXTENSION_TLV:
			/* TODO: One should check for duplicates here */
			if (!check_vendor_extension(tlv))
				ret = 0;
			break;

		default:
			printf("Unexpected TLV %u in Peer Update signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	return ret;
}

int check_destination_up_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
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
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_MAC_ADDRESS_TLV:
			if (seen_mac)
			{
				printf("Multiple MAC Address TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_mac_address(tlv))
				ret = 0;
			else
				seen_mac = 1;
			break;

		case DLEP_IPV4_ADDRESS_TLV:
			if (!check_ipv4_address(tlv))
				ret = 0;
			break;

		case DLEP_IPV6_ADDRESS_TLV:
			if (!check_ipv6_address(tlv))
				ret = 0;
			break;

		case DLEP_MDRR_TLV:
			if (seen_mdrr)
			{
				printf("Multiple Maximum Data Rate (Receive) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_mdrr(tlv))
				ret = 0;
			else
				seen_mdrr = 1;
			break;

		case DLEP_MDRT_TLV:
			if (seen_mdrt)
			{
				printf("Multiple Maximum Data Rate (Transmit) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_mdrt(tlv))
				ret = 0;
			else
				seen_mdrt = 1;
			break;

		case DLEP_CDRR_TLV:
			if (seen_cdrr)
			{
				printf("Multiple Current Data Rate (Receive) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_cdrr(tlv))
				ret = 0;
			else
				seen_cdrr = 1;
			break;

		case DLEP_CDRT_TLV:
			if (seen_cdrt)
			{
				printf("Multiple Current Data Rate (Transmit) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_cdrt(tlv))
				ret = 0;
			else
				seen_cdrt = 1;
			break;

		case DLEP_LATENCY_TLV:
			if (seen_latency)
			{
				printf("Multiple Latency TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_latency(tlv))
				ret = 0;
			else
				seen_latency = 1;
			break;

		case DLEP_RESR_TLV:
			if (seen_resr)
			{
				printf("Multiple Resources (Receive) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_resr(tlv))
				ret = 0;
			else
				seen_resr = 1;
			break;

		case DLEP_REST_TLV:
			if (seen_rest)
			{
				printf("Multiple Resources (Transmit) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_rest(tlv))
				ret = 0;
			else
				seen_rest = 1;
			break;

		case DLEP_RLQR_TLV:
			if (seen_rlqr)
			{
				printf("Multiple Relative Link Quality (Receive) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_rlqr(tlv))
				ret = 0;
			else
				seen_rlqr = 1;
			break;

		case DLEP_RLQT_TLV:
			if (seen_rlqt)
			{
				printf("Multiple Relative Link Quality (Transmit) TLVs in Destination Up signal\n");
				ret = 0;
			}
			else if (!check_rlqt(tlv))
				ret = 0;
			else
				seen_rlqt = 1;
			break;

		default:
			printf("Unexpected TLV %u in Destination Up signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_mac)
	{
		printf("Missing mandatory MAC Address TLV in Destination Up signal\n");
		ret = 0;
	}

	return ret;
}

int check_destination_update_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
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
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_MAC_ADDRESS_TLV:
			if (seen_mac)
			{
				printf("Multiple MAC Address TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_mac_address(tlv))
				ret = 0;
			else
				seen_mac = 1;
			break;

		case DLEP_IPV4_ADDRESS_TLV:
			if (!check_ipv4_address(tlv))
				ret = 0;
			break;

		case DLEP_IPV6_ADDRESS_TLV:
			if (!check_ipv6_address(tlv))
				ret = 0;
			break;

		case DLEP_MDRR_TLV:
			if (seen_mdrr)
			{
				printf("Multiple Maximum Data Rate (Receive) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_mdrr(tlv))
				ret = 0;
			else
				seen_mdrr = 1;
			break;

		case DLEP_MDRT_TLV:
			if (seen_mdrt)
			{
				printf("Multiple Maximum Data Rate (Transmit) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_mdrt(tlv))
				ret = 0;
			else
				seen_mdrt = 1;
			break;

		case DLEP_CDRR_TLV:
			if (seen_cdrr)
			{
				printf("Multiple Current Data Rate (Receive) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_cdrr(tlv))
				ret = 0;
			else
				seen_cdrr = 1;
			break;

		case DLEP_CDRT_TLV:
			if (seen_cdrt)
			{
				printf("Multiple Current Data Rate (Transmit) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_cdrt(tlv))
				ret = 0;
			else
				seen_cdrt = 1;
			break;

		case DLEP_LATENCY_TLV:
			if (seen_latency)
			{
				printf("Multiple Latency TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_latency(tlv))
				ret = 0;
			else
				seen_latency = 1;
			break;

		case DLEP_RESR_TLV:
			if (seen_resr)
			{
				printf("Multiple Resources (Receive) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_resr(tlv))
				ret = 0;
			else
				seen_resr = 1;
			break;

		case DLEP_REST_TLV:
			if (seen_rest)
			{
				printf("Multiple Resources (Transmit) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_rest(tlv))
				ret = 0;
			else
				seen_rest = 1;
			break;

		case DLEP_RLQR_TLV:
			if (seen_rlqr)
			{
				printf("Multiple Relative Link Quality (Receive) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_rlqr(tlv))
				ret = 0;
			else
				seen_rlqr = 1;
			break;

		case DLEP_RLQT_TLV:
			if (seen_rlqt)
			{
				printf("Multiple Relative Link Quality (Transmit) TLVs in Destination Update signal\n");
				ret = 0;
			}
			else if (!check_rlqt(tlv))
				ret = 0;
			else
				seen_rlqt = 1;
			break;

		default:
			printf("Unexpected TLV %u in Destination Update signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_mac)
	{
		printf("Missing mandatory MAC Address TLV in Destination Update signal\n");
		ret = 0;
	}

	return ret;
}

int check_destination_down_signal(const char* tlvs, size_t len)
{
	int ret = 1;
	const char* tlv;
	int seen_mac = 0;

	/* Check for mandatory TLV's */
	for (tlv = tlvs; tlv < tlvs + len; tlv += tlv[1] + 2 /* Octet 1 is the TLV length */)
	{
		/* Octet 0 is the TLV type */
		switch ((enum dlep_tlvs)tlv[0])
		{
		case DLEP_MAC_ADDRESS_TLV:
			if (seen_mac)
			{
				printf("Multiple MAC Address TLVs in Destination Down signal\n");
				ret = 0;
			}
			else if (!check_mac_address(tlv))
				ret = 0;
			else
				seen_mac = 1;
			break;

		default:
			printf("Unexpected TLV %u in Destination Down signal\n",tlv[0]);
			ret = 0;
			break;
		}
	}

	if (!seen_mac)
	{
		printf("Missing mandatory MAC Address TLV in Destination Down signal\n");
		ret = 0;
	}

	return ret;
}
