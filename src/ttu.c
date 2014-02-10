/* ttu: A program to silently convert TCP sockets to Unix sockets
 * Copyright (C) 2014 Cyphar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * 1. The above copyright notice and this permission notice shall be included in
 *    all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define _BSD_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>

#include <sys/un.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>

#include <dlfcn.h>

/* TODO: allow options for mappings [ip:port] => [socket], remove the hardcoded socket file */
/* TODO: use a hashmap to store the given settings for what the [ip:port] => [socket] mappings are */
/* XXX: is 'connect' in the plan? */

static void *_dlhandle;
static int (*_socket)(int, int, int);
static int (*_bind)(int, const struct sockaddr *, socklen_t);

static void _bail(char *err) {
	fprintf(stderr, "libttu: %s\n", err);
	abort();
} /* _bail() */

int socket(int domain, int type, int protocol) {
	if(domain == AF_INET || domain == AF_INET6) {
		domain = AF_UNIX;
		protocol = 0;
	}

	return _socket(domain, type, protocol);
} /* socket() */

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if(addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
		struct sockaddr_in iaddr = *(struct sockaddr_in *) addr;
		struct sockaddr_un uaddr;

		memset(&uaddr, 0, sizeof(struct sockaddr_un));

		uaddr.sun_family = AF_UNIX;
		memcpy(uaddr.sun_path, "./socket", 108);

		addr = (struct sockaddr *) &uaddr;
		addrlen = sizeof(struct sockaddr_un);
	}

	return _bind(sockfd, addr, addrlen);
} /* bind() */

static void *_dlsym(void *handle, char *name) {
	void *symbol = dlsym(handle, name);

	char *err = dlerror();
	if(err)
		_bail(err);

	return symbol;
} /* _dlsym() */

void __attribute__((constructor)) init(void) {
	_dlhandle = dlopen("libc.so.6", RTLD_LAZY | RTLD_LOCAL);

	if(!_dlhandle)
		_bail(dlerror());

	_socket = _dlsym(_dlhandle, "socket");
	_bind = _dlsym(_dlhandle, "bind");
} /* init() */

void __attribute__((destructor)) fini(void) {
	int err = dlclose(_dlhandle);

	if(err)
		_bail(dlerror());

	unlink("./socket");
} /* fini() */
