#ifndef MP3_H_
#define MP3_H_

/* 4 bytes of mp3 header */
unsigned int MP3header_offsets[] = { 0, 11, 13, 15, 16, 20, 22, 23, 24, 26, 28, 29, 30 };
unsigned int MP3header_masks[] = { 0x7FF, 0x1800, 0x6000, 0x8000, 0xF0000, 0x300000,
        0x400000, 0x800000, 0x3000000, 0xC000000, 0x10000000, 0x20000000, 0xC0000000 };
typedef enum {         /* bits: description */
    MP3H_FRAMESYNC,    /*   11: frame sync (all bits set) */
    MP3H_MPEGID,       /*    2: MPEG audio version ID: 00|01|10|11 */
    MP3H_LAYER,        /*    2: layer description: 00|01|10|11 (layer 1 from right) */
    MP3H_PROTECTBIT,   /*    1: CRC protection */
    MP3H_BITRATEINDX,  /*    4: bitrate index */
    MP3H_SAMPLEFREQ,   /*    2: sampling rate freq index */
    MP3H_PADDINGBIT,   /*    1: padding bit */
    MP3H_PRIVATEBIT,   /*    1: private bit: freely used for specific needs */
    MP3H_CHANNEL,      /*    2: channel mode: 00|01|10|11 (stereo..single channel) */
    MP3H_MODEEXT,      /*    2: mode extension */
    MP3H_COPYRIGHT,    /*    1: copyright: 0|1 (1 is copyrighted) */
    MP3H_ORIGINAL,     /*    1: original: 0|1 (1 is original media) */
    MP3H_EMPHASIS      /*    2: emphasis */
} MP3header_enum;


typedef struct {
    char        tagid[3];
    char        name[30];
    char        artist[30];
    char        album[30];
    uint32_t    year;
    char        comment[30];
    char        genre[8];
} ID3tagv1_t;


typedef struct {
    char        tagid[3];   /* TAG identifier: ID3 */
    uint16_t    tagver;     /* TAG version */
    char        flags;      /* flags */
    uint64_t    size;       /* TAG MSB sizes */
} ID3tagv2_t;


void showbits(unsigned int);
int read_ID3tag(FILE *, uint8_t *);
int read_header(FILE *);

#endif
