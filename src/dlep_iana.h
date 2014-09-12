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
#define DLEP_PEER_DISCOVERY 0
#define DLEP_PEER_OFFER     1

/* The TLV discriminators */
#define DLEP_PORT_TLV               0
#define DLEP_HEARTBEAT_INTERVAL_TLV 1
#define DLEP_IPV4_ADDRESS_TLV       2
#define DLEP_IPV6_ADDRESS_TLV       3
#define DLEP_PEER_TYPE_TLV          4


/* Other, non-IANA, dlep_router default values */

/* The delay between Peer Discovery messages */
#define DEFAULT_DISCOVERY_RETRY    3

/* The default Heartbeat Interval */
#define DEFAULT_HEARTBEAT_INTERVAL 5

#endif /* DLEP_IANA_H_ */
