#include "tangle.h"
#define windowSize 60
#define bufferSize 40
#define arraySize bufferSize + windowSize


int findMatch(unsigned char window[], unsigned char str[], int strLen) {
    int j, k, pos = -1;

    for (int i = 0; i <= 60 - strLen; i++) {
        pos = k = i;

        for (j = 0; j < strLen; j++) {
            if (str[j] == window[k])
                k++;
            else
                break;
        }
        if (j == strLen)
            return pos;
    }

    return -1;
}

void LZ77::decompress(FILE* input, FILE* output) {
    bool last = false;
    int inputLength = 0;
    int outputLength = 0;
    int endOffset = 0;
    int pos = -1;
    int i, size, shift, c_in;
    size_t bytesRead = (size_t)-1;
    unsigned char c;
    unsigned char array[arraySize];
    unsigned char window[windowSize];
    unsigned char buffer[bufferSize];
    unsigned char loadBuffer[bufferSize];
    unsigned char str[bufferSize];


    // Get input length
    fseek(input, 0, SEEK_END);
    inputLength = ftell(input);
    fseek(input, 0, SEEK_SET);

    // If file is empty, return alert
    if (inputLength == 0) {
        std::cout << "Warning - LZ77::decompress() - the input file is empty!" << std::endl;
        return;
    }

    // If file length is smaller than arraySize, not worth processing
    if (inputLength < arraySize) {
        std::cout << "Warning - LZ77::decompress() - the input file is too small to decompress" << std::endl;
        return;
    }

    // Load array with initial bytes
    fread(array, 1, arraySize, input);

    // Write the first bytes to output file
    fwrite(array, 1, windowSize, output);

    // LZ77 logic beginning
    while (true) {
        if ((c_in = fgetc(input)) == EOF)
            last = true;
        else
            c = (unsigned char)c_in;

        // Load window (dictionary)
        for (int k = 0; k < windowSize; k++)
            window[k] = array[k];

        // Load buffer (lookahead)
        for (int k = windowSize, j = 0; k < arraySize; k++, j++) {
            buffer[j] = array[k];
            str[j] = array[k];
        }

        // Search for longest match in window
        if (endOffset != 0) {
            size = bufferSize - endOffset;
            if (endOffset == bufferSize)
                break;
        }
        else {
            size = bufferSize;
        }

        pos = -1;
        for (i = size; i > 0; i--) {
            pos = findMatch(window, str, i);
            if (pos != -1)
                break;
        }

        // No match found
        // Write only one byte instead of two
        // 255 -> offset = 0, match = 0
        if (pos == -1) {
            fputc(255, output);
            fputc(buffer[0], output);
            shift = 1;
        }
        // Found match
        // offset = windowSize - position of match
        // i = number of match bytes
        // endOffset = number of bytes in lookahead buffer not to be considered (EOF)
        else {
            fputc(windowSize - pos, output);
            fputc(i, output);
            if (i == bufferSize) {
                shift = bufferSize + 1;
                if (!last)
                    fputc(c, output);
                else
                    endOffset = 1;
            }
            else {
                if (i + endOffset != bufferSize)
                    fputc(buffer[i], output);
                else
                    break;
                shift = i + 1;
            }
        }

        // Shift buffers
        for (int j = 0; j < arraySize - shift; j++)
            array[j] = array[j + shift];
        if (!last)
            array[arraySize - shift] = c;

        if (shift == 1 && last)
            endOffset++;

        // If (shift != 1) -> read more bytes from file
        if (shift != 1) {
            // Load loadBuffer with new bytes
            bytesRead = fread(loadBuffer, 1, (size_t)shift - 1, input);

            // Load array with new bytes
            // Shift bytes in array, then splitted into window[] and buffer[] during next iteration
            for (int k = 0, l = arraySize - shift + 1; k < shift - 1; k++, l++)
                array[l] = loadBuffer[k];

            if (last) {
                endOffset += shift;
                continue;
            }

            if (bytesRead < shift - 1)
                endOffset = shift - 1 - bytesRead;
        }
    }

    // Get output length
    fseek(output, 0, SEEK_END);
    outputLength = ftell(output);
    fseek(output, 0, SEEK_SET);
}

void LZ77::compress(FILE* input, FILE* output, string inputName) {
    bool last = false;
    int inputLength = 0;
    int outputLength = 0;
    int endOffset = 0;
    int pos = -1;
    int i, size, shift, c_in;
    size_t bytesRead = (size_t)-1;
    unsigned char c;
    unsigned char array[arraySize];
    unsigned char window[windowSize];
    unsigned char buffer[bufferSize];
    unsigned char loadBuffer[bufferSize];
    unsigned char str[bufferSize];

    // Get input length
    fseek(input, 0, SEEK_END);
    inputLength = ftell(input);
    fseek(input, 0, SEEK_SET);

    // If file is empty, return alert
    if (inputLength == 0) {
        std::cout << "Warning -  - LZ77::compress() - the file is empty!" << std::endl;
        return;
    }

    // If file length is smaller than arraySize, not worth processing
    if (inputLength < arraySize) {
        std::cout << "Warning - LZ77::compress() - the file " << inputName << " is too small to compress" << std::endl;
        return;
    }

    // Load array with initial bytes
    fread(array, 1, arraySize, input);

    // Write the first bytes to output file
    fwrite(array, 1, windowSize, output);

    // LZ77 logic beginning
    while (true) {
        if ((c_in = fgetc(input)) == EOF)
            last = true;
        else
            c = (unsigned char)c_in;

        // Load window (dictionary)
        for (int k = 0; k < windowSize; k++)
            window[k] = array[k];

        // Load buffer (lookahead)
        for (int k = windowSize, j = 0; k < arraySize; k++, j++) {
            buffer[j] = array[k];
            str[j] = array[k];
        }

        // Search for longest match in window
        if (endOffset != 0) {
            size = bufferSize - endOffset;
            if (endOffset == bufferSize)
                break;
        }
        else {
            size = bufferSize;
        }

        pos = -1;
        for (i = size; i > 0; i--) {
            pos = findMatch(window, str, i);
            if (pos != -1)
                break;
        }

        // No match found
        // Write only one byte instead of two
        // 255 -> offset = 0, match = 0
        if (pos == -1) {
            fputc(255, output);
            fputc(buffer[0], output);
            shift = 1;
        }
        // Found match
        // offset = windowSize - position of match
        // i = number of match bytes
        // endOffset = number of bytes in lookahead buffer not to be considered (EOF)
        else {
            fputc(windowSize - pos, output);
            fputc(i, output);
            if (i == bufferSize) {
                shift = bufferSize + 1;
                if (!last)
                    fputc(c, output);
                else
                    endOffset = 1;
            }
            else {
                if (i + endOffset != bufferSize)
                    fputc(buffer[i], output);
                else
                    break;
                shift = i + 1;
            }
        }

        // Shift buffers
        for (int j = 0; j < arraySize - shift; j++)
            array[j] = array[j + shift];
        if (!last)
            array[arraySize - shift] = c;

        if (shift == 1 && last)
            endOffset++;

        // If (shift != 1) -> read more bytes from file
        if (shift != 1) {
            // Load loadBuffer with new bytes
            bytesRead = fread(loadBuffer, 1, (size_t)shift - 1, input);

            // Load array with new bytes
            // Shift bytes in array, then splitted into window[] and buffer[] during next iteration
            for (int k = 0, l = arraySize - shift + 1; k < shift - 1; k++, l++)
                array[l] = loadBuffer[k];

            if (last) {
                endOffset += shift;
                continue;
            }

            if (bytesRead < shift - 1)
                endOffset = shift - 1 - bytesRead;
        }
    }

    // Get output length
    fseek(output, 0, SEEK_END);
    outputLength = ftell(output);
    fseek(output, 0, SEEK_SET);
}