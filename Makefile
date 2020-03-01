all: build

build: pandemic.cc
	g++ -Wall -Wextra -g pandemic.cc -pthread -o p
