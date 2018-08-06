#include "half_float.h"

#define HALF_FLOAT_SIGN_MASK_32 0x80000000
#define HALF_FLOAT_SIGN_MASK_16 0x00008000
#define HALF_FLOAT_SIGN_SHIFT 16

#define HALF_FLOAT_EXPONENT_MASK_32 0x7f800000
#define HALF_FLOAT_EXPONENT_MASK_16	0x00007c00

#define HALF_FLOAT_MANTISSA_MASK_32 0x007fffff
#define HALF_FLOAT_MANTISSA_MASK_16 0x000003ff
#define HALF_FLOAT_MANTISSA_SHIFT 13


static int single_to_half_table_generated = 0;
static short base_table[512];
static char shift_table[512];



static int half_to_single_table_generated = 0;
static int mantissa_table[2028];
static int offset_table[64];
static int exponent_table[64];


half_t convert_to_half(float value)
{
    short ret = 0;
	int b_value;

	unsigned int i;
	int e;

    if(!single_to_half_table_generated)
	{
		for(i = 0; i < 256; i++)
		{
            e = i - 127;

			if(e < -24)
			{
				/* very small numbers map to zero... */
                base_table[i | 0x000] = 0x0000;
				base_table[i | 0x100] = 0x8000;

                shift_table[i | 0x000] = 24;
                shift_table[i | 0x100] = 24;
			}
			else if(e < -14)
			{
				/* small numbers map to denorms... */
				base_table[i | 0x000] = (0x0400 >> (-e - 14));
				base_table[i | 0x100] = (0x0400 >> (-e - 14)) | 0x8000;

				shift_table[i | 0x000] = -e - 1;
				shift_table[i | 0x100] = -e - 1;
			}
			else if(e <= 15)
			{
				/* normal numbers only lose some precision... */
                base_table[i | 0x000] = ((e + 15) << 10);
				base_table[i | 0x100] = ((e + 15) << 10) | 0x8000;

				shift_table[i | 0x000] = 13;
				shift_table[i | 0x100] = 13;
			}
			else
			{
				/* infinity remain infinity, NaN remain NaN... */
				base_table[i | 0x000] = 0x7c00;
				base_table[i | 0x100] = 0xfc00;

				if(e < 128)
				{
					shift_table[i | 0x000] = 24;
					shift_table[i | 0x100] = 24;
				}
				else
				{
					shift_table[i | 0x000] = 13;
					shift_table[i | 0x100] = 13;
				}
			}
		}

		single_to_half_table_generated = 1;
	}

	b_value = *(int *)&value;

	ret = base_table[(b_value >> 23) & 0x1ff] + ((b_value& 0x007fffff) >> shift_table[(b_value >> 23) & 0x1ff]);
	ret |= (b_value & 0x80000000) >> 16;

    return (half_t)ret;
}




float convert_to_single(half_t value)
{
	float ret;
	int b_value = 0;
	int int_ret = 0;


    if(!half_to_single_table_generated)
	{
		mantissa_table[0] = 0;


		half_to_single_table_generated = 1;
	}




	/* copy the bit pattern as-is... */
    b_value = *(unsigned short *)&value;

	//int_ret =


	return ret;
}





