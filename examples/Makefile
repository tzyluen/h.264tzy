CC=gcc
CFLAGS=-O0 -g -Wall
LIBS_libav=-lm -lz -lpthread -lavformat -lavcodec -lavutil

LIBS=$(LIBS_libav)

all: metadata filter_audio avcodec output transcode_aac

metadata:
	$(CC) $(CFLAGS) $(LIBS) metadata.c -o metadata
filter_audio:
	$(CC) $(CFLAGS) $(LIBS) filter_audio.c -o filter_audio
avcodec:
	$(CC) $(CFLAGS) $(LIBS) avcodec.c -o avcodec
output:
	$(CC) $(CFLAGS) $(LIBS) output.c -o output
transcode_aac:
	$(CC) $(CFLAGS) $(LIBS) transcode_aac.c -o transcode_aac

clean:
	rm -f *.o a.out avcodec output metadata filter_audio transcode_aac
