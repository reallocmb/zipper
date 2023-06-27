#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<getopt.h>
#include"ziply.h"

struct option long_options[] = {
    { "help", no_argument, 0, 'h' },
    { "open", no_argument, 0, 'o' },
    { "bruteforce", no_argument, 0, 'b' },
    { "dictionary", required_argument, 0, 'd' },
    { "password", required_argument, 0, 'p' },
    { "extract", required_argument, 0, 'e' },
    { "include", required_argument, 0, 'i' },
};

/* print's the usage of the program to the terminal */
void help_print(void)
{
    puts("Usage zipper [OPTIONS] [ZIP_FILE]");
    printf("-%c, --%s\t%s\n", (char)long_options[0].val, long_options[0].name, "Show this help");
    printf("-%c, --%s\t%s\n", (char)long_options[1].val, long_options[1].name, "Open a zip file for browsing");
    printf("-%c, --%s\t%s\n", (char)long_options[2].val, long_options[2].name, "Try to bruteforce the password");
    printf("-%c, --%s [FILE]\t%s\n", (char)long_options[3].val, long_options[3].name, "Try to bruteforce the password with a dictionary");
    printf("-%c, --%s [PASSWORD]\t%s\n", (char)long_options[4].val, long_options[4].name, "Use this password");
    printf("-%c, --%s [FILE]\t%s\n", (char)long_options[5].val, long_options[5].name, "Extract this file");
    printf("-%c, --%s [FILE]\t%s\n", (char)long_options[6].val, long_options[6].name, "Include this file");
}

void archive_browse(ZipArchive *archive, ArchiveContent *content, int entries_number)
{
    ziply_archive_content_print(content, entries_number, NULL);

    int index;
    char c;
    char *directory_path = NULL;
    while (1) {
        /* input */
        ziply_browse_input(&c, &index);
        switch (c) {
            case 'e':
                ziply_archive_extract(archive, content, "./", index);
                break;
            case 'v':
                directory_path = ziply_browse_path_by_index(content, entries_number, index);
                ziply_browse_step_forwards(content, entries_number, directory_path);
                break;
            case 'b':
                ziply_browse_step_backwards(content, entries_number, &directory_path);
                break;
            case 'o':
                ziply_archive_file_open(archive, content, index);
                break;
            case 'q':
                exit(0);
                break;
        }
        ziply_archive_content_print(content, entries_number, directory_path);
    }
}

int main(int argc, char **argv)
{
    ZipArchive *archive;
    ArchiveContent *content;
    int entries_number;

    if (argc > 2) {
        archive = ziply_archive_open(argv[argc - 1]);
        content = ziply_browse(archive);
        entries_number = ziply_archive_entries_number_get(archive);
    }

    if (argc < 2) {
        help_print();
        return EXIT_SUCCESS;
    }

    char c;
    int option_index;
    bool brute_force = false;

    while ((c = getopt_long(argc, argv, "hobd:p:e:i:", long_options, &option_index)) != -1) {
        switch (c) {
            case 'h':
                help_print();
                return 0;
            case 'o':
                archive_browse(archive, content, entries_number);
                break;
            case 'b':
                brute_force = true;
                break;
            case 'd':
                ziply_archive_dictionary_set(archive, optarg);
                brute_force = true;
                break;
            case 'p':
                ziply_archive_password_set(archive, content, optarg);
                archive_browse(archive, content, entries_number);
                break;
            case 'e':
                ziply_archive_extract(archive, content, optarg, -1);
                break;
            case 'i':
                ziply_archive_include(archive, optarg);
                break;
        }
    }
    if (brute_force)
        ziply_archive_bruteforce(archive, content);

    free(content);
    ziply_archive_close(archive);

    return EXIT_SUCCESS;
}
