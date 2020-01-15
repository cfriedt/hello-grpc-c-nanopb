#!/usr/bin/env python3

import sys

import grpc
import hello_pb2
import hello_pb2_grpc

def main(argv):
	channel = grpc.insecure_channel('localhost:50051')
	stub = hello_pb2_grpc.GreeterStub(channel)
	hello_req = hello_pb2.HelloRequest(name='gRPC world')
	hello_rsp = stub.SayHello(hello_req)
	print(hello_rsp.message)

if __name__ == '__main__':
	main(sys.argv)
