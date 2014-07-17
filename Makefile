CC=gcc
CFLAGS=-std=c99 -O0 -g -Wall
X264LIBS=-lm -lpthread -lx264 -lswscale

CXX=g++
CXXFLAGS=-O0 -g -Wall

LIBS=$(X264LIBS)

all: h264tzy mp4 mp3

h264tzy:
	$(CC) $(CFLAGS) $(LIBS) h264tzy.c -o h264tzy

mp4:
	$(CXX) $(CXXFLAGS) $(shell pkg-config --cflags --libs taglib) mp4.cpp -o mp4

mp3:
	$(CC) $(CFLAGS) mp3.c -o mp3

clean:
	rm -f *.o a.out h264tzy mp4 mp3
