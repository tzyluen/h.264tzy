h.264tzy - H.264/MPEG-4 AVC (or Part 10) reader
===============================================
Based on popular open source [x264][1] library and reference to [MP3][2] file format and header.


##### Required Packages
On debian, use backports to get the latest libs:

    sudo apt-get -t wheezy-backports install libavformat-dev libavutil-dev libav-tools
    sudo apt-get -t wheezy-backports libtag1-dev

because earlier versions of libtag1-dev prior to v1.9.1 did not support `prop->codec()`:

    TagLib::MP4::Properties::Codec codec = prop->codec();


##### Notes
1. Demux a mp4 file into raw H.264 and mp3 respectively:

        avconv -i SerenityHDDVDTrailer.mp4 -c:v copy -bsf h264_mp4toannexb -an out.h264
        avconv -i SerenityHDDVDTrailer.mp4 -f mp3 -b 192k -vn out.mp3


##### High-Level steps to decode a h264 stream.
1. register all the codecs using the `avcodec_register_all()` function.
2. find the suitable decoder using `avcodec_find_decoder(AV_CODEC_ID_H264)`.
3. once found a decoder, create a codec context which keeps track of the general state of the decoding process.
4. open the file and initialize the h264 parser using av_parser_init(AV_CODEC_ID_H264).
5. use `av_parser_parse2()` to decode the h264 bitstream.
6. when it finds a complete packet, we decode the frame using `avcodec_decode_video2()` and call the callback function which is passed to the constructor of the H264_Decoder class


##### References
----------------
1. http://en.wikipedia.org/wiki/H.264/MPEG-4_AVC
2. http://support.apple.com/kb/ht1425
3. http://stackoverflow.com/questions/16772558/what-is-ffmpeg-avcodec-x264
4. http://mpgedit.org/mpgedit/mpeg_format/MP3Format.html
5. http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm


[1]:http://www.videolan.org/developers/x264.html
[2]:http://en.wikipedia.org/wiki/MP3#mediaviewer/File:Mp3filestructure.svg
[3]:http://mpgedit.org/mpgedit/mpeg_format/MP3Format.html
[4]:http://mpgedit.org/mpgedit/mpeg_format/mpeghdr.htm
