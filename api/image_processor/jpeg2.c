#include "jpeg.h"

const int zigzag_idx[] = {
	0,  1,  8,  16, 9,  2,  3,  10,
	17, 24, 32, 25, 18, 11, 4,  5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6,  7,  14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

/**
 * generate_huffcodes - Generates Huffman code corresponding
 *                      to code length in the table `table`.
 *
 * @table: A pointer to the Huffman Table to generate codes for.
 */
void generate_huffcodes(t_Huffman_table *table)
{
	unsigned int code, i, j;

	code = 0;
	for (i = 0; i < 16; i++)
	{
		for (j = table->huffsizes[i]; j < table->huffsizes[i + 1]; ++j)
		{
			table->huffcodes[j] = code;
			code += 1;
		}
		code <<= 1;
	}
}


/**
 * generate_all_huffcodes - Generates all possible ac and dc huffman codes.
 * @jpeg: A pointer to a jpeg struct.
 */
void generate_all_huffcodes(t_jpeg *jpeg)
{

	int i;

	for (i = 0; i < 4; i++)
	{
		if (jpeg->huff_dc_tables[i].available)
			generate_huffcodes(&(jpeg->huff_dc_tables[i]));

		if (jpeg->huff_ac_tables[i].available)
			generate_huffcodes(&(jpeg->huff_ac_tables[i]));
	}
}


/**
 * decode_component_mcu - Decoded a single component's MCU.
 * @data: A pointer to the entropy coded data to decode.
 * @color_component: A pointer to the coefficient array of the color
 *                   component to decode.
 * @prev_dc: A pointer to the prevouis DC coefficient of the component.
 * @dc_table: The Huffman DC table for the component.
 * @ac_table: The Huffman Ac table for the component.
 *
 * Return: 1 on success or -1 on failure.
 */
int decode_component_mcu(
	t_Entropy_Coded_data *data,
	int *color_component,
	int *prev_dc,
	t_Huffman_table dc_table,
	t_Huffman_table ac_table)
{
	int length, coeff, i, j, symbol, zeros;

	/**
	 * Decode the DC coefficient.
	 */
	length = read_next_symbol(data, dc_table);
	if (length == -1)
		return (-1);

	coeff = read_next_bits(data, length);
	if (length != 0 && coeff < (1 << (length -1)))
		coeff -= (1 << length) - 1;

	color_component[0] = coeff + (*prev_dc);
	*prev_dc = color_component[0];
	printf("coeff[0] = %d\n", coeff);
	/**
	 * Decode the 63 AC coefficients.
	 */
	for (i = 0; i < 64; i++)
	{
		symbol = read_next_symbol(data, ac_table);
		if (symbol == -1)
			return (-1);

		if (symbol == 0x00)
		{
			printf("0x00 found at %d filling 0s.\n", i);
			printf("next length = %d \n", length = symbol & 0x0F);
			for (; i < 64; i++)
			{
				printf("coeff[%d]=%d\n", i + 1, 0);
				color_component[zigzag_idx[i]] = 0;
			}
			return (1);
		}

		/**
		 * AC coefficient structure.
		 * [4bits nbr of 0][4bits coeff length][length bits coeffs].
		 */
		zeros = symbol == 0xF0 ? 16 : symbol >> 4;
		length = symbol & 0x0F;
		if ((i + zeros) >= 64)
		{
			printf("Number of coefficients exced 64 (%d)\n", i + zeros);
			return (-1);
		}

		printf("zeros=%d, length=%d\n",zeros, length);
		for (j = 0; j < zeros; j++)
		{
			printf("coeff[%d]=%d\n", i + 1, 0);
			color_component[zigzag_idx[i]] = 0;
			i++;
		}

		coeff = 0;
		if (length != 0)
		{
			coeff = read_next_bits(data, length);
			if (coeff == -1)
				return (-1);

			if (coeff < (1 << (length -1)))
				coeff -= (1 << length) - 1;
			color_component[i] = coeff;
		}
		printf("coeff[%d]=%d\n\n", i + 1, coeff);
	}

	return (1);
}


/**
 * get_mcu_component - Gets a single component of an MCU.
 * @id: The component's id.
 * @mcu: The mcu struct.
 * @component: A pointer to store the color component in.
 *
 * Return: A pointer to the coefficients array of the component or
 *         NULL if the id is not 0, 1 or 2.
 */
int *get_mcu_component(int id, t_mcu mcu, int *component)
{
	printf("get_mcu_component %d\n", id);
	if (id < 0 || id > 2)
		return (NULL);

	if (id == 0)
		component = mcu.y;
	else if (id == 1)
		component = mcu.cb;
	else
		component = mcu.cr;

	return (component);
}


/**
 * read_next_symbol - Reads the next symbol in the entropy coded data
 *                    that matches one the code in the given huffman table.
 * @data: A pointer to the entropy coded data.
 * @table: The huffman table struct to use.
 *
 * Return: The next symbol found in the data or -1 if
 *         no matching symbol was found.
 */
int read_next_symbol(t_Entropy_Coded_data *data, t_Huffman_table huff_table)
{
	unsigned int code, i, j, bit;

	code = 0;
	for(i = 0; i < 16; i++)
	{
		bit = read_next_bit(data);
		if (bit == -1)
			return (-1);

		/**
		 * `concatenate` the bit each time.
		 */
		code = (code << 1) | bit;

		/**
		 * Let's check if any code of length i matches
		 * the current sequence of bits read.
		 */
		for (j = huff_table.huffsizes[i]; j < huff_table.huffsizes[i + 1]; j++)
		{
			if (code == huff_table.huffcodes[j])
			{
				printf("symfbol found >>> %d\n", huff_table.huffvals[j]);
				return (huff_table.huffvals[j]);
			}
		}
	}

	printf("Code not found \n");
	return (-1);
}


/**
 * read_next_bit - Reads the next bit of data
 *                 in the entropy coded data.
 * @data: A pointer to the Entropy coded data
 *        (t_Entropy_Code struct).
 * Return: A char containing the bit read or -1
 *         if an error happened.
 */
int read_next_bit(t_Entropy_Coded_data *data)
{
	unsigned int bit;

	if (data->byte_cursor >= data->current_size)
		return (-1);

	/**
	 * The entropy coded Data unit are stored as bytes (8 bits),
	 * the following allows to read one bit at a time of a single
	 * byte from MSB to LSB as the bit cursor increases from 0 to 7.
	 *
	 * bit cursor = 0 -> MSB
	 * bit cursor = 7 -> LSB
	 */
	bit = (data->mcus[data->byte_cursor] >> (7 - data->bit_cursor)) & 1;

	/**
	 * Update the cursors to keep track of the
	 * next byte and bit to read.
	 */
	if (data->bit_cursor >= 7)
	{
		data->bit_cursor = 0;
		data->byte_cursor++;
	}
	else
		data->bit_cursor++;

	return (bit);
}


/**
 * read_next_bits - Reads the next `length` bits of data
 *                 in the entropy coded data.
 * @data: A pointer to the Entropy coded data
 *        (t_Entropy_Code struct).
 * @length: The number of bits to read.
 *
 * Return: A char containing the bit read or -1
 *         if an error happened.
 */
int read_next_bits(t_Entropy_Coded_data *data, int length)
{
	unsigned int value, bit, i;

	value = 0;
	for (i = 0; i < length; i++)
	{
		bit = read_next_bit(data);
		if (bit == -1)
			return (-1);
		value = (value << 1) | bit;
	}

	return (value);
}
