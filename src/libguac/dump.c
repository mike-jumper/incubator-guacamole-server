
#include <stdio.h>

/**
 * Convert a section of data to a row of hex characters
 */
static void __get_row(char* row, unsigned char* data, int length) {

    /* For each byte in data */
    while (length >= 1) {

        /* Get next byte */
        unsigned char value = *(data++);

        /* Add to output */
        snprintf(row, 4, "%02X ", value);

        /* Advance to next section of output */
        row += 3;
        length--;

    }

    /* Terminate string */
    *row = 0;

}

static void __get_contents(char* contents, unsigned char* data, int length) {

    /* For each byte in input */
    while (length > 0) {

        /* If byte printable, add to contents */
        char value = *(data++);
        if (value >= 0x20 && value <= 0x7E)
            *(contents++) = value;

        /* Otherwise, replace with period */
        else
            *(contents++) = '.';

        length--;

    }

    /* Terminate string */
    *contents = 0;

}

void dump(const char* file, const void* buffer, int length) {

    FILE* output = fopen(file, "a");
    fprintf(output, "--- %i bytes:\n", length);

    /* Convert pointer type */
    unsigned char* data = (unsigned char*) buffer;

    int offset = 0;
    char row[80];
    char contents[17];

    /* For each row of data */
    while (length > 0) {

        int i;

        /* Output no more than 16 bytes */
        int row_size = length;
        if (row_size > 16)
            row_size = 16;

        /* Print row */
        __get_row(row, data, row_size);
        fprintf(output, "%08X:  %s", offset, row);

        /* Pad to same end column */
        for (i=row_size; i<16; i++)
            fprintf(output, "   ");

        /* Print contents */
        __get_contents(contents, data, row_size);
        fprintf(output, " %s\n", contents);

        /* Advance to next row */
        data   += row_size;
        offset += row_size;
        length -= row_size;

    }

    fclose(output);

}

