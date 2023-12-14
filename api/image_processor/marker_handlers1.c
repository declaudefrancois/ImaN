#include "jpeg.h"

/**
 * handle_SOFO - Handle SOF0 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF0(int fd, t_jpeg *jpeg)
{
	jpeg->mode = SEQUENTIAL_BASELINE;
	handle_frame_header(fd, jpeg);
}


/**
 * handle_SOF1 - Handle SOF1 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF1(int fd, t_jpeg *jpeg)
{
	jpeg->mode = SEQUENTIAL_EXTENDED;
	handle_frame_header(fd, jpeg);
}


/**
 * handle_SOF2 - Handle SOF2 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF2(int fd, t_jpeg *jpeg)
{
	jpeg->mode = PROGRESSIVE;	
	handle_frame_header(fd, jpeg);
}

/**
 * handle_SOF3 - Handle SOF3 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF3(int fd, t_jpeg *jpeg)
{
	jpeg->mode = LOSSLESS;
	handle_frame_header(fd, jpeg);
}

/**
 * handle_frame_header - Handle start of frame segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_frame_header(int fd, t_jpeg *jpeg)
{
	char bytes[2];
	int length, i;

	length = getMarkerLength(fd);

	readBytes(fd, bytes, 1);
	jpeg->precision = bytes[0] & 0xFF;

	readBytes(fd, bytes, 2);
	jpeg->lines_count = ((bytes[0] && 0xFF) << 8) + (bytes[1] && 0xFF);

	readBytes(fd, bytes, 2);
	jpeg->samples_per_line = ((bytes[0] && 0xFF) << 8) + (bytes[1] && 0xFF);

	readBytes(fd, bytes, 1);
	jpeg->components_count = bytes[0] & 0xFF;

	for (i = 0; i < jpeg->components_count; i++)
	{
		readBytes(fd, bytes, 1);
		jpeg->components[i].id = bytes[0] & 0xFF;

		readBytes(fd, bytes, 1);
		jpeg->components[i].h_sampling_factor = bytes[0] & 0xF0;
		jpeg->components[i].v_sampling_factor = bytes[0] & 0x0F;

		readBytes(fd, bytes, 1);
		jpeg->components[i].qt_id = bytes[0] & 0xFF;
	}

	printSOF(jpeg);
}
