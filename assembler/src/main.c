#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib/asmlib.h"
#include "assembler.h"

/*
 * base inst encoding:
 * bits 15-13: class
 * bits 12-9: opcode
 * bits 8-6: destination reg
 * bits 5-3: source reg 1
 * bits 2-0: source reg 2
 */

/*
 * for the class in the base inst encoding the MSB changes between this data being a prefix or the instruction
 * 0XX = class
 * 1XXX = prefix byte/word
 * prefix bytes/words use the most significant nibble to identify them
 * the code 1111 is reserved as an escape byte for extended instructions
 * this allows for a total of 7 prefixes and 4 distinct classes
 */

/* MEX prefix is used to change the interpretation of an instructions operands or its addressing mode
 * MEX is 2 bytes (word)
 * The most significant nibble is used to identify it and the rest of the nibbles are used to change the addressing mode
 * of each of the 3 operands
 * layout example:
 * bits 15-12 =  MEX tag
 * bits 11-8 = op1 mode
 * bits 7-4 = op2 mode
 * bits 3-0 = op3 mode
 */

/*
 * MEX prefix (0x8)
 * 0x0 = register ✔
 * 0x1 = 8/16 bit immediate ✔/✔
 * 0x2 = register indirect ✔
 * 0x3 = Absolute
 * 0x4 = register indirect ± 8/16-bit displacement
 * 0x5 = SIB addressing
 * 0x6 = SIB ± 8/16-bit displacement
 * 0x7-0xF = reserved
 */

/* the native size when accessing memory or using immediate or offsets/displacements is 1 byte
 * to use a full word size the AEX prefix is required
 * Unlike MEX, AEX does not act on the per operand level. This is per instructions
 * AEX is a single byte
 * AEX = 0x9X
 * low nibble is for the size
 */

/*
 * AEX (0x9)
 * 0x0 = 8-bit
 * 0x1 = 16-bit
 * 0x2-0xF = reserved
 */

/*
 * any immediates will follow the base encoding in the order specified by the MEX prefix
 * absolutes act the same but are just interpreted as absolutes
 */

i32 error_count = 0;

#define OUTPUT_DEFAULT "a.out"
#define ASSEMBLER_VERSION "1.5.3"

const char *ChangeFileExt(char *string) {
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '.') {
            string[i+1] = 'o';
            string[i+2] = '\0';
            break;
        }
    }
    return string;
}

int hasExtension(const char *string) {
    for (int i = 0; i < strlen(string); i++) {
        if (string[i] == '.') {
            return 1;
        }
    }
    return 0;
}

static struct option long_opts[] = {
    {"version", no_argument, nullptr, 0},
    {"help", no_argument, nullptr, 'h'},
    {nullptr, 0, nullptr, 0}
};

char *fout = nullptr;

void prelude(const int argc, char **argv) {
    opterr = 0;

    int option;
    int long_opt_idx = 0;

    while ((option = getopt_long(argc, argv, "ho:v", long_opts, &long_opt_idx)) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: naas filename [options...]\n\n");
                printf("Options:\n\n");
                printf("    -h\tshow this text and exit (also --help)\n");
                printf("    --version\tprint NAAS version number and exit\n");
                printf("    -o\twrite to outfile\n");
                exit(EXIT_SUCCESS);
            case 'o':
                const int length = (int)strlen(optarg);
                free(fout);
                fout = optarg;
                if (!hasExtension(optarg)) {
                    char o[length+3];
                    strcpy(o, optarg);
                    o[length] = '.';
                    o[length+1] = 'o';
                    o[length+2] = 0;
                    fout = strdup(o);
                }
                break;
            case 'v':
                break;
            case 0:
                if (strcmp(long_opts[option].name, "version") == 0) {
                    printf("NAAS.exe version %s compiled on %s\n", ASSEMBLER_VERSION, __DATE__);
                    exit(EXIT_SUCCESS);
                }
                break;
            case '?':
                fprintf(stderr, "NAAS: error: unrecognised command-line option '%s'\n", argv[optind-1]);
                exit(EXIT_FAILURE);
            default:
                fprintf(stderr, "NAAS: error: unrecognised option '-%c'\n", option);
                exit(EXIT_FAILURE);
        }
    }
}

int main(const int argc, char **argv) {
    fout = strdup((char *)OUTPUT_DEFAULT);

    prelude(argc, argv);

    if (argc == optind) {
        fprintf(stderr, "NAAS: fatal error: no input files\n");
        exit(EXIT_FAILURE);
    }

    if (argc - optind != 1 && strcmp(fout, OUTPUT_DEFAULT) != 0) {
        fprintf(stderr, "NAAS: fatal error: cannot use '-o' when assembling multiple files\n");
        exit(EXIT_FAILURE);
    }

    char *fin[argc - optind];

    for (i32 i = optind; i < argc; i++) {
        fin[i - optind] = argv[i];
    }

    if (argc - optind > 1) {
        for (int i = 0; i < argc - optind; i++) {
            assemble(fin[i], ChangeFileExt(fin[i]));
        }
        free(fout);
        return 0;
    }

    int retval = assemble(fin[0], fout);

    free(fout);

    return retval;
}
