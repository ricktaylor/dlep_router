/*

Copyright (c) 2014 Airbus DS Limited

*/

/*
 * This file defines the constant values that will one day be defined by an
 * IANA registry.  Currently, invented values are used.  Do not assume that
 * these values will survive the standardisation process.
 */

#ifndef DLEP_IANA_H_
#define DLEP_IANA_H_

/* The well-known multicast address for modem discovery */
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS "224.0.0.109"
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS_6 "FF02::6D"

/* The well-known port for modem discovery */
#define DLEP_WELL_KNOWN_MULTICAST_PORT 22222

/* The well-known TCP port for session, Section 11.7) */
#define DLEP_WELL_KNOWN_TCP_PORT 22223

/* The signal numbers - The values are NOT final */
enum dlep_signals {
  DLEP_PEER_DISCOVERY               =  0,
  DLEP_PEER_OFFER                   =  1,
  DLEP_PEER_INIT                    =  2,
  DLEP_PEER_INIT_ACK                =  3,
  DLEP_PEER_UPDATE                  =  4,
  DLEP_PEER_UPDATE_ACK              =  5,
  DLEP_PEER_TERM                    =  6,
  DLEP_PEER_TERM_ACK                =  7,
  DLEP_DEST_UP                      =  8,
  DLEP_DEST_UP_ACK                  =  9,
  DLEP_DEST_DOWN                    = 10,
  DLEP_DEST_DOWN_ACK                = 11,
  DLEP_DEST_UPDATE                  = 12,
  DLEP_PEER_HEARTBEAT               = 13,
  DLEP_LINK_CHAR_REQ                = 14,
  DLEP_LINK_CHAR_ACK                = 15,

  DLEP_SIGNAL_COUNT
};

/* The TLV discriminators - The values are NOT final */
enum dlep_tlvs {
  DLEP_VERSION_TLV             =  0,
  DLEP_STATUS_TLV              =  1,
  DLEP_IPV4_CONN_POINT_TLV     =  2,
  DLEP_IPV6_CONN_POINT_TLV     =  3,
  DLEP_PEER_TYPE_TLV           =  4,
  DLEP_PEER_HEARTBEAT_INTERVAL_TLV  =  5,
  DLEP_EXTS_SUPP_TLV           =  6,
  DLEP_EXP_DEFNS_TLV           =  7,
  DLEP_MAC_ADDRESS_TLV         =  8,
  DLEP_IPV4_ADDRESS_TLV        =  9,
  DLEP_IPV6_ADDRESS_TLV        = 10,
  DLEP_IPV4_ATT_SUBNET_TLV     = 11,
  DLEP_IPV6_ATT_SUBNET_TLV     = 12,
  DLEP_MDRR_TLV                = 13,
  DLEP_MDRT_TLV                = 14,
  DLEP_CDRR_TLV                = 15,
  DLEP_CDRT_TLV                = 16,
  DLEP_LATENCY_TLV             = 17,
  DLEP_RESR_TLV                = 18,
  DLEP_REST_TLV                = 19,
  DLEP_RLQR_TLV                = 20,
  DLEP_RLQT_TLV                = 21,
  DLEP_LINK_CHAR_ACK_TIMER_TLV = 22,

  DLEP_TLV_COUNT
};

/* The DLEP Status Codes - The values are NOT final */
enum dlep_status_code {
  DLEP_SC_SUCCESS              =  0,
  DLEP_SC_UNKNOWN_SIGNAL       =  1,
  DLEP_SC_INVALID_DATA         =  2,
  DLEP_SC_UNEXPECTED_SIGNAL    =  3,
  DLEP_SC_REQUEST_DENIED       =  4,
  DLEP_SC_TIMEDOUT             =  5,
  DLEP_SC_INVALID_DEST         =  6
};

/* Other, non-IANA, dlep_router default values */

#define PEER_TYPE	"dlep_router: Simple example DLEP router"

/* The delay between Peer Discovery messages */
#define DEFAULT_DISCOVERY_RETRY    3

/* The default Heartbeat Interval */
#define DEFAULT_HEARTBEAT_INTERVAL 5

#endif /* DLEP_IANA_H_ */
