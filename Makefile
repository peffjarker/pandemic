all: build

build: pandemic.cc
	g++ -g pandemic.cc -pthread -o p
