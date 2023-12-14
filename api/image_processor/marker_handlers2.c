#include "jpeg.h"


/**
 * handle_DRI - Handle Define Restart Interval marker.
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_DRI(int fd, t_jpeg *jpeg)
{
	char bytes[2];
	int length;

	getMarkerLength(fd);

	readBytes(fd, bytes, 2);
	jpeg->restart_interval = (bytes[0] << 8) + bytes[1];
	printf("\n\n\n Restart interval is %d\n\n\n", jpeg->restart_interval);
}
