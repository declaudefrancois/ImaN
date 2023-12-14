#ifndef JPEG_H
#define JPEG_H

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include "image.h"

/**
 * enum JPEG_marquer_code - JFIF markers code.
 *
 * @SOI: Start Of Frame marker.
 * @APP0: JFIF Application Marker.
 * @SOF0: Start OF FRAME, Baseline DCT.
 */
enum JPEG_marquer_code
{
	SOI = 0xD8, /** Start of IMAGE */
	APP0 = 0xE0, /** JFIF APP MARKER */


	/** Non differential, Huffman coding */
	SOF0 = 0xC0, /** Start OF FRAME, Baseline DCT */
	SOF1 = 0xC1, /** Start OF FRAME, Extended sequential DCT */
	SOF2 = 0xC2, /** Start OF FRAME, Progressive DCT */
	SOF3 = 0xC3, /** Start OF FRAME, Lossless (sequential) */
	/** Non differential, Huffman coding */

	DHT = 0xC4, /** Define Huffman Tables(s) */
	DRI = 0xDD, /** Define restart intervall */
	/** Differential, Huffman coding */
	SOF5 = 0xC5, /** Start OF FRAME, Differential sequential DCT */
	SOF6 = 0xC6, /** Start OF FRAME, Differential progressive DCT */
	SOF7 = 0xC7, /** Start OF FRAME, Differential lossless (sequential) */
	/** Differential, Huffman coding */

	/** Non differential, arithmetic coding */
	JPG = 0xC8, /** Reserved for JPEG extensions */
	SOF9 = 0xC9, /** Start OF FRAME, Extended sequential DCT */
	SOF10 = 0xCA, /** Start OF FRAME, Progressive DCT */
	SOF11 = 0xCB, /** Start OF FRAME, Lossless (sequential) */
	/** Non differential, arithmetic coding */

	/** Differential, arithmetic coding */
	SOF13 = 0xCB, /** Start OF FRAME, Differential sequential DCT */
	SOF14 = 0xCB, /** Start OF FRAME, Differential progressive DCT */
	SOF15 = 0xCB, /** Start OF FRAME, Differential lossless (sequential) */
	/** Differential, arithmetic coding */

	DAC = 0xCC, /** Define arithmetic coding conditioning(s) */

	RST0 = 0xD0,
	RST1 = 0xD1,
	RST2 = 0xD2,
	RST3 = 0xD3,
	RST4 = 0xD4,
	RST5 = 0xD5,
	RST6 = 0xD6,
	RST7 = 0xD7,

	SOS = 0xDA, /** Start Of Scan */
	DQT = 0xDB, /** Define Quantization Table */
	EOI = 0xD9, /** End Of Image */
};

typedef struct mcu {
	int y[64];
	int cb[64];
	int cr[64];
} t_mcu;

typedef struct _Entropy_Coded_data {
	char *mcus;
	size_t current_size;
	size_t max_size;
	size_t byte_cursor;
	int bit_cursor;
} t_Entropy_Coded_data;


typedef enum _JPEG_mode {
	JPEG_MODE_NOT_SET,
	SEQUENTIAL_BASELINE,
	SEQUENTIAL_EXTENDED,
	PROGRESSIVE,
	LOSSLESS,
} JPEG_mode;


typedef struct Quantization_table {
	int coeffs[64];
	int available;
} t_Quantization_table;


typedef struct Huffman_table {
	int available;
	int huffcodes[162];
	int huffvals[162];
	int huffsizes[17];
} t_Huffman_table;


typedef struct Component_spec {
	int id;
	int h_sampling_factor;
	int v_sampling_factor;
	int act_selector;
	int dct_selector;
	int qt_id;
} t_component_spec;


typedef struct MCU {
	int y[64];
	int cb[64];
	int cr[64];
} t_MCU;


typedef struct jpeg
{
	int precision;
	int lines_count;
	int samples_per_line;
	int components_count;

	int start_of_spectral_selection;
	int end_of_spectral_selection;

	int sa_high;
	int sa_low;

	/** We can have at most 3 components. */
	t_component_spec components[3];

	t_Entropy_Coded_data data;

	JPEG_mode mode;
	int restart_interval;

	/** At most 4 of each tables is allowed by the standard. */
	t_Quantization_table qt_tables[4];
	t_Huffman_table huff_ac_tables[4];
	t_Huffman_table huff_dc_tables[4];
} t_jpeg;

/**
 * isJPEG - Test if the file is a JPEG image.
 *
 * @fd: A file descriptor to the file to test.
 *
 * Return: 1 if the file is a JPEG image,
 *          0 if it is not and -1 if an error
 *          happened.
 */
int isJPEG(int fd);


/**
 * init_jpeg - Allocates memory for a jpeg struct
 *             to be used in the codec.
 *
 * @jpeg: A double pointer to the jpeg struct to
 *          initializes.
 *
 * Return: 1 on success or -1 on failure.
 */
int init_jpeg(t_jpeg **jpeg);

/**
 * free_jpeg - Frees memory used by the jpeg struct.
 * @jpeg: A pointer to the jpeg struct to free.
 */
void free_jpeg(t_jpeg *jpeg);

/**
 * decodeJPEG - Decodes a JPEG file from an open file
 *              descriptor.
 * @fd: A file descriptor to the image file.
 *
 * Return: A pointer a t_image struct representing
 *         the decoded image with bitmap pixel data.
 */
t_image *decodeJPEG(int fd);


/**
 * read_marker - Tries to read 4 bytes marker data
 *               in marker.
 * @fd: Image file descriptor.
 * @marker: A char pointer with a least 4 bytes of size.
 */
void read_marker(int fd, char *marker);


/**
 * handle_marker - Handles a single image marker.
 * @code: The marker code.
 * @fd: The file descriptor to the openned image.
 *
 * Return: The last cursor of the file descriptor
 *         not handled by the handler.
 */
void handle_marker(int code, int fd, t_jpeg *jpeg);

typedef struct frame_header
{
	int precision;
	int lines_count;
	int samples_per_line;
	int components_count;
	int *components_id;
	int *h_sampling_factors;
	int *v_sampling_factors;
	int *qt_selector;
} t_frame_header;


typedef struct scan_header
{
	int components_count;
	int *components_selector;
	int dct_selector;
	int act_selector;
	int ss;
	int se;
	int ah;
	int al;
} t_scan_header;


/**
 * struct marker_handler - Structure for mapping markers
 *                         to a function that handles it.
 * @marker: The marker code
 * @f: The function pointer
 */
typedef struct marker_handler
{
	int marker;
	void (*func)(int, t_jpeg*);
} t_marker_handler;

/**
 * handle_APP0 - handle `APP0` marker-segment.
 *
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_APP0(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_DQT - Handles the Define Quantization Table (DQT)
 *              marker segment.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_DQT(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_DHT - Handles a Define Huffman Table (DHT)
 *              marker segment.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to a jpeg struct.
 */
void handle_DHT(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_SOS - Handles Start Of Scan marker.
 * @fd: A open file descriptor to the file.
 * @jpeg: A pointer to jpeg struct.
 */
void handle_SOS(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_SOFO - Handle SOF0 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF0(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_SOF1 - Handle SOF1 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF1(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_SOF2 - Handle SOF2 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF2(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_SOF3 - Handle SOF3 marker segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_SOF3(int fd, t_jpeg *jpeg_decomp);

/**
 * handle_frame_header - Handle start of frame segment.
 *
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_frame_header(int fd, t_jpeg *jpeg);

/**
 * read_raw_mcus - Reads the encoded mcus and stores them
 *                  in the jpeg struct.
 * @fd: A file descriptor to the file being processed.
 * @jpeg: A pointer to a jpeg struct.
 */
void read_raw_mcus(int fd, t_jpeg *jpeg);

/**
 * decode_mcus - Decodes the raw mcus data in the jpeg struct
 *               and stores the decoded data in the image struct.
 * @jpeg: A pointer to the jpeg struct containing the mcus to decode.
 * @image: A pointer to the image struct.
 *
 * Return: A pointer to the all the decoded MCUs
 *         or NULL if the decoding failed.
 */
t_MCU *decode_mcus(t_jpeg *jpeg, t_image *image);

int push_mcus_data(t_jpeg *jpeg, char data);

int realloc_mcus(t_jpeg *jpeg);

int init_mcus(t_jpeg *jpeg);

/**
 * generate_huffcodes - Generates Huffman code corresponding
 *                      to code length in the table `table`.
 *
 * @table: A pointer to the Huffman Table to generate codes for.
 */
void generate_huffcodes(t_Huffman_table *table);


/**
 * generate_all_huffcodes - Generates all the dc and the ac
 *                          huffman codes based on the corresponding
 *                          code length.
 * @jpeg: A pointer to a jpeg struct.
 */
void generate_all_huffcodes(t_jpeg *jpeg);

/**
 * get_mcu_component - Gets a single component of an MCU.
 * @id: The component's id.
 * @mcu: The mcu struct.
 * @component: A pointer to store the color component in.
 *
 * Return: A pointer to the coefficients array of the component or
 *         NULL if the id is not 0, 1 or 2.
 */
int *get_mcu_component(int id, t_mcu mcu, int *component);


/**
 * read_next_bits - Reads the next `length` bits of data
 *                 in the entropy coded data.
 * @data: A pointer to the Entropy coded data
 *        (t_Entropy_Code struct).
 * @length: The number of bits to read.
 * Return: A char containing the bit read or -1
 *         if an error happened.
 */
int read_next_bits(t_Entropy_Coded_data *data, int length);


/**
 * read_next_bit - Reads the next bit of data
 *                 in the entropy coded data.
 * @data: A pointer to the Entropy coded data
 *        (t_Entropy_Code struct).
 * Return: A char containing the bit read or -1
 *         if an error happened.
 */
int read_next_bit(t_Entropy_Coded_data *data);


/**
 * read_next_symbol - Reads the next symbol in the entropy coded data
 *                    that matches one the code in the given huffman table.
 * @data: A pointer to the entropy coded data.
 * @table: The huffman table struct to use.
 *
 * Return: The next symbol found in the data or -1 if
 *         no matching symbol was found.
 */
int read_next_symbol(t_Entropy_Coded_data *data, t_Huffman_table huff_table);


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
	t_Huffman_table ac_table);


/**
 * handle_DRI - Handle Define Restart Interval marker.
 * @fd: An open file descriptor to the file
 *      being processed.
 * @jpeg: A pointer to a jepg struct.
 */
void handle_DRI(int fd, t_jpeg *jpeg);


/**
 * init_array - Set all items of array to zero.
 * @size: The array size.
 * @array: The array.
 */
void init_array(int size, int *array);

void skip_marker(int fd);
void readBytes(int fd, char *buff, int count);
int getMarkerLength(int fd);
char read_byte(int fd);

void printSOF(t_jpeg *jpeg);
void printDQT(t_jpeg *jpeg);
void printDHT(t_jpeg *jpeg);
void printAPP0(t_jpeg *jpeg);
void printSOS(t_jpeg *jpeg);

#endif /** JPEG_H */
