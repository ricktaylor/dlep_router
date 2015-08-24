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

/* The well-known TCP port for session, Section 11.7 */
#define DLEP_WELL_KNOWN_TCP_PORT 22222

/* Currently we use the version number 0.14 until WG last call completes */
#define DLEP_MAJOR_VERSION 0
#define DLEP_MINOR_VERSION 14

/* The signal numbers - The values are NOT final */
enum dlep_signal {
  DLEP_PEER_DISCOVERY               =  1,
  DLEP_PEER_OFFER                   =  2
};

/* The message numbers - The values are NOT final */
enum dlep_message {
  DLEP_SESSION_INIT                 =  3,
  DLEP_SESSION_INIT_RESP            =  4,
  DLEP_SESSION_UPDATE               =  5,
  DLEP_SESSION_UPDATE_RESP          =  6,
  DLEP_SESSION_TERM                 =  7,
  DLEP_SESSION_TERM_RESP            =  8,
  DLEP_DEST_UP                      =  9,
  DLEP_DEST_UP_RESP                 = 10,
  DLEP_DEST_DOWN                    = 11,
  DLEP_DEST_DOWN_RESP               = 12,
  DLEP_DEST_UPDATE                  = 13,
  DLEP_PEER_HEARTBEAT               = 14,
  DLEP_LINK_CHAR_REQ                = 15,
  DLEP_LINK_CHAR_RESP               = 16
};

/* The Data item numbers - The values are NOT final */
enum dlep_data_item {
  DLEP_STATUS_DATA_ITEM              =  1,
  DLEP_IPV4_CONN_POINT_DATA_ITEM     =  2,
  DLEP_IPV6_CONN_POINT_DATA_ITEM     =  3,
  DLEP_PEER_TYPE_DATA_ITEM           =  4,
  DLEP_HEARTBEAT_INTERVAL_DATA_ITEM  =  5,
  DLEP_EXTS_SUPP_DATA_ITEM           =  6,
  DLEP_MAC_ADDRESS_DATA_ITEM         =  7,
  DLEP_IPV4_ADDRESS_DATA_ITEM        =  8,
  DLEP_IPV6_ADDRESS_DATA_ITEM        =  9,
  DLEP_IPV4_ATT_SUBNET_DATA_ITEM     = 10,
  DLEP_IPV6_ATT_SUBNET_DATA_ITEM     = 11,
  DLEP_MDRR_DATA_ITEM                = 12,
  DLEP_MDRT_DATA_ITEM                = 13,
  DLEP_CDRR_DATA_ITEM                = 14,
  DLEP_CDRT_DATA_ITEM                = 15,
  DLEP_LATENCY_DATA_ITEM             = 16,
  DLEP_RESR_DATA_ITEM                = 17,
  DLEP_REST_DATA_ITEM                = 18,
  DLEP_RLQR_DATA_ITEM                = 19,
  DLEP_RLQT_DATA_ITEM                = 20,
  DLEP_LINK_CHAR_RESP_TIMER_DATA_ITEM = 21,

  /* No credit-windowing support */

  /* A bank of experimental data items */
  DLEP_EXPERIMENTAL_DATA_ITEM_FIRST   = 65408,
  DLEP_EXPERIMENTAL_DATA_ITEM_LAST    = 65534
};

/* The DLEP Status Codes - The values are NOT final */
enum dlep_status_code {
  DLEP_SC_SUCCESS              =   0,
  DLEP_SC_UNKNOWN_MESSAGE      =   1,
  DLEP_SC_UNEXPECTED_MESSAGE   =   2,
  DLEP_SC_INVALID_DATA         =   3,
  DLEP_SC_INVALID_DEST         =   4,
  DLEP_SC_NOT_INTERESTED       = 100,
  DLEP_SC_REQUEST_DENIED       = 101,
  DLEP_SC_TIMEDOUT             = 102
};

/* Other, non-IANA, dlep_router default values */

#define PEER_TYPE	"dlep_router: Simple example DLEP router"

/* The delay between Peer Discovery messages */
#define DEFAULT_DISCOVERY_RETRY    3

/* The default Heartbeat Interval */
#define DEFAULT_HEARTBEAT_INTERVAL 5

#endif /* DLEP_IANA_H_ */
