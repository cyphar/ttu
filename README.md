ttu
===

Tcp To Unix (sockets).
A small wrapper program which switches TCP (actually, any `AF_INET`) sockets to Unix sockets transparently.

### Usage ###
`ttu` is provided in two forms:

* `libttu.so` -- the injection library (overrides `bind`, `connect`, etc) which does the magic
* `ttu` -- small wrapper program to make using `libttu.so` easier

#### libttu.so ####
`ttu` use `LD_PRELOAD` to inject the library into a program, silently overriding several socket-related system calls.

Basically, use `libttu.so` like this:
```
$ LD_PRELOAD="/path/to/libttu.so" TTU_BIND="..." TTU_CONNECT="..." program [args]...
```

Where:
* `/path/to/libttu.so` is the absolute path to the `libttu.so` binary.
* `TTU_BIND` and `TTU_CONNECT` are optional and described [here](#parameters).
* `program [args]` are the usual command-line arguments to run the program.

#### ttu ####
`ttu` automates the above process, making it much easier to use:

```
$ ttu [-l "/path/to/libttu.so"] [-b bind-map] [-c connect-map] -- program [args]
```

Where:
* `/path/to/libttu.so` is the absolute path to the `libttu.so` binary.
* `-b bind-map` and `-c connect-map` are optional and described [here](#parameters).
* `program [args]` are the usual command-line arguments to run the program.

### Parameters ###
The paramaters to ttu are the following:

* `TTU_BIND`: A mapping of bindings (inet listening sockets) to unix sockets (aka a bind-map).
* `TTU_CONNECT`: A mapping of connections (inet connecting sockets) to unix sockets (aka a connect-map).

Socket mappings are in this format:

```
[ip-addr]:[port]=/path/to/socket.sock,[ip-addr]:[port]=/path/to/socket.sock, ...
```

Where:

* `ip-addr` is the ip address of the inet socket (optional, defaults to `*`).
* `port` is the port of the inet socket (optional, defaults to `*`).
* `/path/to/socket.sock` is the path to the socket (mandatory, the path is *recommended* to be an absolute path [to avoid `chdir(2)` problems])

In the above options, `*` indicates 'any' of the field. The order of preference in finding a socket to bind to is as follows:

1. The exact match of `ip:port` in the parameters.
2. The wildcard match of `*:port` in the parameters.
3. The wildcard match of `ip:*` in the parameters.
4. The wilcard match of `*:*` in the parameters.
5. Passthrough and allow the socket to bind normally (no remapping).

If several different paramaters have the same or equivalent mapping (such as `0.0.0.0:80=/tmp/a,0.0.0.0:80=/tmp/b` or `*:*=/tmp/a,*:*=/tmp/b`), the socket file chosen is undefined.
