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

#include "./util.h"

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

uint16_t get_uint16(const char* p)
{
	/* Avoid ntohs() due to unaligned access issues on some architectures */
	return (p[0] << 8 | p[1]);
}

void set_uint16(uint16_t v, char* p)
{
	/* Avoid htons() due to unaligned access issues on some architectures */
	p[0] = v >> 8;
	p[1] = v & 0xFF;
}

uint32_t get_uint32(const char* p)
{
	/* Avoid ntohl() due to unaligned access issues on some architectures */
	uint32_t v = *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	return v;
}

uint64_t get_uint64(const char* p)
{
	uint64_t v = *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	v = (v << 8) | *p++;
	return v;
}

const char* formatAddress(const struct sockaddr* addr, char* str, size_t str_len)
{
	const char* ret = NULL;
	char address[INET6_ADDRSTRLEN] = {0};
	size_t len = 0;
	in_port_t port = 0;

	if (addr->sa_family == AF_INET)
	{
		port = ntohs(((struct sockaddr_in*)addr)->sin_port);
		ret = inet_ntop(AF_INET,&((struct sockaddr_in*)addr)->sin_addr,address,sizeof(address));
	}
	else if (addr->sa_family == AF_INET6)
	{
		port = ntohs(((struct sockaddr_in6*)addr)->sin6_port);
		ret = inet_ntop(AF_INET6,&((struct sockaddr_in6*)addr)->sin6_addr,address,sizeof(address));
	}
	else
		errno = EINVAL;

	if (!ret)
		return NULL;

	if (addr->sa_family == AF_INET)
		snprintf(str,str_len-1,"%s:%u",address,port);
	else
		snprintf(str,str_len-1,"{%s}:%u",address,port);

	str[str_len-1] = '\0';
	return str;
}

void printfBytes(const char* p, size_t len, char sep)
{
	while (len--)
	{
		printf("%2X%c",*p++,sep);
	}
}

int interval_compare(const struct timespec* start, const struct timespec* end, unsigned int interval)
{
	struct timespec diff_time;
	diff_time.tv_sec = end->tv_sec - start->tv_sec;
	diff_time.tv_nsec = end->tv_nsec - start->tv_nsec;
	if (diff_time.tv_nsec < 0)
	{
		--diff_time.tv_sec;
		diff_time.tv_nsec += 1000000000;
	}

	if (diff_time.tv_sec < interval)
		return -1;
	if (diff_time.tv_sec > interval || diff_time.tv_nsec > 0)
		return 1;

	return 0;
}
