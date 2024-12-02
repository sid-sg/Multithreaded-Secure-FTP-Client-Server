#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <zlib.h>

#define CHUNK 16384

void compressFile(FILE *source, FILE *dest) {
    int ret, flush;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // Initialize the zlib stream for compression
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
    if (ret != Z_OK) return;

    // Compress until end of file
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)deflateEnd(&strm);
            return;
        }
        flush = feof(source) ? Z_FINISH : Z_NO_FLUSH;
        strm.next_in = in;

        // Run deflate() on input until output buffer not full
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = deflate(&strm, flush);
            assert(ret != Z_STREAM_ERROR);
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)deflateEnd(&strm);
                return;
            }
        } while (strm.avail_out == 0);
        assert(strm.avail_in == 0);
    } while (flush != Z_FINISH);
    assert(ret == Z_STREAM_END);

    // Clean up
    (void)deflateEnd(&strm);
}

void decompressFile(FILE *source, FILE *dest) {
    int ret;
    unsigned have;
    z_stream strm;
    unsigned char in[CHUNK];
    unsigned char out[CHUNK];

    // Initialize the zlib stream for decompression
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = inflateInit(&strm);
    if (ret != Z_OK) return;

    // Decompress until deflate stream ends or end of file
    do {
        strm.avail_in = fread(in, 1, CHUNK, source);
        if (ferror(source)) {
            (void)inflateEnd(&strm);
            return;
        }
        if (strm.avail_in == 0) break;
        strm.next_in = in;

        // Run inflate() on input until output buffer not full
        do {
            strm.avail_out = CHUNK;
            strm.next_out = out;
            ret = inflate(&strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);
            switch (ret) {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return;
            }
            have = CHUNK - strm.avail_out;
            if (fwrite(out, 1, have, dest) != have || ferror(dest)) {
                (void)inflateEnd(&strm);
                return;
            }
        } while (strm.avail_out == 0);

        // Done when inflate() says it's done
    } while (ret != Z_STREAM_END);

    // Clean up
    (void)inflateEnd(&strm);
}

int main(int argc, char **argv) {
    if (argc != 4) {
        printf("Usage: %s <input file> <compress|decompress> <output file>\n", argv[0]);
        return 1;
    }

    FILE *inFile = fopen(argv[1], "rb");
    FILE *outFile = fopen(argv[3], "wb");

    if (inFile == NULL || outFile == NULL) {
        fprintf(stderr, "Could not open files\n");
        return 1;
    }

    if (strcmp(argv[2], "compress") == 0) {
        compressFile(inFile, outFile);
    } else if (strcmp(argv[2], "decompress") == 0) {
        decompressFile(inFile, outFile);
    } else {
        fprintf(stderr, "Invalid operation\n");
        return 1;
    }

    fclose(inFile);
    fclose(outFile);
    return 0;
}
