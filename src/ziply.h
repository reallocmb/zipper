#ifndef ZIPLY_H
#define ZIPLY_H

typedef void ZipArchive;
typedef void ArchiveContent;

ZipArchive *ziply_archive_open(char *path);
void ziply_archive_close(ZipArchive *archive);

int ziply_archive_entries_number_get(ZipArchive *archive);

ArchiveContent *ziply_browse(ZipArchive *archive);
char *ziply_browse_visit_directory(ArchiveContent *content, int entries_number, int index);
char *ziply_browse_path_by_index(ArchiveContent *content, int entries_number, int index);
void ziply_browse_step_forwards(ArchiveContent *content, int entries_number, char *path);
void ziply_browse_step_backwards(ArchiveContent *content, int entries_number, char **directory_path);

void ziply_archive_extract(ZipArchive *archive, ArchiveContent *content,  char *extract_path, int index);
void ziply_archive_file_open(ZipArchive *archive, ArchiveContent *content, int index);
void ziply_archive_password_set(ZipArchive *archive, ArchiveContent *content, char *password);
void ziply_archive_include(ZipArchive *archive, char *include_path);
void ziply_archive_bruteforce(ZipArchive *archive, ArchiveContent *content);
void ziply_archive_dictionary_set(ZipArchive *archive, char *dictionary);

/* TUI */
void ziply_archive_content_print(ArchiveContent *content, int entries_number, char *path);

void ziply_browse_input(char *c, int *index);

#endif
