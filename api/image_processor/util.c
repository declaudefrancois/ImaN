#include "jpeg.h"


/**
 * skip_marker - Reads the marker length and skips
 *               the segment delimited by the length.
 * @fd: A open file descriptor to the file.
 */
void skip_marker(int fd)
{
        int length;

        length = getMarkerLength(fd);
        printf("Skipping %d bytes\n", length);
        lseek(fd, length - 2, SEEK_CUR);
}


/**
 * readBytes - Reads `count` bytes from the file referenced by
 *             the file descriptor and store them in buff.
 * @fd: A open file descriptor to the file.
 * @buff: A char pointer with enough space to store `count` bytes.
 * @count: The number of bytes to read.
 */
void readBytes(int fd, char *buff, int count)
{
        if (read(fd, buff, count) != count)
        {
                close(fd);
                printf("Unable to read %d byte(s)", count);
                exit(1);
        }
}


/**
 * read_byte - Reads one byte of data from the file descritor
 *             and return it as a char.
 *
 * @fd: A open file descriptor to the file.
 * Return: A char representing the read byte.
 */
char read_byte(int fd)
{
        char bytes[1];

        readBytes(fd, bytes, 1);
        return (bytes[0]);
}


/**
 * getMarkerLength - Reads and return the a marker's length.
 *                   must be called just after a segemnt-marker
 *                   has been found.
 * @fd: A open file descriptor to the file.
 * Return: The marker's length.
 */
int getMarkerLength(int fd)
{
        char length[2];

        readBytes(fd, length, 2);
        return (((length[0] & 0xFF) << 8) + (length[1] & 0xFF));
}
