#include "jpeg.h"


/**
 * init_jpeg - Allocates memory for a jpeg struct
 *             to be used in the codec.
 *
 * @jpeg: A double pointer to the jpeg struct to
 *          initializes.
 *
 * Return: 1 on success or -1 on failure.
 */
int init_jpeg(t_jpeg **jpeg)
{
	int i;

	if (!jpeg)
		return (-1);

	(*jpeg) = malloc(sizeof(t_jpeg));
	if (*jpeg == NULL)
		return (-1);

	(*jpeg)->data.mcus = NULL;
	(*jpeg)->data.max_size = 0;
	(*jpeg)->data.current_size = 0;
	(*jpeg)->data.byte_cursor = 0;
	(*jpeg)->data.bit_cursor = 0;

	(*jpeg)->precision = 0;
	(*jpeg)->lines_count = 0;
	(*jpeg)->samples_per_line = 0;
	(*jpeg)->components_count = 0;
	(*jpeg)->restart_interval = 0;

	(*jpeg)->start_of_spectral_selection;
	(*jpeg)->end_of_spectral_selection;

	(*jpeg)->sa_high;
	(*jpeg)->sa_low;

	for (i = 0; i < 4; i++)
	{
		(*jpeg)->qt_tables[i].available = 0;
		init_array(64, (*jpeg)->qt_tables[i].coeffs);
	
		(*jpeg)->huff_ac_tables[i].available = 0;
		init_array(162, (*jpeg)->huff_ac_tables[i].huffcodes);
		init_array(162, (*jpeg)->huff_ac_tables[i].huffvals);
		init_array(17, (*jpeg)->huff_ac_tables[i].huffsizes);
	
		(*jpeg)->huff_dc_tables[i].available = 0;
		init_array(162, (*jpeg)->huff_dc_tables[i].huffcodes);
		init_array(162, (*jpeg)->huff_dc_tables[i].huffvals);
		init_array(17, (*jpeg)->huff_dc_tables[i].huffsizes);
	}

	return (1);
}


/**
 * free_jpeg - Frees memory used by the jpeg struct.
 * @jpeg: A pointer to the jpeg struct to free.
 */
void free_jpeg(t_jpeg *jpeg)
{
	free(jpeg->data.mcus);
	free(jpeg);
}


int init_mcus(t_jpeg *jpeg)
{
	if (jpeg->data.mcus != NULL)
		return (1);

	/**
	 * TODO: fix this.
	 * For large file (size >= initial memory)
	 * the program exits with an aborted code dumped.
	 * Allocating a huge memery at the beginning is a temporary fix
	 * before we figure out the problem. 
	 */
	jpeg->data.mcus = (char *) malloc(30000000 * sizeof(char));
	if (jpeg->data.mcus == NULL)
		return (-1);

	jpeg->data.current_size = 0;
	jpeg->data.max_size = 30000000;
	return (1);
}

int realloc_mcus(t_jpeg *jpeg)
{
	jpeg->data.max_size += 10000000;
	jpeg->data.mcus = (char *) realloc(jpeg->data.mcus, jpeg->data.max_size);
	if (jpeg->data.mcus == NULL)
		return (-1);
	return (1);
}


int push_mcus_data(t_jpeg *jpeg, char data)
{
	if (jpeg->data.current_size >= jpeg->data.max_size)
	{
		if (realloc_mcus(jpeg) == -1)
			return (-1);
	}

	jpeg->data.mcus[jpeg->data.current_size - 1] = data;
	jpeg->data.current_size++;

	return (1);
}


/**
 * read_raws_mcus - Reads the entropy coded data and stores them
 *                  in the jpeg struct.
 * @fd: A file descriptor to the file being processed.
 * @jpeg: A pointer to a jpeg struct.
 */
void read_raw_mcus(int fd, t_jpeg *jpeg)
{
	char prev, current;

	init_mcus(jpeg);

	current = read_byte(fd);
	while(1)
	{
		prev = current;
		current = read_byte(fd);
		if ((prev & 0xFF) == 0xFF)
		{
			if ((current & 0xFF) == EOI)
				break;
			else if ((current & 0xFF) == 0x00)
			{
				if (push_mcus_data(jpeg, prev) == -1)
				{
					close(fd);
					free_jpeg(jpeg);
					printf("Error when pushing mcus data.\n");
					exit(1);
				}

				current = read_byte(fd);
			}
			else if ((current & 0xFF) >= RST0 && (current & 0xFF) <= RST7)
			{
				/**
				 * RSTm markers are not followed by a segment.
				 */
				current = read_byte(fd);
			}
			else if ((current & 0xFF) == 0xFF)
				continue;
		}
		else if (push_mcus_data(jpeg, prev) == -1)
		{
			close(fd);
			free_jpeg(jpeg);
			printf("Error when pushing mcus data.\n");
			exit(1);
		}
	}

	lseek(fd, -2 , SEEK_CUR);
	printf("END -- Reading the MCUs\n");
}

/**
 * init_array - Set all items of array to zero.
 * @size: The array size.
 * @array: The array.
 */
void init_array(int size, int *array)
{
	int i;

	for (i = 0; i < size; i++)
		array[i] = 0;
}
