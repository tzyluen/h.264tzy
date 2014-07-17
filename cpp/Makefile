CXX=g++
CXXFLAGS=-O0 -g -Wall
H264FILE=sample_iPod.m4v
LIBS_ffmpeg=-lm -lz -lpthread -lavformat -lavcodec -lavutil

LIBS=$(LIBS_ffmpeg)

all:
	$(CC) $(CFLAGS) $(LIBS) -o YUV420P_Player YUV420P_Player.cpp

clean:
	rm -f *.o a.out 
