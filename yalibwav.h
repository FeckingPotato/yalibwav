#include <stdio.h>
struct chunk {
    char id[4];
    unsigned int size;
    char *data;
};

struct wavFmt {
    unsigned short audioFormat;
    unsigned short channels;
    unsigned int sampleRate;
    unsigned int byteRate;
    unsigned short blockAllign;
    unsigned short bitsPerSample;
    char *etc;
};

typedef struct {
    char fileId[4];
    unsigned int fileSize;
    char format[4];
    unsigned int fmtSize;
    struct wavFmt fmt;
    unsigned int audioSize;
    short *audio;
    unsigned int etcChunksCount;
    struct chunk *etcChunks;
} WAV;

unsigned short wavFileRead(FILE *file, WAV *wav);
unsigned short wavFileWrite(FILE *file, WAV wav);
