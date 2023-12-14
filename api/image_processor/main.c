#include "main.h"
#include "png.h"
#include "jpeg.h"


/**
 * main - Program entry point.
 *
 * Returns: 0 or 1;
 */
int main(int ac, char **av)
{
	char *img_path;
	int img_fd;

	if ((img_fd = open(av[1], O_RDONLY)) < 0) {
		fprintf(stderr, "Unable to open file: %s.\n", img_path);
		printUsageAndExit();
	}

	decodeJPEG(img_fd);
	return (EXIT_SUCCESS);
}

/**
 * printUsageAndExit - Prints the help usage in
 *                     the std error stream and
 *                     exits.
 */
void printUsageAndExit()
{
	fprintf(stderr, "Usage: iman -o <operation> <image file path>.\n");
	fprintf(stderr, "\tWith operation (the operation to perform on the image) ");
	fprintf(stderr, "one of: 'compress', 'toJPEG', 'toPNG'\n");
	exit(EXIT_FAILURE);
}

