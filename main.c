// nanopb example docs can be found at
// https://jpa.kapsi.fi/nanopb/docs/

// nanopb example code can be found at
// https://github.com/nanopb/nanopb/tree/master/examples

#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <pb.h>
#include <pb_encode.h>
#include <pb_decode.h>

#include "hello.pb.h"
#include "hello.grpc.pb.h"

#include "common.h"

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef IP_ADDR
#define IP_ADDR( a, b, c, d ) \
    ( \
        0 \
        | (((a) & 0xff) << 24) \
        | (((b) & 0xff) << 16) \
        | (((c) & 0xff) << 8) \
        | (((d) & 0xff) << 0) \
    )
#endif

int say_hello( int fd ) {
    const char *req_greeting = "Hello, nanopb!";
    HelloRequest req = HelloRequest_init_zero;
    strncpy( req.name, req_greeting, MIN( sizeof( req.name ) - 1, strlen( req_greeting ) ) );
    pb_ostream_t os = pb_ostream_from_socket( fd );
    bool encode_r = pb_encode_delimited( & os, HelloRequest_fields, & req );
    if ( ! encode_r ) {
        fprintf( stderr, "pb_encode_delimited: %s", PB_GET_ERROR( & os ) );
        return -1;
    }

    printf( "req: '%s'\n", req_greeting );

    return EXIT_SUCCESS;
}

int get_reply( int fd ) {
    HelloReply rsp = HelloReply_init_zero;
    pb_istream_t is = pb_istream_from_socket( fd );
    bool decode_r = pb_decode_delimited( & is, HelloReply_fields, & rsp );
    if ( ! decode_r ) {
        fprintf( stderr, "pb_decode_delimited: %s", PB_GET_ERROR( & is ) );
        return -1;
    }

    printf( "rsp: '%s'\n", rsp.message );

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {

    (void) argc;
    (void) argv;

    int fd;
    int r;
    struct sockaddr_in addr;

    r = socket( AF_INET, SOCK_STREAM, 0 );
    if ( -1 == r ) {
        perror("socket");
        r = EXIT_FAILURE;
        goto out;
    }
    fd = r;

    memset( & addr, 0, sizeof( addr ) );
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
    addr.sin_port = htons( 50051 );

    r = connect( fd, (const struct sockaddr *)& addr, sizeof( addr ) );
    if ( -1 == r ) {
        perror("connect");
        r = EXIT_FAILURE;
        goto close_fd;
    }

    r = say_hello( fd );
    if ( -1 ==r ) {
        r = EXIT_FAILURE;
        goto close_fd;
    }

    r = get_reply( fd );
    if ( -1 == r ) {
        r = EXIT_FAILURE;
        goto close_fd;
    }

    r = EXIT_SUCCESS;

close_fd:
    close( fd );
    fd = -1;

out:
    if ( EXIT_SUCCESS != r ) {
        //
    }
    return r;
}
