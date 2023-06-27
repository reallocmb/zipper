/*
 * File: cdb.c
 * Autor: Matthias Brunner
 * Copyright © by Matthias Brunner
 */

#include<stdio.h>
#include<assert.h>

int main(int argc, char **argv)
{
    system("make clean");
    FILE *f = popen("make", "r");
    assert(f != NULL);

    FILE *temp = fopen(".temp", "r+");
    assert(temp != NULL);

    char c;
    int file_length = 0;
    while ((c = fgetc(f)) != EOF) {
        fputc(c, temp);
        file_length++;
    }


    FILE *compile_commands_file = fopen("compile_commands.json", "w+");
    assert(compile_commands_file != NULL);

    printf("%d\n", file_length);

    fprintf(compile_commands_file,
            "[\n"
            "{\n"
            "\"arguments\": [\n");


    fputc('"', compile_commands_file);
    int i;
    rewind(temp);
    for (i = 0; i < file_length - 1; i++) {
        c = fgetc(temp);
        if (c == ' ' || c == '\n') {
            fputc('"', compile_commands_file);
            fputc(',', compile_commands_file);
            fputc('\n', compile_commands_file);
            fputc('"', compile_commands_file);
            continue;
        }
        fputc(c, compile_commands_file);
    }
    fputc('"', compile_commands_file);
    fputc(',', compile_commands_file);

    fprintf(compile_commands_file,
            "],\n"
            "\"file\": \"./bin/zipper\"\n"
            "}\n"
            "]\n");

    pclose(f);
    fclose(compile_commands_file);
    fclose(temp);

    return 0;
}
