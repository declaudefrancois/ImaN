#ifndef PNG_H
#define PNG_H

char isPNG();

typedef struct PNGMeta
{
	int width;
	int height;
	char bitDepth;
	char colorType;
	char compressionMethod;
	char filterMethod;
	char interlaceMethod;
} PNGMeta_t;

#endif /** PNG_H */
