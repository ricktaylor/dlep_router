/*

Copyright (c) 2014 Airbus DS Limited


*/

#ifndef DLEP_TLV_CHECK_H_
#define DLEP_TLV_CHECK_H_

int check_peer_offer_signal(const char* msg, size_t len);
int check_peer_init_ack_signal(const char* msg, size_t len);
int check_heartbeat_signal(const char* tlvs, size_t len);
int check_peer_term_signal(const char* tlvs, size_t len);
int check_peer_update_signal(const char* tlvs, size_t len);
int check_destination_up_signal(const char* tlvs, size_t len);
int check_destination_update_signal(const char* tlvs, size_t len);
int check_destination_down_signal(const char* tlvs, size_t len);

#endif /* DLEP_TLV_CHECK_H_ */
