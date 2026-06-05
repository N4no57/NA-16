#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <string.h>

#include "linker.h"
#include "object_file_reader/obj_reader.h"

#define DEFAULT_OUTFILE "o.bin"
#define LINKER_VERSION "1.0.0"

static struct option long_options[] = {
    {"version", no_argument, nullptr, 0},
    {nullptr, 0, nullptr, 0}
};

char *outfile = DEFAULT_OUTFILE;

void prelude(int argc, char **argv) {
    opterr = 0;

    int option;
    int option_index = 0;
    while ((option = getopt_long(argc, argv, "ho:", long_options, &option_index)) != -1) {
        switch (option) {
            case 'h':
                printf("Usage: nald [options]\n");
                printf("Options:\n");
                printf("  --version          Display current version\n");
                printf("  -h                 Display this help screen\n");
                printf("  -o <file>          Place the output into <file>\n");
                printf("  -C <file>.cfg      Specify a linker config file\n");
                exit(0);
            case 'o':
                outfile = optarg;
                break;
            case 0:
                if (strcmp(long_options[option_index].name, "version") == 0) {
                    printf("NALD.exe version %s compiled on %s\n", LINKER_VERSION, __DATE__);
                } else {
                    printf("NALD: error: unrecognised command-line option '%s'\n", argv[optind-1]);
                    exit(EXIT_FAILURE);
                }
            default:
                printf("NALD: error: unrecognised command-line option '-%c'\n", option);
                exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char **argv) {
    prelude(argc, argv);

    const size_t num_files = argc - optind;

    if (num_files == 0) {
        printf("NALD: fatal: no input files\n");
        return 1;
    }

    ObjectFile *objs = malloc(num_files * sizeof(ObjectFile));

    for (u64 i = optind; i < argc; i++) {
        read_obj(&objs[i-optind], argv[i]);
        objs[i-optind].header.filename = argv[i];
    }

    link(objs, num_files, outfile);

    // cleanup
    for (u64 i = 0; i < num_files; i++) {
        ObjectFile *obj = &objs[i];
        free(obj->section_table);
        free(obj->symbol_table);
        free(obj->relocation_table);
        free(obj->string_table);
        free(obj->data);
    }
    free(objs);

    return 0;
}
