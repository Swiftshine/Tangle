#include "tangle.h"

// largely left untouched from Phillip Gage's code
// aside from minor formatting and stack size adjustments for larger files

// i wish it wasn't ugly but if it ain't broke, don't fix it

void tangle::bpe_decode(FILE *input, FILE *output) {
  unsigned char left[256], right[256], stack[50000];
  short int c, count, i, size;

  /* Unpack each block until end of file */
	while ((count = getc(input)) != EOF) {	
	  /* Set left to itself as literal flag */
		for (i = 0; i < 256; i++)
		  left[i] = i;	
		/* Read pair table */
		for (c = 0;;) {	
		/* Skip range of literal bytes */
		if (count > 127) {
		  c += count - 127;
		  count = 0;
		}
		if (c == 256) break;	
		/* Read pairs, skip right if literal */
		for (i = 0; i <= count; i++, c++) {
		  left[c] = getc(input);
		  if (c != left[c])
		    right[c] = getc(input);
		}
		if (c == 256) break;
		count = getc(input);
	}

  /* Calculate packed data block size */
  	size = 256 * getc(input) + getc(input);

  /* Unpack data block */
		for (i = 0;;) {

    	  /* Pop byte from stack or read byte */
    	  	if (i)
    	  	  	c = stack[--i];
    	  	else {
    	  	  	if (!size--) break;
    	  	  	c = getc(input);
    	  	}

      		/* Output byte or push pair on stack */
      		if (c == left[c])
      		  putc(c,output);
      		else {
        		stack[i++] = right[c];
        		stack[i++] = left[c];
      		}
    	}
  	}
}