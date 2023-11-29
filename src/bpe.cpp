#include "tangle.h"
#pragma warning(disable : 4996) // just so it compiles

// adapted from Philip Gage's expand.c for Byte Pair Encoding.
// i can't find a source anywhere but here: https://stackoverflow.com/questions/14688460/from-char-to-stream
// http://www.pennelynn.com/Documents/CUJ/HTML/94HTML/19940045.HTM

/* DECOMPRESSION */
void BPE::decompress(FILE* input, FILE* output) {
    unsigned char left[256], right[256], stack[30];
    short int c, count, i, size;

    /* unpack each block until end of file */
    while ((count = getc(input)) != EOF)
    {
        /* set left to itself as literal flag */
        for (i = 0; i < 256; i++)
        {
            left[i] = i;
        }

        /* read pair table */
        for (c = 0; ; )
        {
            /* skip range of literal bytes */
            if (count > 127)
            {
                c += count - 127;
                count = 0;
            }
            if (c == 256)
            {
                break;
            }

            /* read pairs, skip right if literal */
            for (i = 0; i <= count; i++, c++)
            {
                left[c] = getc(input);
                if (c != left[c])
                {
                    right[c] = getc(input);
                }
            }
            if (c == 256)
            {
                break;
            }
            count = getc(input);
        }

        /* calculate packed data block size */
        size = 256 * getc(input) + getc(input);

        /* unpack data block */
        for (i = 0; ; )
        {
            /* pop byte from stack or read byte */
            if (i)
            {
                c = stack[--i];
            }
            else
            {
                if (!size--)
                {
                    break;
                }
                c = getc(input);
            }

            /* output byte or push pair on stack */
            if (c == left[c])
            {
                putc(c, output);
            }
            else
            {
                stack[i++] = right[c];
                stack[i++] = left[c];
            }
        }
    }
}

/* COMPRESSION */

#define BLOCKSIZE 5000   /* Maximum block size */
#define HASHSIZE  4096   /* Size of hash table */
#define MAXCHARS   200   /* Char set per block */
#define THRESHOLD    3   /* Minimum pair count */

unsigned char buffer[BLOCKSIZE]; /* Data block */
unsigned char leftcode[256];     /* Pair table */
unsigned char rightcode[256];    /* Pair table */
unsigned char left[HASHSIZE];    /* Hash table */
unsigned char right[HASHSIZE];   /* Hash table */
unsigned char count[HASHSIZE];   /* Pair count */
int size;        /* Size of current data block */

int lookup(unsigned char, unsigned char);
int fileread(FILE*);
void filewrite(FILE*);
void compress(FILE*, FILE*);

/* Return index of character pair in hash table */
/* Deleted nodes have count of 1 for hashing */
int lookup(unsigned char a, unsigned char b)
{
    int index;

    /* Compute hash key from both characters */
    index = (a ^ (b << 5)) & (HASHSIZE - 1);

    /* Search for pair or first empty slot */
    while ((left[index] != a || right[index] != b) &&
        count[index] != 0)
        index = (index + 1) & (HASHSIZE - 1);

    /* Store pair in table */
    left[index] = a;
    right[index] = b;
    return index;
}

/* Read next block from input file into buffer */
int fileread(FILE* input)
{
    int c, index, used = 0;

    /* Reset hash table and pair table */
    for (c = 0; c < HASHSIZE; c++)
        count[c] = 0;
    for (c = 0; c < 256; c++) {
        leftcode[c] = c;
        rightcode[c] = 0;
    }
    size = 0;

    /* Read data until full or few unused chars */
    while (size < BLOCKSIZE && used < MAXCHARS &&
        (c = getc(input)) != EOF) {
        if (size > 0) {
            index = lookup(buffer[size - 1], c);
            if (count[index] < 255) ++count[index];
        }
        buffer[size++] = c;

        /* Use rightcode to flag data chars found */
        if (!rightcode[c]) {
            rightcode[c] = 1;
            used++;
        }
    }
    return c == EOF;
}

/* Write each pair table and data block to output */
void filewrite(FILE* output)
{
    int i, len, c = 0;

    /* For each character 0..255 */
    while (c < 256) {

        /* If not a pair code, count run of literals */
        if (c == leftcode[c]) {
            len = 1; c++;
            while (len < 127 && c < 256 && c == leftcode[c]) {
                len++; c++;
            }
            putc(len + 127, output); len = 0;
            if (c == 256) break;
        }

        /* Else count run of pair codes */
        else {
            len = 0; c++;
            while (len < 127 && c < 256 && c != leftcode[c] ||
                len < 125 && c < 254 && c + 1 != leftcode[c + 1]) {
                len++; c++;
            }
            putc(len, output);
            c -= len + 1;
        }

        /* Write range of pairs to output */
        for (i = 0; i <= len; i++) {
            putc(leftcode[c], output);
            if (c != leftcode[c])
                putc(rightcode[c], output);
            c++;
        }
    }

    /* Write size bytes and compressed data block */
    putc(size / 256, output);
    putc(size % 256, output);
    fwrite(buffer, size, 1, output);
}

/* Compress from input file to output file */
void BPE::compress(FILE* infile, FILE* outfile)
{
    int leftch, rightch, code, oldsize;
    int index, r, w, best, done = 0;

    /* Compress each data block until end of file */
    while (!done) {

        done = fileread(infile);
        code = 256;

        /* Compress this block */
        for (;;) {

            /* Get next unused char for pair code */
            for (code--; code >= 0; code--)
                if (code == leftcode[code] && !rightcode[code])
                    break;

            /* Must quit if no unused chars left */
            if (code < 0) break;

            /* Find most frequent pair of chars */
            for (best = 2, index = 0; index < HASHSIZE; index++)
                if (count[index] > best) {
                    best = count[index];
                    leftch = left[index];
                    rightch = right[index];
                }

            /* Done if no more compression possible */
            if (best < THRESHOLD) break;

            /* Replace pairs in data, adjust pair counts */
            oldsize = size - 1;
            for (w = 0, r = 0; r < oldsize; r++) {
                if (buffer[r] == leftch &&
                    buffer[r + 1] == rightch) {

                    if (r > 0) {
                        index = lookup(buffer[w - 1], leftch);
                        if (count[index] > 1) --count[index];
                        index = lookup(buffer[w - 1], code);
                        if (count[index] < 255) ++count[index];
                    }
                    if (r < oldsize - 1) {
                        index = lookup(rightch, buffer[r + 2]);
                        if (count[index] > 1) --count[index];
                        index = lookup(code, buffer[r + 2]);
                        if (count[index] < 255) ++count[index];
                    }
                    buffer[w++] = code;
                    r++; size--;
                }
                else buffer[w++] = buffer[r];
            }
            buffer[w] = buffer[r];

            /* Add to pair substitution table */
            leftcode[code] = leftch;
            rightcode[code] = rightch;

            /* Delete pair from hash table */
            index = lookup(leftch, rightch);
            count[index] = 1;
        }
        filewrite(outfile);
    }
}