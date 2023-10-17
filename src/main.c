/*
 * Title:			Hello World - C example
 * Author:			Dean Belfield
 * Created:			22/06/2022
 * Last Updated:	22/11/2022
 *
 * Modinfo:
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <agon/vdp_vdu.h>
#include <agon/vdp_key.h>

// Parameters:
// - argc: Argument count
// - argv: Pointer to the argument string - zero terminated, parameters separated by spaces
//

typedef struct {
	
	uint16_t bmp_width;
	uint16_t bmp_height;
	uint8_t  bmp_bitdepth;
	
	uint32_t pixels_offset;
	uint32_t bmp_size;
	uint32_t main_header_size;
	
	uint32_t compression;
	
	uint32_t redBitField;
	uint32_t greenBitField;
	uint32_t blueBitField;
	uint32_t alphaBitField;
	
	uint32_t color_table_size;
	char color_table[1024];
	
	int8_t red_pos;
	int8_t green_pos;
	int8_t blue_pos;
	int8_t alpha_pos;
	
	uint16_t row_padding;
	uint16_t non_pad_row;
	
} bmp_info;

void write16bit(uint16_t w)
{
	putch(w & 0xFF); // write LSB
	putch(w >> 8);	 // write MSB	
}

void rgba8888_to_rgba2222(char *input, char *output, size_t num_pixels) {
    char *input_ptr = input;
    char *output_ptr = output;

    while (num_pixels--) {
        uint8_t r = *input_ptr++;
		uint8_t g = *input_ptr++;
		uint8_t b = *input_ptr++;
        uint8_t a = *input_ptr++;

        *output_ptr++ = (r & 0xC0) | ((g & 0xC0) >> 2) | ((b & 0xC0) >> 4) | (a >> 6);
    }
}

void clear_buffer(uint16_t buffer_id) {
	
	putch(23);
	putch(0);
	putch(0xA0);
	write16bit(buffer_id);
	putch(2);	
	
}

void select_buffer (uint24_t buffer_id) {
	
	putch(23);
	putch(27);
	putch(0x20);
	write16bit(buffer_id);
	
}

void add_stream_to_buffer(uint16_t buffer_id, char* buffer_content, uint16_t buffer_size) {	

	putch(23);
	putch(0);
	putch(0xA0);
	write16bit(buffer_id);
	putch(0);
	write16bit(buffer_size);
	
	mos_puts(buffer_content, buffer_size, 0);

}

void vdp_extended_select(uint16_t buffer_id) {	

	putch(23);
	putch(27);
	putch(0x20);
	write16bit(buffer_id);

}

void vdp_draw(uint16_t x, uint16_t y) {	

	putch(23);
	putch(27);
	putch(3);
	write16bit(x);
	write16bit(y);

}

void assign_buffer_to_bitmap(uint16_t buffer_id, uint8_t bitmap_format, uint16_t width, uint16_t height) {

	vdp_extended_select(buffer_id);
	
	//Consolidate buffer: (if needed) VDU 23, 0, &A0, bufferId; &0C
	
	putch(23);
	putch(0);
	putch(0xA0);
	write16bit(buffer_id);
	putch(0x0E);
	
	//Create bitmap from buffer: VDU 23, 27, &21, bufferId; format, width; height;
	
	putch(23);
	putch(27);
	putch(0x21);
	//write16bit(buffer_id);
	write16bit(width);
	write16bit(height);
	putch(bitmap_format);
	
}

void bgr888_to_rgba2222(char *input, char *output, size_t num_pixels) {
    char *input_ptr = input;
    char *output_ptr = output;

    while (num_pixels--) {
        uint8_t b = *input_ptr++;
        uint8_t g = *input_ptr++;
        uint8_t r = *input_ptr++;
        uint8_t a = 0xFF; // Alpha channel is always set to max value

		// *output_ptr = 0;
		// *output_ptr |= CONVR64[r >> 6];
		// *output_ptr |= CONVG64[g >> 6];
		// *output_ptr |= CONVB64[b >> 6];
		// *output_ptr |= CONVA64[a >> 6];
		// output_ptr++;
		
		*output_ptr = ((r >> 6) & 0x03) | 
              (((g >> 6) & 0x03) << 2) | 
              (((b >> 6) & 0x03) << 4) | 
              (((a >> 6) & 0x03) << 6);	
		output_ptr++;
		
    }
}

void bgra8888_to_rgba2222(char *input, char *output, size_t num_pixels) {
    char *input_ptr = input;
    char *output_ptr = output;

    while (num_pixels--) {
        uint8_t b = *input_ptr++;
        uint8_t g = *input_ptr++;
        uint8_t r = *input_ptr++;
        uint8_t a = *input_ptr++;;

		*output_ptr = ((r >> 6) & 0x03) | 
              (((g >> 6) & 0x03) << 2) | 
              (((b >> 6) & 0x03) << 4) | 
              (((a >> 6) & 0x03) << 6);	
		output_ptr++;
		
    }
}

void generic8888_to_rgba2222(char *input, char *output, size_t num_pixels, uint8_t width, int8_t red_byte, int8_t green_byte, int8_t blue_byte, int8_t alpha_byte) {
    char *input_ptr = input;
    char *output_ptr = output;

    while (num_pixels--) {
        uint8_t r = input_ptr[red_byte];
        uint8_t g = input_ptr[green_byte];
        uint8_t b = input_ptr[blue_byte];
        uint8_t a = (alpha_byte < 0) ? 255 : input_ptr[alpha_byte];

		*output_ptr = ((r >> 6) & 0x03) | 
              (((g >> 6) & 0x03) << 2) | 
              (((b >> 6) & 0x03) << 4) | 
              (((a >> 6) & 0x03) << 6);	
		output_ptr++;
		
		input_ptr += width;
		
    }
}

int8_t getByte(uint32_t bitmask) {

    if (bitmask & 0xFF) {
        return 0;
    }
    else if ((bitmask >> 8) & 0xFF) {
        return 1;
    }
	else if ((bitmask >> 16) & 0xFF) {
        return 2;
    }
    else if ((bitmask >> 24) & 0xFF) {
        return 3;
    }

    return -1;
}

void print_bin(void* value, size_t size) {
    
	int i, j;
	unsigned char* bytes = (unsigned char*)value;
	
	if (size == 0) {
        printf("Error: Invalid size\n");
        return;
    }

    for (i = size - 1; i >= 0; i--) {
        for (j = 7; j >= 0; j--) {
            printf("%d", (bytes[i] >> j) & 1);
        }
    }
}

bmp_info get_info(const char * filename) {

	uint8_t file;
	//FIL * fo;
	bmp_info bmp;
	char initial_header[18];
	char *main_header;
	
	memset(&bmp, 0, sizeof(bmp));	
	
	file = mos_fopen(filename, 0x01);
	//fo = (FIL * ) mos_getfil(file);
	
    if (!file) {
        printf("Error: could not open %s.\r\n", filename);
        return bmp;
    }

	mos_fread(file, initial_header, 14 + 4); //14 Bytes for core header, 4 bytes for full header size
	
	bmp.pixels_offset = * (uint32_t * ) & initial_header[10];
    bmp.main_header_size = * (uint32_t * ) & initial_header[14];
	
	main_header = malloc(bmp.main_header_size);
	
	mos_flseek(file, 14);
	mos_fread(file, main_header, bmp.main_header_size);
	
	bmp.bmp_width = *(int32_t *) & main_header[4];
	bmp.bmp_height = *(int32_t *) & main_header[8];
	bmp.bmp_bitdepth = *(uint16_t *) & main_header[14];	
	bmp.compression = *(uint32_t *) & main_header[16];
	bmp.color_table_size = * (uint32_t * ) & main_header[32];

    if (bmp.color_table_size == 0 && bmp.bmp_bitdepth == 8) {
        bmp.color_table_size = 256;
    }

	if (bmp.color_table_size > 0) mos_fread(file, bmp.color_table, bmp.color_table_size * 4);
	
	bmp.row_padding = (4 - (bmp.bmp_width * (bmp.bmp_bitdepth / 8)) % 4) % 4;
	bmp.non_pad_row = bmp.bmp_width * bmp.bmp_bitdepth / 8;
	
	printf("Debug: BMP is %u x %u x %u, compression type %lu, and DIB size %lu\r\n", bmp.bmp_width, bmp.bmp_height, bmp.bmp_bitdepth, bmp.compression, bmp.main_header_size);

	if (((bmp.compression == 3) || (bmp.compression == 6)) && bmp.main_header_size >= 108) {
		
		if (bmp.bmp_bitdepth == 32) {
							
			bmp.redBitField = *(uint32_t *) & main_header[40];
			bmp.red_pos = getByte(bmp.redBitField);
			
			bmp.greenBitField = *(uint32_t *) & main_header[44];
			bmp.green_pos = getByte(bmp.greenBitField);
			
			bmp.blueBitField = *(uint32_t *) & main_header[48];
			bmp.blue_pos = getByte(bmp.blueBitField);
			
			bmp.alphaBitField = *(uint32_t *) & main_header[52];			
			bmp.alpha_pos = getByte(bmp.alphaBitField);
			
			printf("Red bitfield:   "); print_bin(&bmp.redBitField, sizeof(bmp.redBitField));	printf(" (byte %u in pixel)\r\n", bmp.red_pos);
			printf("Green bitfield: "); print_bin(&bmp.greenBitField, sizeof(bmp.greenBitField));	printf(" (byte %u in pixel)\r\n", bmp.green_pos);
			printf("Blue bitfield:  "); print_bin(&bmp.blueBitField, sizeof(bmp.blueBitField));	printf(" (byte %u in pixel)\r\n", bmp.blue_pos);

			if (bmp.alpha_pos == -1) printf("No alpha channel\r\n");
			else { printf("Alpha bitfield: "); print_bin(&bmp.alphaBitField, sizeof(bmp.alphaBitField)); printf(" (byte %u in pixel)\r\n", bmp.alpha_pos); }
					
		} else if (bmp.bmp_bitdepth == 16) {
			
			uint16_t redmask, greenmask, bluemask, alphamask;
			
			bmp.redBitField = *(uint32_t *) & main_header[40];
			redmask = (uint16_t)(bmp.redBitField & 0xFFFF);
			
			bmp.greenBitField = *(uint32_t *) & main_header[44];
			greenmask = (uint16_t)(bmp.greenBitField & 0xFFFF);
			
			bmp.blueBitField = *(uint32_t *) & main_header[48];
			bluemask = (uint16_t)(bmp.blueBitField & 0xFFFF);
			
			bmp.alphaBitField = *(uint32_t *) & main_header[52];			
			alphamask = (uint16_t)(bmp.alphaBitField & 0xFFFF);
			
			printf("Red bitfield:   "); print_bin(&redmask, sizeof(redmask));
			printf("\r\nGreen bitfield: "); print_bin(&greenmask, sizeof(greenmask));
			printf("\r\nBlue bitfield:  "); print_bin(&bluemask, sizeof(bluemask));

			if (bmp.alphaBitField == 0) printf("\r\nNo alpha channel\r\n");
			else { printf("\r\nAlpha bitfield: "); print_bin(&alphamask, sizeof(alphamask)); printf("\r\n");}		
			
		}
		
		
	}
	
	mos_fclose(file);
	return bmp;

}

bmp_info load_bmp_clean(const char * filename, uint8_t slot) {
	
	uint8_t file;
	FIL * fo;
	bmp_info bmp;
	char initial_header[18];
	char *main_header;
	char * row_rgba2222;
	int16_t y = 0;
	
	memset(&bmp, 0, sizeof(bmp));	
	
	file = mos_fopen(filename, 0x01);
	fo = (FIL * ) mos_getfil(file);
	
    if (!file) {
        printf("Error: could not open %s.\r\n", filename);
        return bmp;
    }

	mos_fread(file, initial_header, 14 + 4); //14 Bytes for core header, 4 bytes for full header size
	
	bmp.pixels_offset = * (uint32_t * ) & initial_header[10];
    bmp.main_header_size = * (uint32_t * ) & initial_header[14];
	
	main_header = malloc(bmp.main_header_size);
	
	mos_flseek(file, 14);
	mos_fread(file, main_header, bmp.main_header_size);
	
	bmp.bmp_width = *(int32_t *) & main_header[4];
	bmp.bmp_height = *(int32_t *) & main_header[8];
	bmp.bmp_bitdepth = *(uint16_t *) & main_header[14];	
	bmp.compression = *(uint32_t *) & main_header[16];
	bmp.color_table_size = * (uint32_t * ) & main_header[32];

    if (bmp.color_table_size == 0 && bmp.bmp_bitdepth == 8) {
        bmp.color_table_size = 256;
    }

	if (bmp.color_table_size > 0) mos_fread(file, bmp.color_table, bmp.color_table_size * 4);
	
	bmp.row_padding = (4 - (bmp.bmp_width * (bmp.bmp_bitdepth / 8)) % 4) % 4;
	bmp.non_pad_row = bmp.bmp_width * bmp.bmp_bitdepth / 8;
	row_rgba2222 = (char * ) malloc(bmp.bmp_width);
	
	if ((bmp.compression != 0) && (bmp.compression != 3)) {
		printf("Non standard BMP compression, exiting.\r\n");
		return bmp;
	}
	
	if (((bmp.compression == 3) || (bmp.compression == 6)) && bmp.main_header_size >= 108) {
		
		if (bmp.bmp_bitdepth == 16) {
			
			printf("16-bit BMP files not supported, use 8-bit (small), 24-bit (fast) or 32-bit (alpha-enabled).\r\n");
			return bmp;

		} else if (bmp.bmp_bitdepth == 32) {
					
			char * src;
			
			bmp.redBitField = *(uint32_t *) & main_header[40];
			bmp.red_pos = getByte(bmp.redBitField);
			bmp.greenBitField = *(uint32_t *) & main_header[44];
			bmp.green_pos = getByte(bmp.greenBitField);
			bmp.blueBitField = *(uint32_t *) & main_header[48];
			bmp.blue_pos = getByte(bmp.blueBitField);
			bmp.alphaBitField = *(uint32_t *) & main_header[52];			
			bmp.alpha_pos = getByte(bmp.alphaBitField);
			
		    src = (char * ) malloc(bmp.bmp_width * bmp.bmp_bitdepth / 8);
			mos_flseek(file, bmp.pixels_offset + ((bmp.bmp_height - 1) * (bmp.non_pad_row + bmp.row_padding)));
			clear_buffer(slot);

			for (y = bmp.bmp_height - 1; y >= 0; y--) {

				mos_fread(file, src, bmp.non_pad_row);
				generic8888_to_rgba2222(src, row_rgba2222,bmp.bmp_width,bmp.bmp_bitdepth / 8,bmp.red_pos,bmp.green_pos,bmp.blue_pos,bmp.alpha_pos);
				add_stream_to_buffer(slot,row_rgba2222,bmp.bmp_width);
				mos_flseek(file, fo -> fptr - ((bmp.non_pad_row * 2) + bmp.row_padding));

			}
			free(src);
			
		}
		
	} else if (bmp.compression == 0) {

		if (bmp.bmp_bitdepth == 16) {
			
			printf("16-bit BMP files not supported, use 8-bit (small), 24-bit (fast) or 32-bit (alpha-enabled).\r\n");
			return bmp;
			
		} else if (bmp.bmp_bitdepth == 8) {
			
			int16_t x,y;
			uint8_t index, b, g, r;

			mos_flseek(file, bmp.pixels_offset + ((bmp.bmp_height - 1) * (bmp.non_pad_row + bmp.row_padding)));
			clear_buffer(slot);
			
			for (y = bmp.bmp_height - 1; y >= 0; y--) {
				for (x = 0; x < bmp.bmp_width; x++) {

					index = (char) mos_fgetc(file);
					b = bmp.color_table[index * 4];
					g = bmp.color_table[index * 4 + 1];
					r = bmp.color_table[index * 4 + 2];
					
					row_rgba2222[x] = 	(((r >> 6) & 0x03) | 
										(((g >> 6) & 0x03) << 2) | 
										(((b >> 6) & 0x03) << 4) | 
										0xC0);
					

				}
				
				add_stream_to_buffer(slot,row_rgba2222,bmp.bmp_width);
				mos_flseek(file, fo -> fptr - ((bmp.non_pad_row * 2) + bmp.row_padding));

			}

		}
		
		if (bmp.bmp_bitdepth == 24) {
		
			//uint16_t new_row_size;
			
		    char * src = (char * ) malloc(bmp.bmp_width * bmp.bmp_bitdepth / 8);
			mos_flseek(file, bmp.pixels_offset + ((bmp.bmp_height - 1) * (bmp.non_pad_row + bmp.row_padding)));
			clear_buffer(slot);
			
			for (y = bmp.bmp_height - 1; y >= 0; y--) {

				mos_fread(file, src, bmp.non_pad_row);
				generic8888_to_rgba2222(src, row_rgba2222,bmp.bmp_width,bmp.bmp_bitdepth / 8,2,1,0,-1);
				add_stream_to_buffer(slot,row_rgba2222,bmp.bmp_width);
				mos_flseek(file, fo -> fptr - ((bmp.non_pad_row * 2) + bmp.row_padding));

			}		
			free(src);
		
		}
	}
	
	assign_buffer_to_bitmap(slot,1,bmp.bmp_width,bmp.bmp_height);
	free(row_rgba2222);
	
	mos_fclose(file);
	return bmp;
	
}

uint16_t strtou16(const char *str) {
    uint16_t result = 0;
    const uint16_t maxDiv10 = 6553;  // 65535 / 10
    const uint16_t maxMod10 = 5;     // 65535 % 10

    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        uint16_t digit = *str - '0';
        if (result > maxDiv10 || (result == maxDiv10 && digit > maxMod10)) {
            return 65535;
        }
        result = result * 10 + digit;
        str++;
    }

    return result;
}

uint8_t strtou8(const char *str) {
    uint8_t result = 0;
    const uint8_t maxDiv10 = 255 / 10;
    const uint8_t maxMod10 = 255 % 10;

    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }

    while (*str >= '0' && *str <= '9') {
        uint8_t digit = *str - '0';
        if (result > maxDiv10 || (result == maxDiv10 && digit > maxMod10)) {
            return 255;
        }
        result = result * 10 + digit;
        str++;
    }

    return result;
}

uint24_t strtou24(const char *str) {
    uint32_t result = 0;
    const uint32_t maxDiv10 = 1677721;
    const uint32_t maxMod10 = 5;

    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
	
    while (*str >= '0' && *str <= '9') {
        uint32_t digit = *str - '0';
        if (result > maxDiv10 || (result == maxDiv10 && digit > maxMod10)) {
            return 16777215;
        }
        result = result * 10 + digit;
        str++;
    }

    return result;
}

static volatile SYSVAR *sv;

int main(int argc, char * argv[])
//int main(void)
{
	
    uint24_t x, y;
	uint8_t bitmap_slot = 0;
	bmp_info bmp;
	
	sv = vdp_vdu_init();
	if ( vdp_key_init() == -1 ) return 1;

	//Args = 0:binary name, 1:filname, 2:slot, 3:topleft, 3:topright
	
	if ((argc < 2) || (argc == 4) || (argc > 5)) {
        printf("Usage is %s <filename> [bitmap slot] [top-left x or C] [top-left y or C]\r\n", argv[0]);
        return 0;
		//bmp = load_bmp_clean(argv[1], 0);
    }
	
	if (argc > 2) bitmap_slot = strtou8(argv[2]);
	
    //vdp_mode(8);
	
	if (argc == 2) {
		
		bmp = load_bmp_clean(argv[1], 0);
		
	} else if (argc == 3) {
		
		if (strcmp(argv[2], "/i") == 0) get_info(argv[1]);
		else bmp = load_bmp_clean(argv[1], bitmap_slot);
		
	} else if (argc == 5) {
	
		bmp = load_bmp_clean(argv[1], bitmap_slot);
		
		if (argv[3][0] == 'C' || argv[3][0] == 'c') x = (sv->scrWidth - bmp.bmp_width) / 2;
		else x = strtou16(argv[4]);
		
		if (argv[4][0] == 'C' || argv[4][0] == 'c') y = (sv->scrHeight - bmp.bmp_height) / 2;
		else y = strtou16(argv[4]);
		
		vdp_extended_select(bitmap_slot);
		
		vdp_draw(x,y);
		
	}

	return 0;
}