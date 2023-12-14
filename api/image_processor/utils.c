#include "jpeg.h"


void printSOF(t_jpeg *jpeg)
{
	int i;

	printf("START OF FRAME ----> : \n");
	printf("\tPrecision = %d\n", jpeg->precision);
	printf("\tlines = %d\n", jpeg->lines_count);
	printf("\tSample per lines = %d\n", jpeg->samples_per_line);
	printf("\tmode = %d\n", jpeg->mode);
	printf("\tComponents count = %d\n", jpeg->components_count);
	printf("\tComponents: \n");
	for (i = 0; i < jpeg->components_count; i++)
	{
		printf("\t\tid = %d, h = %d, v = %d, qt seletor = %d\n",
				jpeg->components[i].id, jpeg->components[i].h_sampling_factor,
				jpeg->components[i].v_sampling_factor, jpeg->components[i].qt_id);
	}
	puts("");
	printf("/START OF FRAME\n\n");
}


void printDQT(t_jpeg *jpeg)
{
	int i;

	printf("DQT ----> : \n");
	for (i = 0; i < 4; i++)
	{
		if (jpeg->qt_tables[i].available == 1)
		{
			printf("\t TABLE ID = %d\n", i);
		}
	}
	printf("/DQT\n\n");
}

void printDHT(t_jpeg *jpeg)
{
	int i, j, k;

	printf("DHT ----> : \n");
	printf("\n====== AC tables ====== \n");
        for (i = 0; i < 4; i++)
        {
                if (jpeg->huff_ac_tables[i].available == 1)
                {
                        printf("TABLE ID = %d\nSymbols:\n", i);
			for (j = 0; j < 16; j++)
			{
				printf("%d: ", j + 1);
				for (k = jpeg->huff_ac_tables[i].huffsizes[j]; k < jpeg->huff_ac_tables[i].huffsizes[j + 1]; k++)
				{
					printf("%d ", jpeg->huff_ac_tables[i].huffvals[k]);
				}
				puts("");
			}
			puts("");
                }
        }

	printf("\n====== DC tables ====== \n");
        for (i = 0; i < 4; i++)
	{
                if (jpeg->huff_dc_tables[i].available == 1)
                {
                        printf("TABLE ID = %d\nSymbols:\n", i);
			for (j = 0; j < 16; j++)
                        for (j = 0; j < 16; j++)
                        {
                                printf("%d: ", j + 1);
                                for (k = jpeg->huff_dc_tables[i].huffsizes[j]; k < jpeg->huff_dc_tables[i].huffsizes[j + 1]; k++)
                                {
                                        printf("%d ", jpeg->huff_dc_tables[i].huffvals[k]);
                                }
				puts("");
                        }
                        puts("");
                }
        }
        printf("/DHT\n\n");
}


void printAPP0(t_jpeg *jpeg)
{
	printf("APP0 ----> : \n");
        printf("/APP0\n\n");
}


void printSOS(t_jpeg *jpeg)
{
	int i;

	printf("SOS ----> : \n");
        printf("\tstart_of_spectral_selection = %d\n", jpeg->start_of_spectral_selection);
	printf("\tend_of_spectral_selection = %d\n", jpeg->end_of_spectral_selection);
	printf("\tsa_high = %d\n", jpeg->sa_high);
	printf("\tsa_low = %d\n\n", jpeg->sa_low);

        printf("\tComponents: \n");
	for (i = 0; i < jpeg->components_count; i++)
        {
                printf("\t\tact_selector = %d, dct_selector = %d\n",
                                jpeg->components[i].act_selector,
                                jpeg->components[i].dct_selector);
        }
        puts("");
        printf("/SOS\n\n");
}


