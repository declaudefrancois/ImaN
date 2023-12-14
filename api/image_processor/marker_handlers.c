#include "jpeg.h"

/**
 * handle_APP0 - handle `APP0` marker-segment.
 *
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_APP0(int fd, t_jpeg *jpeg)
{
	skip_marker(fd);
	printAPP0(jpeg);
}


/**
 * handle_DQT - Handles the Define Quantization Table (DQT)
 *              marker segment.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_DQT(int fd, t_jpeg *jpeg)
{
	char bytes[2] = {};
	int length, i, table_id, precision;

	length = getMarkerLength(fd);
	length -= 2;
	do {
		readBytes(fd, bytes, 1);
		length -= 1;

		/**
		 * 1 for 1 byte per coeff.
		 * 2 for 2 bytes per coeff.
		 */
		precision = (bytes[0] & 0xF0) + 1;

		table_id = (bytes[0] & 0x0F);

		if (table_id > 3)
		{
			printf("Invalid Quantization table destination identifier: %d", table_id);
			close(fd);
			exit(1);
		}

		jpeg->qt_tables[table_id].available = 1;
		for (i = 0; i < 64; i++)
		{
			readBytes(fd, bytes, precision);
			jpeg->qt_tables[table_id].coeffs[i] = precision == 2
				? ((bytes[0] << 8) + (bytes[1] & 0xFF))
				: (bytes[0] & 0xFF);
		}
		length -= precision * 64;
	} while (length > 0);

	printDQT(jpeg);
}


/**
 * handle_DHT - Handles a Define Huffman Table (DHT)
 *              marker segment.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_DHT(int fd, t_jpeg *jpeg)
{
	char bytes[2] = {};
	int length, table_id, table_class, i, symbols_count;
	int precision;
	t_Huffman_table *huff_tables;

	length = getMarkerLength(fd);
	length -= 2;

	huff_tables = NULL;
	while (length > 0)
	{
		readBytes(fd, bytes, 1);

		table_class = (bytes[0] >> 4) & 0xFF;
		printf("Table class %d\n", table_class);
		huff_tables = table_class
			? jpeg->huff_ac_tables
			: jpeg->huff_dc_tables;

		table_id = (bytes[0] & 0x0F);
		if (table_id > 3)
		{
			printf("Invalid Huffman table destination identifier: %d", table_id);
			close(fd);
			exit(1);
		}

		huff_tables[table_id].available = 1;
		huff_tables[table_id].huffsizes[0] = 0;
		symbols_count = 0;

		/**
		 * Read the 16-bytes huffman BITS (counts of symbol's length).
		 * An store their offset.
		 */
		for (i = 1; i < 17; i++)
		{
			readBytes(fd, bytes, 1);
			symbols_count += bytes[0] & 0xFF;
			huff_tables[table_id].huffsizes[i] = symbols_count;
		}

		/**
		 * Read all the Symbols.
		 */
		for (i = 0; i < symbols_count; i++)
		{
			readBytes(fd, bytes, 1);
			huff_tables[table_id].huffvals[i] = bytes[0] & 0xFF;
		}

		length -= 17 + symbols_count;
	}

	printDHT(jpeg);
}


/**
 * handle_SOS - Handles Start Of Scan marker.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to jpeg struct.
 */
void handle_SOS(int fd, t_jpeg *jpeg)
{
	int length, components, i, comp_id, act_seletor, dct_seletor;
	char bytes[3];

	if (jpeg->mode == JPEG_MODE_NOT_SET)
	{
		printf("Fatal attempted to read a scan header before the frame header.");
		close(fd);
		exit(1);
	}

	length = getMarkerLength(fd);

	readBytes(fd, bytes, 1);
	components = bytes[0] & 0xFF;
	for (i = 0; i < components; i++)
	{
		readBytes(fd, bytes, 2);
		comp_id = bytes[0] & 0xFF;

		printf("component id = %d, dtc_id = %d, act_id = %d\n", comp_id, (bytes[1] >> 4) & 0xFF, bytes[1] & 0x0F);

		jpeg->components[comp_id - 1].dct_selector = (bytes[1] >> 4) & 0xFF;
		jpeg->components[comp_id - 1].act_selector = bytes[1] & 0x0F;
	}

	readBytes(fd, bytes, 3);

	jpeg->start_of_spectral_selection = bytes[0] & 0xFF;
	jpeg->end_of_spectral_selection = bytes[1] & 0xFF;
	jpeg->sa_high = (bytes[2] >> 4) & 0xFF;
	jpeg->sa_low = bytes[2] & 0x0F;

	printf("6 + 2 * Ns = %d and length = %d\n", 6 + 2 * components, length);

	printSOS(jpeg);

	read_raw_mcus(fd, jpeg);
}
