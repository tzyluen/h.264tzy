#define _BSD_SOURCE
#include <endian.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "mp3.h"


int main(int argc, char **argv)
{
    FILE *f = fopen(argv[1], "r");
    printf("-I- %s\n", argv[1]);

    if (read_header(f))
        return EXIT_FAILURE;

    printf("(%d)\n", __LINE__);

    return EXIT_SUCCESS;
}


void showbits(unsigned int bits)
{
    int x;
    for( x =0; x < sizeof bits << 3; x++ )
        printf("%d", !!(bits  & (1<<x)) );
    printf("\n");
}


int unpacktagsize(ID3tagv2_t id3tag)
{
    int a, b, c, d;
    a = id3tag.size & 0x7F000000;
    b = id3tag.size & 0x007F0000;
    c = id3tag.size & 0x00007F00;
    d = id3tag.size & 0x0000007F;

    return a>>3 | b>>2 | c>>1 | d>>0;
}


int read_ID3tag(FILE *f, uint8_t *version)
{
    //ID3tagv1_t *id3tag = malloc(sizeof(ID3tagv1_t));
    ID3tagv2_t *id3tag = malloc(sizeof(ID3tagv2_t));
    rewind(f);  /* force file point to the beginning */

    if (fread(id3tag->tagid, 1, sizeof(id3tag->tagid), f) < 1)
        return -1;

    if (fread(&id3tag->tagver, 1, sizeof(uint16_t), f) < 1) {
        //id3tag->tagver = htobe16(id3tag->tagver);
        return -1;
    }

    if (fread(&id3tag->flags, 1, sizeof(char), f) < 1)
        return -1;

    if (fread(&id3tag->size, 1, sizeof(uint64_t), f) < 1) {
        //id3tag->size = htobe64(id3tag->size);
        return -1;
    }


    /* ID3v2 tag begin with ID3 */
    if (! memcmp(id3tag->tagid, "ID3", 3)) {
        fseek(f, unpacktagsize(*id3tag), SEEK_CUR);
    } else {
        rewind(f);  /* not a valid ID3v2 tag; reset */
        /* ID3v1 tag located at the end of file */
        //TODO: ID3v1
        //fseek(f, -, SEEK_END);
    }

    return 0;
}


int read_header(FILE *f)
{
    int i;
    ID3tagv2_t *id3tag = read_ID3tag(f, (uint8_t *) 2);

    //printf("(%d) %s, %"PRId16", %c, %"PRId64"\n",
    //        __LINE__, id3tag->tagid, id3tag->tagver, id3tag->flags, id3tag->size);

    printf("(%d) %s\n", __LINE__, *id3tag->tagid);

    for (i = 0; i < sizeof(MP3header_masks); ++i) {
        //fseek(f, masks[i], SEEK_SET);
        //fread(buf, masks[i]);
    }

    return 0;
}
