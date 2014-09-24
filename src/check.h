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