#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#define HEADER_SIZE 44

typedef struct {
    char chunkID[4];
    int chunkSize;
    char format[4];
    char subchunk1ID[4];
    int subchunk1Size;
    short audioFormat;
    short numChannels;
    int sampleRate;
    int byteRate;
    short blockAlign;
    short bitsPerSample;
    char subchunk2ID[4];
    int subchunk2Size;
} WAVHeader;

void readWAVHeader(FILE *file, WAVHeader *header) {
    fread(header, sizeof(WAVHeader), 1, file);
}

short* readWAVData(FILE *file, int dataSize) {
    short *data = (short*)malloc(dataSize);
    fread(data, dataSize, 1, file);
    return data;
}

int calculateError(short *data, int length, int start1, int start2) {
    int error = 0;
    for (int i = 0; i < length; i++) {
        int diffL = data[2 * (start1 + i)] - data[2 * (start2 + i)];
        int diffR = data[2 * (start1 + i) + 1] - data[2 * (start2 + i) + 1];
        error += diffL * diffL + diffR * diffR;
    }
    return error;
}

void findRepeatingSegment(short *data, int dataSize, int *start1, int *start2, int *segmentSize) {
    int minError = INT_MAX;
    int bestStart1 = 0, bestStart2 = 0, bestLength = 0;

    for (int i = 0; i < dataSize / 2; i++) {
        int maxLength = (dataSize / 2 - i) / 2;
        for (int j = maxLength / 2; j <= maxLength; j++) {
            int length = j - i;
            int error = calculateError(data, length, i, j);
            if (error < minError) {
                minError = error;
                bestStart1 = i;
                bestStart2 = j;
                bestLength = length;
            }
        }
    }

    *start1 = bestStart1;
    *start2 = bestStart2;
    *segmentSize = bestLength;
}

void saveWAVFile(const char *filename, WAVHeader *header, short *data, int dataSize) {
    FILE *outFile = fopen(filename, "wb");
    fwrite(header, sizeof(WAVHeader), 1, outFile);
    fwrite(data, dataSize, 1, outFile);
    fclose(outFile);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: %s <Chopin.wav> <NewChopin.wav>\n", argv[0]);
        return 1;
    }

    FILE *inFile = fopen(argv[1], "rb");
    if (!inFile) {
        perror("FUCK YOU CANT OPEN INPUT WAV FILE");
        return 1;
    }

    WAVHeader header;
    readWAVHeader(inFile, &header);

    int dataSize = header.subchunk2Size;
    short *data = readWAVData(inFile, dataSize);
    fclose(inFile);

    int start1 = 0, start2 = 0, segmentSize = 0;
    findRepeatingSegment(data, dataSize / sizeof(short), &start1, &start2, &segmentSize);

    int newSize = (start1 + segmentSize) * sizeof(short);
    saveWAVFile(argv[2], &header, data, newSize);

    free(data);

    printf("You are good boy output file is saved as %s\n", argv[2]);

    return 0;
}

/*
#include <stdio.h>

#define LEFT 1 
#define RIGHT 1

int main(){
    printf("%d %d %d", RIGHT, LEFT, LEFT+1);
    return 0;
}
*/
