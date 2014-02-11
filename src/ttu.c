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

#include <ohmic.h>
#include <ttu.h>

#define _lenprintf(fmt, ...) (snprintf(NULL, 0, fmt, __VA_ARGS__) + 1)

static void *_dlhandle;

static int (*_bind)(int, const struct sockaddr *, socklen_t);
static int (*_connect)(int, const struct sockaddr *, socklen_t);

static struct ohm_t *_bindmap = NULL;
static struct ohm_t *_connectmap = NULL;

static void _bail(char *err) {
	fprintf(stderr, "libttu: %s\n", err);
	abort();
} /* _bail() */

static char *_intos(struct in_addr addr, in_port_t port) {
	char *saddr = inet_ntoa(addr);
	port = htons(port);

	char *key = malloc(_lenprintf("%s:%d", saddr, port));
	sprintf(key, "%s:%d", saddr, port);

	return key;
} /* _intos() */

static int _ttusock(int sockfd) {
	int newsockfd = 0,
		socktp = 0;

	socklen_t size = sizeof(int);

	getsockopt(sockfd, SOL_SOCKET, SO_TYPE, &socktp, &size);
	newsockfd = socket(AF_UNIX, socktp, 0);

	return dup2(newsockfd, sockfd);
} /* _ttusock() */

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if(addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
		struct sockaddr_in *iaddr = (struct sockaddr_in *) addr;

		char *key = _intos(iaddr->sin_addr, iaddr->sin_port),
			 *sockfile = ohm_search(_bindmap, key, strlen(key) + 1);

		if(sockfile != NULL) {
			struct sockaddr_un uaddr;

			memset(&uaddr, 0, sizeof(struct sockaddr_un));

			uaddr.sun_family = AF_UNIX;
			memcpy(uaddr.sun_path, sockfile, 108);

			addr = (struct sockaddr *) &uaddr;
			addrlen = sizeof(struct sockaddr_un);
			_ttusock(sockfd);
		}

		free(key);
	}

	return _bind(sockfd, addr, addrlen);
} /* bind() */

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	if(addr->sa_family == AF_INET || addr->sa_family == AF_INET6) {
		struct sockaddr_in *iaddr = (struct sockaddr_in *) addr;

		char *key = _intos(iaddr->sin_addr, iaddr->sin_port),
			 *sockfile = ohm_search(_connectmap, key, strlen(key) + 1);

		if(sockfile != NULL) {
			struct sockaddr_un uaddr;

			memset(&uaddr, 0, sizeof(struct sockaddr_un));

			uaddr.sun_family = AF_UNIX;
			memcpy(uaddr.sun_path, sockfile, 108);

			addr = (struct sockaddr *) &uaddr;
			addrlen = sizeof(struct sockaddr_un);
			_ttusock(sockfd);
		}

		free(key);
	}

	return _connect(sockfd, addr, addrlen);
} /* connect() */

static void *_dlsym(void *handle, char *name) {
	void *symbol = dlsym(handle, name);

	char *err = dlerror();
	if(err)
		_bail(err);

	return symbol;
} /* _dlsym() */

static char *_strdup(char *s) {
	int len = strlen(s);
	char *ret = malloc(len + 1);

	strncpy(ret, s, len);
	ret[len] = '\0';

	return ret;
} /* _strdup() */

static void _etohm(struct ohm_t *hm, char *env) {
	if(!env)
		return;

	env = _strdup(env);

	char *current = strtok(env, ",");
	while(current) {
		char *addr = strtok(current, "="),
			 *sockfile = strtok(NULL, "=");

		if(!addr || !sockfile)
			break;

		ohm_insert(hm, addr, strlen(addr) + 1, sockfile, strlen(sockfile) + 1);
		current = strtok(NULL, ",");
	}

	free(env);
} /* _etohm() */

void __attribute__((constructor)) init(void) {
	_dlhandle = dlopen("libc.so.6", RTLD_LAZY | RTLD_LOCAL);

	if(!_dlhandle)
		_bail(dlerror());

	_bind    = _dlsym(_dlhandle, "bind");
	_connect = _dlsym(_dlhandle, "connect");

	_bindmap    = ohm_init(4, NULL);
	_connectmap = ohm_init(4, NULL);

	_etohm(_bindmap, getenv(TTU_BIND_ENV));
	_etohm(_connectmap, getenv(TTU_CONNECT_ENV));

	struct ohm_iter ii = ohm_iter_init(_bindmap);
	for(ii = ohm_iter_init(_bindmap); ii.key != NULL; ohm_iter_inc(&ii))
		unlink(ii.value);
} /* init() */

void __attribute__((destructor)) fini(void) {
	int err = dlclose(_dlhandle);

	if(err)
		_bail(dlerror());

	struct ohm_iter ii = ohm_iter_init(_bindmap);
	for(ii = ohm_iter_init(_bindmap); ii.key != NULL; ohm_iter_inc(&ii))
		unlink(ii.value);

	ohm_free(_bindmap);
	ohm_free(_connectmap);
} /* fini() */
