#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

//decode a 32 but unsigned integer according to the LEB128 encoding
unsigned int readVaruint(FILE *in, FILE *out) {
    unsigned int result = 0;
    unsigned int shift = 0;
    
    for (;;) {
        unsigned char byte = getc(in);
        if (out) {
            fputc(byte, out);
        }
        result |= (byte & 0x7F) << shift;

        if ((byte & 0x80) == 0) {
            break;
        }

        shift += 7;
    }

    return result;
}

int main(int argc, char **argv) {
    //stdin and stdout must be files because we are processing binary data
    if (isatty(fileno(stdin))) {
        fputs("Use the < operator to redirect a .wasm file to stdin\n", stderr);
    }

    if (isatty(fileno(stdout))) {
        fputs("Use the > operator to redirect stdout to a .wasm file\n", stderr);
    }

    if (isatty(fileno(stdin)) || isatty(fileno(stdout))) {
        return -1;
    }

    //remove section 0 by default
    int sectionsToIgnore = 1;

    int argumentType = 0;
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--remove-sections") == 0) {
            argumentType = 1;
            sectionsToIgnore = 0;
            continue;
        }

        if (argumentType == 1) {
            //any non-integer inputs will be treated as section 0 (custom)
            int section = atoi(argv[i]);
            sectionsToIgnore |= 1 << section;
        }
    }

    int header[2];
    const int elementsRead = fread(&header, sizeof(int), 2, stdin);

    if (elementsRead != 2 || header[0] != 0x6d736100 || header[1] != 0x01) {
        fputs("Input file is not valid wasm\n", stderr);
        return -1;
    }

    //write header back out
    fwrite(&header, sizeof(int), 2, stdout);

    for (;;) {
        const char sectionCode = getc(stdin);
        if (sectionCode == EOF) {
            break;
        }

        //skip over custom sections
        if ((sectionsToIgnore & (1 << sectionCode)) != 0) {
            const unsigned int sectionSize = readVaruint(stdin, (FILE*) 0);
            fseek(stdin, sectionSize, SEEK_CUR);
        } else {
            fputc(sectionCode, stdout);

            const unsigned int sectionSize = readVaruint(stdin, stdout);
            // fprintf(stderr, "Section %i: %i bytes\n", sectionCode, sectionSize);

            //copy the body of the module from stdin to stdout byte-by-byte
            //if this becomes a performance issue, I'll buffer it
            for (unsigned int i = 0; i < sectionSize; ++i) {
                const unsigned char byte = getc(stdin);
                fputc(byte, stdout);
            }
        }
    }

    return 0;
}