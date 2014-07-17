/*
 * This file is part of Libav.
 *
 * Libav is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Libav is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Libav; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <stdio.h>
#include "libavformat/avformat.h"
#include "libavformat/avio.h"
#include "libavcodec/avcodec.h"
#include "libavutil/audio_fifo.h"
#include "libavutil/avstring.h"
#include "libavutil/frame.h"
#include "libavutil/opt.h"
#include "libavresample/avresample.h"
#define OUTPUT_BIT_RATE 48000
#define OUTPUT_CHANNELS 2
#define OUTPUT_SAMPLE_FORMAT AV_SAMPLE_FMT_S16
static char *const get_error_text(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}
static int open_input_file(const char *filename,
                           AVFormatContext **input_format_context,
                           AVCodecContext **input_codec_context)
{
    AVCodec *input_codec;
    int error;
    if ((error = avformat_open_input(input_format_context, filename, NULL,
                                     NULL)) < 0) {
        fprintf(stderr, "Could not open input file '%s' (error '%s')\n",
                filename, get_error_text(error));
        *input_format_context = NULL;
        return error;
    }
    if ((error = avformat_find_stream_info(*input_format_context, NULL)) < 0) {
        fprintf(stderr, "Could not open find stream info (error '%s')\n",
                get_error_text(error));
        avformat_close_input(input_format_context);
        return error;
    }
    if ((*input_format_context)->nb_streams != 1) {
        fprintf(stderr, "Expected one audio input stream, but found %d\n",
                (*input_format_context)->nb_streams);
        avformat_close_input(input_format_context);
        return AVERROR_EXIT;
    }
    if (!(input_codec = avcodec_find_decoder((*input_format_context)->streams[0]->codec->codec_id))) {
        fprintf(stderr, "Could not find input codec\n");
        avformat_close_input(input_format_context);
        return AVERROR_EXIT;
    }
    if ((error = avcodec_open2((*input_format_context)->streams[0]->codec,
                               input_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open input codec (error '%s')\n",
                get_error_text(error));
        avformat_close_input(input_format_context);
        return error;
    }
    *input_codec_context = (*input_format_context)->streams[0]->codec;
    return 0;
}
static int open_output_file(const char *filename,
                            AVCodecContext *input_codec_context,
                            AVFormatContext **output_format_context,
                            AVCodecContext **output_codec_context)
{
    AVIOContext *output_io_context = NULL;
    AVStream *stream               = NULL;
    AVCodec *output_codec          = NULL;
    int error;
    if ((error = avio_open(&output_io_context, filename,
                           AVIO_FLAG_WRITE)) < 0) {
        fprintf(stderr, "Could not open output file '%s' (error '%s')\n",
                filename, get_error_text(error));
        return error;
    }
    if (!(*output_format_context = avformat_alloc_context())) {
        fprintf(stderr, "Could not allocate output format context\n");
        return AVERROR(ENOMEM);
    }
    (*output_format_context)->pb = output_io_context;
    if (!((*output_format_context)->oformat = av_guess_format(NULL, filename,
                                                              NULL))) {
        fprintf(stderr, "Could not find output file format\n");
        goto cleanup;
    }
    av_strlcpy((*output_format_context)->filename, filename,
               sizeof((*output_format_context)->filename));
    if (!(output_codec = avcodec_find_encoder(AV_CODEC_ID_AAC))) {
        fprintf(stderr, "Could not find an AAC encoder.\n");
        goto cleanup;
    }
    if (!(stream = avformat_new_stream(*output_format_context, output_codec))) {
        fprintf(stderr, "Could not create new stream\n");
        error = AVERROR(ENOMEM);
        goto cleanup;
    }
    *output_codec_context = stream->codec;
    (*output_codec_context)->channels       = OUTPUT_CHANNELS;
    (*output_codec_context)->channel_layout = av_get_default_channel_layout(OUTPUT_CHANNELS);
    (*output_codec_context)->sample_rate    = input_codec_context->sample_rate;
    (*output_codec_context)->sample_fmt     = AV_SAMPLE_FMT_S16;
    (*output_codec_context)->bit_rate       = OUTPUT_BIT_RATE;
    if ((*output_format_context)->oformat->flags & AVFMT_GLOBALHEADER)
        (*output_codec_context)->flags |= CODEC_FLAG_GLOBAL_HEADER;
    if ((error = avcodec_open2(*output_codec_context, output_codec, NULL)) < 0) {
        fprintf(stderr, "Could not open output codec (error '%s')\n",
                get_error_text(error));
        goto cleanup;
    }
    return 0;
cleanup:
    avio_close((*output_format_context)->pb);
    avformat_free_context(*output_format_context);
    *output_format_context = NULL;
    return error < 0 ? error : AVERROR_EXIT;
}
static void init_packet(AVPacket *packet)
{
    av_init_packet(packet);
    packet->data = NULL;
    packet->size = 0;
}
static int init_input_frame(AVFrame **frame)
{
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate input frame\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}
static int init_resampler(AVCodecContext *input_codec_context,
                          AVCodecContext *output_codec_context,
                          AVAudioResampleContext **resample_context)
{
    if (input_codec_context->sample_fmt != output_codec_context->sample_fmt ||
        input_codec_context->channels != output_codec_context->channels) {
        int error;
        if (!(*resample_context = avresample_alloc_context())) {
            fprintf(stderr, "Could not allocate resample context\n");
            return AVERROR(ENOMEM);
        }
        av_opt_set_int(*resample_context, "in_channel_layout",
                       av_get_default_channel_layout(input_codec_context->channels), 0);
        av_opt_set_int(*resample_context, "out_channel_layout",
                       av_get_default_channel_layout(output_codec_context->channels), 0);
        av_opt_set_int(*resample_context, "in_sample_rate",
                       input_codec_context->sample_rate, 0);
        av_opt_set_int(*resample_context, "out_sample_rate",
                       output_codec_context->sample_rate, 0);
        av_opt_set_int(*resample_context, "in_sample_fmt",
                       input_codec_context->sample_fmt, 0);
        av_opt_set_int(*resample_context, "out_sample_fmt",
                       output_codec_context->sample_fmt, 0);
        if ((error = avresample_open(*resample_context)) < 0) {
            fprintf(stderr, "Could not open resample context\n");
            avresample_free(resample_context);
            return error;
        }
    }
    return 0;
}
static int init_fifo(AVAudioFifo **fifo)
{
    if (!(*fifo = av_audio_fifo_alloc(OUTPUT_SAMPLE_FORMAT, OUTPUT_CHANNELS, 1))) {
        fprintf(stderr, "Could not allocate FIFO\n");
        return AVERROR(ENOMEM);
    }
    return 0;
}
static int write_output_file_header(AVFormatContext *output_format_context)
{
    int error;
    if ((error = avformat_write_header(output_format_context, NULL)) < 0) {
        fprintf(stderr, "Could not write output file header (error '%s')\n",
                get_error_text(error));
        return error;
    }
    return 0;
}
static int decode_audio_frame(AVFrame *frame,
                              AVFormatContext *input_format_context,
                              AVCodecContext *input_codec_context,
                              int *data_present, int *finished)
{
    AVPacket input_packet;
    int error;
    init_packet(&input_packet);
    if ((error = av_read_frame(input_format_context, &input_packet)) < 0) {
        if (error == AVERROR_EOF)
            *finished = 1;
        else {
            fprintf(stderr, "Could not read frame (error '%s')\n",
                    get_error_text(error));
            return error;
        }
    }
    if ((error = avcodec_decode_audio4(input_codec_context, frame,
                                       data_present, &input_packet)) < 0) {
        fprintf(stderr, "Could not decode frame (error '%s')\n",
                get_error_text(error));
        av_free_packet(&input_packet);
        return error;
    }
    if (*finished && *data_present)
        *finished = 0;
    av_free_packet(&input_packet);
    return 0;
}
static int init_converted_samples(uint8_t ***converted_input_samples,
                                  AVCodecContext *output_codec_context,
                                  int frame_size)
{
    int error;
    if (!(*converted_input_samples = calloc(output_codec_context->channels,
                                            sizeof(**converted_input_samples)))) {
        fprintf(stderr, "Could not allocate converted input sample pointers\n");
        return AVERROR(ENOMEM);
    }
    if ((error = av_samples_alloc(*converted_input_samples, NULL,
                                  output_codec_context->channels,
                                  frame_size,
                                  output_codec_context->sample_fmt, 0)) < 0) {
        fprintf(stderr,
                "Could not allocate converted input samples (error '%s')\n",
                get_error_text(error));
        av_freep(&(*converted_input_samples)[0]);
        free(*converted_input_samples);
        return error;
    }
    return 0;
}
static int convert_samples(uint8_t **input_data,
                           uint8_t **converted_data, const int frame_size,
                           AVAudioResampleContext *resample_context)
{
    int error;
    if ((error = avresample_convert(resample_context, converted_data, 0,
                                    frame_size, input_data, 0, frame_size)) < 0) {
        fprintf(stderr, "Could not convert input samples (error '%s')\n",
                get_error_text(error));
        return error;
    }
    if (avresample_available(resample_context)) {
        fprintf(stderr, "Converted samples left over\n");
        return AVERROR_EXIT;
    }
    return 0;
}
static int add_samples_to_fifo(AVAudioFifo *fifo,
                               uint8_t **converted_input_samples,
                               const int frame_size)
{
    int error;
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + frame_size)) < 0) {
        fprintf(stderr, "Could not reallocate FIFO\n");
        return error;
    }
    if (av_audio_fifo_write(fifo, (void **)converted_input_samples,
                            frame_size) < frame_size) {
        fprintf(stderr, "Could not write data to FIFO\n");
        return AVERROR_EXIT;
    }
    return 0;
}
static int read_decode_convert_and_store(AVAudioFifo *fifo,
                                         AVFormatContext *input_format_context,
                                         AVCodecContext *input_codec_context,
                                         AVCodecContext *output_codec_context,
                                         AVAudioResampleContext *resampler_context,
                                         int *finished)
{
    AVFrame *input_frame = NULL;
    uint8_t **converted_input_samples = NULL;
    int data_present;
    int ret = AVERROR_EXIT;
    if (init_input_frame(&input_frame))
        goto cleanup;
    if (decode_audio_frame(input_frame, input_format_context,
                           input_codec_context, &data_present, finished))
        goto cleanup;
    if (*finished && !data_present) {
        ret = 0;
        goto cleanup;
    }
    if (data_present) {
        if (init_converted_samples(&converted_input_samples, output_codec_context,
                                   input_frame->nb_samples))
            goto cleanup;
        if (convert_samples(input_frame->extended_data, converted_input_samples,
                            input_frame->nb_samples, resampler_context))
            goto cleanup;
        if (add_samples_to_fifo(fifo, converted_input_samples,
                                input_frame->nb_samples))
            goto cleanup;
        ret = 0;
    }
    ret = 0;
cleanup:
    if (converted_input_samples) {
        av_freep(&converted_input_samples[0]);
        free(converted_input_samples);
    }
    av_frame_free(&input_frame);
    return ret;
}
static int init_output_frame(AVFrame **frame,
                             AVCodecContext *output_codec_context,
                             int frame_size)
{
    int error;
    if (!(*frame = av_frame_alloc())) {
        fprintf(stderr, "Could not allocate output frame\n");
        return AVERROR_EXIT;
    }
    (*frame)->nb_samples     = frame_size;
    (*frame)->channel_layout = output_codec_context->channel_layout;
    (*frame)->format         = output_codec_context->sample_fmt;
    (*frame)->sample_rate    = output_codec_context->sample_rate;
    if ((error = av_frame_get_buffer(*frame, 0)) < 0) {
        fprintf(stderr, "Could allocate output frame samples (error '%s')\n",
                get_error_text(error));
        av_frame_free(frame);
        return error;
    }
    return 0;
}
static int encode_audio_frame(AVFrame *frame,
                              AVFormatContext *output_format_context,
                              AVCodecContext *output_codec_context,
                              int *data_present)
{
    AVPacket output_packet;
    int error;
    init_packet(&output_packet);
    if ((error = avcodec_encode_audio2(output_codec_context, &output_packet,
                                       frame, data_present)) < 0) {
        fprintf(stderr, "Could not encode frame (error '%s')\n",
                get_error_text(error));
        av_free_packet(&output_packet);
        return error;
    }
    if (*data_present) {
        if ((error = av_write_frame(output_format_context, &output_packet)) < 0) {
            fprintf(stderr, "Could not write frame (error '%s')\n",
                    get_error_text(error));
            av_free_packet(&output_packet);
            return error;
        }
        av_free_packet(&output_packet);
    }
    return 0;
}
static int load_encode_and_write(AVAudioFifo *fifo,
                                 AVFormatContext *output_format_context,
                                 AVCodecContext *output_codec_context)
{
    AVFrame *output_frame;
    const int frame_size = FFMIN(av_audio_fifo_size(fifo),
                                 output_codec_context->frame_size);
    int data_written;
    if (init_output_frame(&output_frame, output_codec_context, frame_size))
        return AVERROR_EXIT;
    if (av_audio_fifo_read(fifo, (void **)output_frame->data, frame_size) < frame_size) {
        fprintf(stderr, "Could not read data from FIFO\n");
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    if (encode_audio_frame(output_frame, output_format_context,
                           output_codec_context, &data_written)) {
        av_frame_free(&output_frame);
        return AVERROR_EXIT;
    }
    av_frame_free(&output_frame);
    return 0;
}
static int write_output_file_trailer(AVFormatContext *output_format_context)
{
    int error;
    if ((error = av_write_trailer(output_format_context)) < 0) {
        fprintf(stderr, "Could not write output file trailer (error '%s')\n",
                get_error_text(error));
        return error;
    }
    return 0;
}
int main(int argc, char **argv)
{
    AVFormatContext *input_format_context = NULL, *output_format_context = NULL;
    AVCodecContext *input_codec_context = NULL, *output_codec_context = NULL;
    AVAudioResampleContext *resample_context = NULL;
    AVAudioFifo *fifo = NULL;
    int ret = AVERROR_EXIT;
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <input file> <output file>\n", argv[0]);
        exit(1);
    }
    av_register_all();
    if (open_input_file(argv[1], &input_format_context,
                        &input_codec_context))
        goto cleanup;
    if (open_output_file(argv[2], input_codec_context,
                         &output_format_context, &output_codec_context))
        goto cleanup;
    if (init_resampler(input_codec_context, output_codec_context,
                       &resample_context))
        goto cleanup;
    if (init_fifo(&fifo))
        goto cleanup;
    if (write_output_file_header(output_format_context))
        goto cleanup;
    while (1) {
        const int output_frame_size = output_codec_context->frame_size;
        int finished                = 0;
        while (av_audio_fifo_size(fifo) < output_frame_size) {
            if (read_decode_convert_and_store(fifo, input_format_context,
                                              input_codec_context,
                                              output_codec_context,
                                              resample_context, &finished))
                goto cleanup;
            if (finished)
                break;
        }
        while (av_audio_fifo_size(fifo) >= output_frame_size ||
               (finished && av_audio_fifo_size(fifo) > 0))
            if (load_encode_and_write(fifo, output_format_context,
                                      output_codec_context))
                goto cleanup;
        if (finished) {
            int data_written;
            do {
                if (encode_audio_frame(NULL, output_format_context,
                                       output_codec_context, &data_written))
                    goto cleanup;
            } while (data_written);
            break;
        }
    }
    if (write_output_file_trailer(output_format_context))
        goto cleanup;
    ret = 0;
cleanup:
    if (fifo)
        av_audio_fifo_free(fifo);
    if (resample_context) {
        avresample_close(resample_context);
        avresample_free(&resample_context);
    }
    if (output_codec_context)
        avcodec_close(output_codec_context);
    if (output_format_context) {
        avio_close(output_format_context->pb);
        avformat_free_context(output_format_context);
    }
    if (input_codec_context)
        avcodec_close(input_codec_context);
    if (input_format_context)
        avformat_close_input(&input_format_context);
    return ret;
}
