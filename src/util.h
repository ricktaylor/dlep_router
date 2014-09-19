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
