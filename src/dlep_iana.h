/*

Copyright (c) 2017 Airbus DS Limited

*/

/*
 * This file defines the constant values in the various DLEP IANA registries
 */

#ifndef DLEP_IANA_H_
#define DLEP_IANA_H_

/* The well-known multicast address for modem discovery */
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS "224.0.0.117"
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS_6 "FF02::1:7"

/* The well-known TCP port for session, Section 15.14 */
#define DLEP_WELL_KNOWN_PORT 854

/* The signal numbers */
enum dlep_signal {
  DLEP_PEER_DISCOVERY               =  1,
  DLEP_PEER_OFFER                   =  2
};

/* The message numbers */
enum dlep_message {
  DLEP_SESSION_INIT                 =  1,
  DLEP_SESSION_INIT_RESP            =  2,
  DLEP_SESSION_UPDATE               =  3,
  DLEP_SESSION_UPDATE_RESP          =  4,
  DLEP_SESSION_TERM                 =  5,
  DLEP_SESSION_TERM_RESP            =  6,
  DLEP_DEST_UP                      =  7,
  DLEP_DEST_UP_RESP                 =  8,
  DLEP_DEST_ANNOUNCE                =  9,
  DLEP_DEST_ANNOUNCE_RESP           = 10,
  DLEP_DEST_DOWN                    = 11,
  DLEP_DEST_DOWN_RESP               = 12,
  DLEP_DEST_UPDATE                  = 13,
  DLEP_LINK_CHAR_REQ                = 14,
  DLEP_LINK_CHAR_RESP               = 15,
  DLEP_PEER_HEARTBEAT               = 16
};

/* The Data item numbers */
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
  DLEP_RESOURCES_DATA_ITEM           = 17,
  DLEP_RLQR_DATA_ITEM                = 18,
  DLEP_RLQT_DATA_ITEM                = 19,
  DLEP_MTU_DATA_ITEM                 = 20
};

/* The DLEP Status Codes - The values are NOT final */
enum dlep_status_code {
  DLEP_SC_SUCCESS              =   0,
  DLEP_SC_NOT_INTERESTED       =   1,
  DLEP_SC_REQUEST_DENIED       =   2,
  DLEP_SC_INCONSISTENT         =   3,
  DLEP_SC_UNKNOWN_MESSAGE      = 128,
  DLEP_SC_UNEXPECTED_MESSAGE   = 129,
  DLEP_SC_INVALID_DATA         = 130,
  DLEP_SC_INVALID_DEST         = 131,
  DLEP_SC_TIMEDOUT             = 132,

  DLEP_SC_SHUTDOWN             = 255
};

/* Other, non-IANA, dlep_router default values */

#define PEER_TYPE	"dlep_router: Simple example DLEP router"

/* The delay between Peer Discovery messages */
#define DEFAULT_DISCOVERY_RETRY    3

/* The default Heartbeat Interval */
#define DEFAULT_HEARTBEAT_INTERVAL 30

#endif /* DLEP_IANA_H_ */
