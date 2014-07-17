#include <stdio.h>
#include <stdlib.h>
#include <taglib/tag.h>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <string>
#include <taglib/mp4properties.h>
#include "mp4.h"

using namespace std;

int main(int argc, char **argv)
{
    const char *mp4file = argv[1];
    TagLib::FileRef f(mp4file);
    p_mp4_header(f);

    return EXIT_SUCCESS;
}


/**
 * MPEG-4:
 * Video Codecs:
 *    H.264 Baseline: avc1.42E0xx, where xx is the AVC level
 *    H.264 Main: avc1.4D40xx, where xx is the AVC level
 *    H.264 High: avc1.6400xx, where xx is the AVC level
 *    MPEG-4 Visual Simple Profile Level 0: mp4v.20.9
 *    MPEG-4 Visual Advanced Simple Profile Level 0: mp4v.20.240
 * Audio Codecs:
 *    Low-Complexity AAC: mp4a.40.2
 */
void p_mp4_header(TagLib::FileRef f)
{
    TagLib::String title = f.tag()->title();
    TagLib::String artist = f.tag()->artist();
    TagLib::String album = f.tag()->album();
    TagLib::String comment = f.tag()->comment();
    TagLib::String genre = f.tag()->genre();
    TagLib::uint year = f.tag()->year();
    TagLib::uint track = f.tag()->track();
    TagLib::MP4::Properties *prop = (TagLib::MP4::Properties*) f.audioProperties();
    TagLib::MP4::Properties::Codec codec = prop->codec();

    printf("Metadata\n");
    printf("Artist    :  %s\nTitle     :  %s\nAlbum     :  %s\nGenre     :  %s\n",
            artist.toCString(true),
            title.toCString(true),
            album.toCString(true),
            genre.toCString(true));
    printf("Comment   :  %s\nYear      :  %d\nTrack     :  %d\n",
            comment.toCString(true),
            year, track);
    printf("Length    :  %d\nBitrate   :  %d\nS' Rate   :  %d\nChannels  :  %d\n",
            prop->length(),
            prop->bitrate(),
            prop->sampleRate(),
            prop->channels());
    printf("BPS       :  %d\nEncrypted :  %d\nAudio Codec  :  %s\n",
            prop->bitsPerSample(),
            prop->isEncrypted(),
            mp4_codec_str[codec]);
}
