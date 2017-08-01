/*

Copyright (c) 2017 Airbus DS Limited

*/

#ifndef DLEP_UTIL_H_
#define DLEP_UTIL_H_

#define _GNU_SOURCE

#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <arpa/inet.h>

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif

uint16_t read_uint16(const uint8_t* p);
uint8_t* write_uint16(uint16_t v, uint8_t* p);

uint32_t read_uint32(const uint8_t* p);
uint8_t* write_uint32(uint32_t v, uint8_t* p);

uint64_t read_uint64(const uint8_t* p);

#define FORMATADDRESS_LEN INET6_ADDRSTRLEN+6
const char* formatAddress(const struct sockaddr* addr, char* str, size_t str_len);

int interval_compare(const struct timespec* start, const struct timespec* end, unsigned int interval);

#endif /* DLEP_UTIL_H_ */
