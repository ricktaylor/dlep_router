/*

Copyright (c) 2014 Airbus DS Limited

*/

#ifndef DLEP_UTIL_H_
#define DLEP_UTIL_H_

#define _GNU_SOURCE

#include <sys/time.h>
#include <stdint.h>
#include <arpa/inet.h>

uint16_t get_uint16(const char* p);
void set_uint16(uint16_t v, char* p);

uint32_t get_uint32(const char* p);
uint64_t get_uint64(const char* p);

#define FORMATADDRESS_LEN INET6_ADDRSTRLEN+6
const char* formatAddress(const struct sockaddr* addr, char* str, size_t str_len);

void printfBytes(const char* p, size_t len, char sep);

int interval_compare(const struct timespec* start, const struct timespec* end, unsigned int interval);

#endif /* DLEP_UTIL_H_ */
