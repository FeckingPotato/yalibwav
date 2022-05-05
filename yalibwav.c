#include "yalibwav.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseStr(FILE *file, char *result, unsigned int bytes) {
    for (int i = 0; i < bytes; i++) {
        *(result+i) = (char) fgetc(file);
    }
}

int parseInt(FILE *file, unsigned short bytes) {
    char temp[bytes];
    for (int i = 0; i < bytes; i++) {
        temp[i] = (char) fgetc(file);
    }
    int *result = (int*) temp;
    return *result;
}

struct chunk *chunksRead(FILE *file, unsigned int fileSize, unsigned int *chunksSize) {
    struct chunk *chunks = NULL;
    unsigned int position = ftell(file);
    unsigned int i = 0;
    while (position < fileSize) {
        (*chunksSize)++;
        chunks = realloc(chunks, sizeof(struct chunk) * (*chunksSize));
        parseStr(file, chunks[i].id, 4);
        chunks[i].size = parseInt(file, 4);
        chunks[i].data = (char *) malloc(chunks[i].size);
        parseStr(file, chunks[i].data, chunks[i].size);
        position = ftell(file);
        i++;
    }
    return chunks;
}

unsigned short wavFileRead(FILE *file, WAV *wav) {
    if (file == NULL) {
        return -1; //bad input file
    }
    parseStr(file, wav->fileId, 4);
    if (wav->fileId[0] != 'R' || wav->fileId[1] != 'I' || wav->fileId[2] != 'F' || wav->fileId[3] != 'F') {
        return 1; //input is not riff
    }
    wav->fileSize = parseInt(file, 4);
    fseek(file, 0, SEEK_END);
    if (wav->fileSize + 8 != ftell(file)) {
        return 2; //file size mismatch
    }
    fseek(file, 8, SEEK_SET);
    parseStr(file, wav->format, 4);
    if (wav->format[0] != 'W' || wav->format[1] != 'A' || wav->format[2] != 'V' || wav->format[3] != 'E') {
        return 3; //input is not wav
    }
    unsigned int chunksSize = 0;
    struct chunk *chunks = chunksRead(file, wav->fileSize, &chunksSize);
    unsigned short fmtExists = 0;
    unsigned short dataExists = 0;
    wav->audio = NULL;
    wav->etcChunksCount = 0;
    wav->etcChunks = NULL;
    for (unsigned int i = 0; i < chunksSize; i++) {
        if (chunks[i].id[0] == 'f' && chunks[i].id[1] == 'm' && chunks[i].id[2] == 't' && chunks[i].id[3] == '\40') {
            fmtExists = 1;
            wav->fmtSize = chunks[i].size;
            wav->fmt.audioFormat = (unsigned short) ((unsigned char) chunks[i].data[0] | (unsigned char) chunks[i].data[1] << 8);
            wav->fmt.channels = (unsigned short) (chunks[i].data[2] | chunks[i].data[3] << 8);
            wav->fmt.sampleRate = (unsigned int) (
                    (unsigned char) chunks[i].data[4]
                    | (unsigned char) chunks[i].data[5] << 8
                    | (unsigned char) chunks[i].data[6] << 16
                    | (unsigned char) chunks[i].data[7] << 24);
            wav->fmt.byteRate = (unsigned int) (
                    (unsigned char) chunks[i].data[8]
                    | (unsigned char) chunks[i].data[9] << 8
                    | (unsigned char) chunks[i].data[10] << 16
                    |(unsigned char) chunks[i].data[11] << 24);
            wav->fmt.blockAllign = (unsigned short) ((unsigned char) chunks[i].data[12]| (unsigned char) chunks[i].data[13] << 8);
            wav->fmt.bitsPerSample = (unsigned short) ((unsigned char) chunks[i].data[14] | (unsigned char) chunks[i].data[15] << 8);
        }
        else if (chunks[i].id[0] == 'd' && chunks[i].id[1] == 'a' && chunks[i].id[2] == 't' && chunks[i].id[3] == 'a') {
            dataExists = 1;
            wav->audioSize = chunks[i].size;
            wav->audio = (short *) malloc(wav->audioSize);
            memcpy(wav->audio, chunks[i].data, wav->audioSize);
        }
        else {
            wav->etcChunksCount++;
            wav->etcChunks = realloc(wav->etcChunks, sizeof(struct chunk) * (wav->etcChunksCount));
            memcpy(wav->etcChunks[wav->etcChunksCount-1].id, chunks[i].id, 4);
            wav->etcChunks[wav->etcChunksCount-1].size = chunks[i].size;
            wav->etcChunks[wav->etcChunksCount-1].data = (char *) malloc(wav->etcChunks[wav->etcChunksCount-1].size);
            memcpy(wav->etcChunks[wav->etcChunksCount-1].data, chunks[i].data, wav->etcChunks[wav->etcChunksCount-1].size);
        }
    }
    free(chunks);
    if (!fmtExists) {
        return 5; //no fmt chunk
    }
    if (!dataExists) {
        return 6; //no data chunk
    }
    return 0;
}

unsigned short wavFileWrite(FILE *file, WAV wav) {
    if (file == NULL) {
        return -1; //bad output file
    }
    fwrite(&wav, 1, 12, file);
    fwrite("fmt\40", 1, 4, file);
    fwrite(&wav.fmtSize, 1, 4, file);
    fwrite(&wav.fmt, 1, 16, file);
    fwrite(wav.fmt.etc, 1, wav.fmtSize-16, file);
    fwrite("data", 1, 4, file);
    fwrite(&wav.audioSize, 1, 4, file);
    fwrite(wav.audio, 1, wav.audioSize, file);
    for (unsigned int i = 0; i < wav.etcChunksCount; i++) {
        fwrite(wav.etcChunks[i].id, 1, 4, file);
        fwrite(&wav.etcChunks[i].size, 1, 4, file);
        fwrite(wav.etcChunks[i].data, 1, wav.etcChunks[i].size, file);
    }
    return 0;
}
