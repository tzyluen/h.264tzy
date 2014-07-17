#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <libswscale/swscale.h>
#include <x264.h>
#include "h264tzy.h"

int main(int argc, char **argv)
{
    create_raw_h264();
    return EXIT_SUCCESS;
}


void create_raw_h264()
{
    vpfile = fopen("sample.h264", "wb");
    
    x264_param_t param;
    x264_param_default_preset(&param, "veryfast", "zerolatency");
    param.i_threads = 1;
    param.i_width = H264TZY_DEFAULT_WIDTH;
    param.i_height = H264TZY_DEFAULT_HEIGHT;
    param.i_fps_num = 30;
    param.i_fps_den = 1;
    /* intra refres: */
    param.i_keyint_max = 30;
    param.b_intra_refresh = 1;
    /* rate control: */
    param.rc.i_rc_method = X264_RC_CRF;
    param.rc.f_rf_constant = 25;
    param.rc.f_rf_constant_max = 35;
    /* a .264 uses annexB with headers prepended to IDR frames.  */
    param.b_annexb = 1;
    /* for streaming: */
    param.b_repeat_headers = 1;
    param.i_log_level = X264_LOG_DEBUG;
    x264_param_apply_profile(&param, "baseline");

    /* initialize the encoder */
    x264_t *encoder = x264_encoder_open(&param);
    x264_picture_t pic_in, pic_out;
    x264_picture_alloc(&pic_in, X264_CSP_I420, H264TZY_DEFAULT_WIDTH, H264TZY_DEFAULT_HEIGHT);

    /* x264 expects YUV420P data; we could use libswscale to convert images to the right format.
       initialize as following (assuming RGB data with 24bpp) */
    struct SwsContext *convert_ctx = sws_getContext(
            H264TZY_DEFAULT_WIDTH, H264TZY_DEFAULT_HEIGHT,
            PIX_FMT_RGB24, H264TZY_DEFAULT_WIDTH, H264TZY_DEFAULT_HEIGHT,
            PIX_FMT_YUV420P, SWS_FAST_BILINEAR, NULL, NULL, NULL);

    /* data is a pointer to the RGB structure */
    int src_stride = H264TZY_DEFAULT_WIDTH * 3;
    /* scales image slice in `&data' and puts the resulting scaled slice in the image `pic_in.img.plane' */
    const uint8_t *data;
    sws_scale(convert_ctx, &data, &src_stride, 0, H264TZY_DEFAULT_HEIGHT, pic_in.img.plane, pic_in.img.i_stride);
    x264_nal_t *nals;
    int i_nals;
    int frame_size = x264_encoder_encode(encoder, &nals, &i_nals, &pic_in, &pic_out);
    if (frame_size >= 0) {
        printf("(%d) %d\n", __LINE__, frame_size);
    }

    //x264_encoder_parameters(encoder, &param);
    //x264_encoder_headers(encoder, &headers, &i_nal);
    //int size = headers[0].i_payload + headers[1].i_payload + headers[2].i_payload;
    //fwrite(headers[0].p_payload, 1, size, vpfile);
}
