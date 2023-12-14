#include "png.h"
#include "main.h"

/**
 * isPNG - Tests if the file is a PNG image.
 *
 * Returns: 1 if the file is a PNG image
 *          otherwise 0.
 */
char isPNG(int img_fd)
{
	char *header;
	/** int i; */

	header = malloc(sizeof(8)); /** 8 bytes png header sig */
	if (header == NULL)
		fprintf(stderr, "Malloc failled\n");
	if ((pread(img_fd, header, 8, 0) != 8))
		fprintf(stderr, "Error while reading image headers.");

	/** printf("Image 8 first bytes \n");
        for (i = 0; i < 8; i++)
		printf("%02x ", header[i] & 0xFF);
	printf("\n"); */

	return (0x89 == (header[0] & 0xFF) &&
		0x50 == (header[1] & 0xFF) &&
		0x4E == (header[2] & 0xFF) &&
		0x47 == (header[3] & 0xFF) &&
		0x0D == (header[4] & 0xFF) &&
		0x0A == (header[5] & 0xFF) &&
		0x1A == (header[6] & 0xFF) &&
		0x0A == (header[7] & 0xFF));
}


void loadPNGMeta()
{

}
