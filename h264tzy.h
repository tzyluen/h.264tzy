#ifndef H264TZY_H_
#define H264TZY_H_

#define H264TZY_DEFAULT_WIDTH  1280
#define H264TZY_DEFAULT_HEIGHT  800


x264_nal_t *headers;
int i_nal;
FILE *vpfile;

void create_raw_h264();

#endif
