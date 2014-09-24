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



This file defines the constant values that will one day be defined by an
IANA registry.  Currently, invented values are used.  Do not assume that these
values will survive the standardisation process.

*/

#ifndef DLEP_IANA_H_
#define DLEP_IANA_H_

/* The well-known multicast address for modem discovery */
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS "224.0.0.109"
#define DLEP_WELL_KNOWN_MULTICAST_ADDRESS_6 "FF02::6D"

/* The well-known port for modem discovery */
#define DLEP_WELL_KNOWN_MULTICAST_PORT 22222

/* The signal numbers */
#define DLEP_PEER_DISCOVERY                0
#define DLEP_PEER_OFFER                    1
#define DLEP_PEER_INITIALIZATION           2
#define DLEP_PEER_INITIALIZATION_ACK       3
#define DLEP_PEER_TERMINATION              4
#define DLEP_PEER_TERMINATION_ACK          5
#define DLEP_PEER_UPDATE                   6
#define DLEP_PEER_UPDATE_ACK               7
#define DLEP_DESTINATION_UP                8
#define DLEP_DESTINATION_UP_ACK            9
#define DLEP_DESTINATION_DOWN             10
#define DLEP_DESTINATION_DOWN_ACK         11
#define DLEP_DESTINATION_UPDATE           12
#define DLEP_LINK_CHARACTERISTICS_REQUEST 13
#define DLEP_LINK_CHARACTERISTICS_ACK     14
#define DLEP_HEARTBEAT                    15

/* The TLV discriminators */
#define DLEP_PORT_TLV                 0
#define DLEP_PEER_TYPE_TLV            1
#define DLEP_MAC_ADDRESS_TLV          2
#define DLEP_IPV4_ADDRESS_TLV         3
#define DLEP_IPV6_ADDRESS_TLV         4
#define DLEP_MDRR_TLV                 5
#define DLEP_MDRT_TLV                 6
#define DLEP_CDRR_TLV                 7
#define DLEP_CDRT_TLV                 8
#define DLEP_LATENCY_TLV              9
#define DLEP_RESR_TLV                10
#define DLEP_REST_TLV                11
#define DLEP_RLQR_TLV                12
#define DLEP_RLQT_TLV                13
#define DLEP_STATUS_TLV              14
#define DLEP_HEARTBEAT_INTERVAL_TLV  15
#define DLEP_LINK_CHAR_ACK_TIMER_TLV 16
#define DLEP_CREDIT_WIN_STATUS_TLV   17
#define DLEP_CREDIT_GRANT_REQ_TLV    18
#define DLEP_CREDIT_REQUEST_TLV      19
#define DLEP_OPTIONAL_SIGNALS_TLV    20
#define DLEP_OPTIONAL_DATA_ITEMS_TLV 21
#define DLEP_VENDOR_EXTENSION_TLV    22

/* Other, non-IANA, dlep_router default values */

/* The delay between Peer Discovery messages */
#define DEFAULT_DISCOVERY_RETRY    3

/* The default Heartbeat Interval */
#define DEFAULT_HEARTBEAT_INTERVAL 5

#endif /* DLEP_IANA_H_ */