#ifndef MP4_H_
#define MP4_H_

typedef enum {
    moov, udta, mdia,
    meta, ilst, stbl,
    minf, moof, traf,
    trak, stsd
} t_MP4_containers;

char *mp4_codec_str[] = {
    "Unknown",
    "AAC",
    "ALAC"
};

void p_mp4_header(TagLib::FileRef);
#endif
