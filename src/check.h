/*

Copyright (c) 2014 Airbus DS Limited


*/

#ifndef DLEP_TLV_CHECK_H_
#define DLEP_TLV_CHECK_H_

#include "dlep_iana.h"

enum dlep_status_code check_peer_offer_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_peer_init_ack_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_heartbeat_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_peer_term_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_peer_update_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_destination_up_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_destination_update_signal(const uint8_t* msg, size_t len);
enum dlep_status_code check_destination_down_signal(const uint8_t* msg, size_t len, const uint8_t** mac);
enum dlep_status_code check_link_char_request_signal(const uint8_t* msg, size_t len, const uint8_t** mac);

#endif /* DLEP_TLV_CHECK_H_ */
