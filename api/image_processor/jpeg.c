#include "image.h"
#include "jpeg.h"


/**
 * isJPEG - Test if the file is a JPEG image.
 *
 * @fd: A file descriptor to the file to test.
 *
 * Return: 1 if the file is a JPEG image,
 *          0 if it is not and -1 if an error
 *          happened.
 */
int isJPEG(int fd)
{
	char magic[4]; /** For SOI and APP0 markers. */

	if ((pread(fd, magic, 4, 0) != 4))
	{
		fprintf(stderr, "Error while reading image headers.");
		close(fd);
		return (-1);
	}

	/**
	 *
	 * TODO: add support for different application
	 * segements.
	 *
	 * For now we only support JFIF images.
	 */
	return (0xFF == (magic[0] & 0xFF) &&
			SOI == (magic[1] & 0xFF) &&
			0xFF == (magic[2] & 0xFF) &&
			APP0 == (magic[3] & 0xFF));
}


/**
 * decodeJPEG - Decodes a JPEG file from an open file
 *              descriptor.
 * @fd: A file descriptor to the image file.
 */
t_image *decodeJPEG(int fd)
{
	char bytes[2];
	t_image *image;
	t_jpeg *jpeg;

	if (!isJPEG(fd)) {
		printf("The image is not a Jpeg file.");
		close(fd);
		exit(EXIT_FAILURE);
	}

	image = NULL;

	if (init_jpeg(&jpeg) == -1)
	{
		printf("Error when initializing jpeg struct");
		close(fd);
		return (NULL);
	}

	/**
	 * Set the offset of the file descriptor
	 * to the APP0 marker.
	 */
	lseek(fd, 2, SEEK_SET);

	read_marker(fd, bytes);
	while (EOI != (bytes[1] & 0xFF))
	{
		handle_marker(bytes[1] & 0xFF, fd, jpeg);
		read_marker(fd, bytes);
	}
	printf("EOI, encountered : %02x\n", bytes[1] & 0xFF);


	/**
	 * Decode the mcus and store the
	 * decoded data in the image struct.
	 */
	decode_mcus(jpeg, image);

	close(fd);
	free_jpeg(jpeg);

	return (image);
}

/**
 * read_marker - Tries to read 2 bytes marker data
 *               in marker.
 * @fd: Image file descriptor.
 * @marker: A char pointer with a least 2 bytes of size.
 */
void read_marker(int fd, char *marker)
{
	int i;

	if(read(fd, marker, 2) != 2)
	{
		fprintf(stderr, "Error while reading image data.");
		close(fd);
		exit(EXIT_FAILURE);
	}

	printf("\nIMAGE MARKER => ");
	for (i = 0; i < 2; i++)
		printf("%02x ", marker[i] & 0xFF);
	printf("\n");


	if (0xFF != (marker[0] & 0xFF))
	{
		printf("Missing marker\n");
		close(fd);
		exit(EXIT_FAILURE);
	}
}

/**
 * handle_marker - Handles a single image marker.
 * @code: The marker code.
 * @fd: The file descriptor to the openned image.
 * @jpeg: A pointer to a JPEG struct.
 *
 * Return: The last cursor of the file descriptor
 *         not handled by the handler.
 */
void handle_marker(int code, int fd, t_jpeg *jpeg)
{
	int i;

	/**
	 * We only care about critical markers.
	 */
	t_marker_handler handlers[] = {
		{SOI, NULL},
		{APP0, handle_APP0},
		{DQT, handle_DQT},
		{DHT, handle_DHT},
		{SOS, handle_SOS},
		{SOF0, handle_SOF0},
		{SOF1, handle_SOF1},
		{SOF2, handle_SOF2},
		{SOF3, handle_SOF3},
		{DRI, handle_DRI},
		{EOI, NULL},
	};

	i = 0;
	while (i < 11 && handlers[i].marker)
	{
		if (handlers[i].marker == code)
		{
			/** Marker without segment. */
			if (!handlers[i].func)
				return;

			printf("Calling handler for maker %02x\n", handlers[i].marker);
			return (handlers[i].func(fd, jpeg));
		}
		i++;
	}

	/**
	 * We can skip the non critical
	 * marker segements.
	 */
	skip_marker(fd);
}


/**
 * decode_mcus - Decodes the raw mcus data in the jpeg struct
 *               and stores the decoded data in the image struct.
 * @jpeg: A pointer to the jpeg struct containing the mcus to decode.
 * @image: A pointer to the image struct.
 *
 * Return: A pointer to the all the decoded MCUs
 *         or NULL if the decoding failed.
 */
t_MCU *decode_mcus(t_jpeg *jpeg, t_image *image)
{
	int mcus_per_row, mcus_per_col, mcus_count, i, j;
	t_mcu *mcus;
	int *mcu_component, prev_dc[3];

	generate_all_huffcodes(jpeg);

	mcus_per_row = (jpeg->samples_per_line + 7) / 8;
	mcus_per_col = (jpeg->lines_count + 7) / 8;
	mcus_count = mcus_per_row * mcus_per_col;

	mcus = malloc(mcus_count * sizeof(t_mcu));
	if (mcus == NULL)
		return (NULL);

	printf(">>>>>>>>>>> Decoding %d MCUs\n", mcus_count);
	for (i = 0; i < mcus_count; i++)
	{
		printf("Decoding %d-th mcu\n", i);
		if (jpeg->restart_interval != 0 && i % jpeg->restart_interval == 0)
		{
			prev_dc[0] = 0;
			prev_dc[1] = 0;
			prev_dc[2] = 0;
		}

		for (j = 0; j < jpeg->components_count; j++)
		{
			printf("Next component %d\n", j);
			if(get_mcu_component(j, mcus[i], mcu_component) == NULL)
			{
				free(mcus);
				return (NULL);
			}
			printf("\n\n======= Decoding MCU %d (%d-th component) with dct = %d and act = %d ======\n\n",i, j, jpeg->components[j].dct_selector, jpeg->components[j].act_selector);	
			if (decode_component_mcu(
				&(jpeg->data),
				mcu_component,
				&(prev_dc[j]),
				jpeg->huff_dc_tables[jpeg->components[j].dct_selector],
				jpeg->huff_ac_tables[jpeg->components[j].act_selector]) == -1
			)
			{
				printf("Error when decoding mcus\n");
				free(mcus);
				return (NULL);
			}
			printf("\n\n======= END-- Decoding MCU %d (%d-th component) with dct = %d and act = %d ======\n\n",i, j, jpeg->components[j].dct_selector, jpeg->components[j].act_selector);	
		}
	}

	printf("decoding mcus \n");
}
