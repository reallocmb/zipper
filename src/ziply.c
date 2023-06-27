#include<zip.h>
#include<stdlib.h>
#include<stdbool.h>
#include<string.h>
#include<sys/stat.h>
#include<dirent.h>

typedef struct {
    zip_t *zip;
    char *path;
    bool encrypted;
    int entries_number;
    char *dictionary;
} ZipArchive;

typedef struct {
    char *path;
    bool show;
} ArchiveContent;


void libzip_error_handle(ZipArchive *archive, bool condition, char *message, int err)
{
    if (condition) {
        zip_error_t error;

        zip_error_init_with_code(&error, err);
        fprintf(stderr, "%s: %s '%s': %s\n",
                "zipper", message, archive->path, zip_error_strerror(&error));
        zip_error_fini(&error);
        exit(EXIT_SUCCESS);
    }
}

void archive_check_for_encryption(ZipArchive *archive)
{
    zip_stat_t zip_stat;
    int err;
    err = zip_stat_index(archive->zip, 0, ZIP_FL_UNCHANGED, &zip_stat);
    libzip_error_handle(archive, err == -1, "cannot open stat of archive", err);
    archive->encrypted = zip_encryption_method_supported(zip_stat.encryption_method, 0);
}

ZipArchive *ziply_archive_open(char *path)
{
    ZipArchive *archive = malloc(sizeof(*archive));
    if (archive == NULL) { fprintf(stderr, "malloc failed FILE: %s, LINE: %d", __FILE__, __LINE__); }

    archive->path = path;

    /* libzip open the archive */
    int err;
    archive->zip = zip_open(path, 0, &err);
    libzip_error_handle(archive, archive->zip == NULL, "cannot open zip archive", err);

    archive_check_for_encryption(archive);

    archive->entries_number = zip_get_num_entries(archive->zip, ZIP_FL_UNCHANGED);
    libzip_error_handle(archive, archive->entries_number == -1, "is a empty archive", archive->entries_number);

    return archive;
}

int ziply_archive_entries_number_get(ZipArchive *archive)
{
    return archive->entries_number;
}


bool browse_is_sub_directory(char *path)
{
    
    char *ptr = strchr(path, '/');
    if (ptr)
        if (ptr[1] != 0)
            return true;

    return false;
}

char *ziply_browse_path_by_index(ArchiveContent *content, int entries_number, int index)
{
    int i;
    int k;
    for (i = 0, k = 1; i < entries_number; i++) {
        if (content[i].show) {
            if (k == index)
                return content[i].path;
            k++;
        }
    }

    return NULL;
}

void ziply_browse_step_forwards(ArchiveContent *content, int entries_number, char *path)
{
    int path_len = strlen(path);
    int i;

    for (i = 0; i < entries_number; i++) {
        if (strncmp(path, content[i].path, path_len) == 0 &&
                browse_is_sub_directory(content[i].path + path_len) == false &&
                strcmp(path, content[i].path) != 0)
            content[i].show = true;
        else
            content[i].show = false;
    }
}

void ziply_browse_step_backwards(ArchiveContent *content, int entries_number, char **directory_path)
{
    if (*directory_path == NULL)
        return;


    int i;
    int k;
    int directory_path_len = strlen(*directory_path);

    for (i = directory_path_len - 2; (*directory_path)[i] != '/'; i--) {
        if (i <= 0) {
            *directory_path = NULL;
            ziply_browse_step_forwards(content, entries_number, "");
            return;
        }
    }

    k = i;

    for (i = 0; i < entries_number; i++) {
        if (strncmp(*directory_path, content[i].path, k + 1) == 0) {
            *directory_path = content[i].path;
            ziply_browse_step_forwards(content, entries_number, content[i].path);
            return;
        }
    }
    
}

ArchiveContent *ziply_browse(ZipArchive *archive)
{
    ArchiveContent *content = malloc(archive->entries_number * sizeof(*content));
    if (content == NULL) { fprintf(stderr, "malloc failed FILE: %s, LINE: %d", __FILE__, __LINE__); }

    int i;
    for (i = 0; i < archive->entries_number; i++) {
        content[i].path = (char *)zip_get_name(archive->zip, i, ZIP_FL_UNCHANGED);
        if (browse_is_sub_directory(content[i].path) == false) {
            content[i].show = true;
        } else
            content[i].show = false;
    }

    return content;
}

void extract(ZipArchive *archive, ArchiveContent *content,  char *extract_path, int i)
{
    char new_path[150];

    strcpy(new_path, extract_path);
    if (extract_path[strlen(extract_path) - 1] != '/') {
        strcat(new_path, "/");
    }
    /* if dir create dir */
    if (content[i].path[strlen(content[i].path) - 1] == '/') {
        strcat(new_path, content[i].path);
        mkdir(new_path, 0777);
    } else {
        strcat(new_path, content[i].path);
        zip_file_t *zip_file = zip_fopen_index(archive->zip, i, 0);
        char buffer[50] = { 0 };
        FILE *f = fopen(new_path, "w+");
        if (f == NULL) {
            char *ptr = content[i].path;;
            while ((ptr = strchr(ptr, '/'))) {
                char dir_buffer[100];
                strncpy(dir_buffer, content[i].path, ptr - content[i].path);
                mkdir(dir_buffer, 0777);
            }
        }
        unsigned int bytes;
        while ((bytes = zip_fread(zip_file, buffer, 50)) > 0) {
            fwrite(buffer, 1, bytes, f);
        }
        fclose(f);
        zip_fclose(zip_file);
    }
}

/* extract the archive */
/* input:
 * ./
 * folder
 * folder/subfolder
 * folder/subfolder/
 * ./folder
 */
void ziply_archive_extract(ZipArchive *archive, ArchiveContent *content, char *extract_path, int index)
{
    int i;
    int k;

    /* make extract_path folder if dosen't exist */
    struct stat st;
    if (stat(extract_path, &st) == -1) {
        mkdir(extract_path, 0777);
    }

    if (index == -1) {
        for (i = 0; i < archive->entries_number; i++) {
            extract(archive, content, extract_path, i);
        }
        return;
    }

    char *path = NULL;

    for (i = 0, k = 1; i < archive->entries_number; i++) {
        if (content[i].show) {
            if (k == index) {
                path = content[i].path;
                break;
            }
            k++;
        }
    }

    if (path == NULL) {
        printf("cant find file to extract\n");
    }

    for (i = i; i < archive->entries_number; i++) {
        if (strncmp(path, content[i].path, strlen(path)) == 0 || index == -1) {
            extract(archive, content, extract_path, i);
        }
    }
}

void ziply_archive_file_open(ZipArchive *archive, ArchiveContent *content, int index)
{
    int i;
    int k;
    for (i = 0, k = 1; i < archive->entries_number; i++) {
        if (content[i].show) {
            if (k == index) {
                break;
            }
            k++;
        }
    }

    zip_file_t *zip_file = zip_fopen_index_encrypted(archive->zip, i, ZIP_FL_UNCHANGED, NULL);
    libzip_error_handle(archive, zip_file == NULL, "can't open file", 0);

    printf("/* File: %s */\n", content[i].path);
    char buffer[50] = { 0 };
    int bytes;
    while ((bytes = zip_fread(zip_file, buffer, 50))) {
        printf("%*s\n", bytes, buffer);
    }
    printf("/* File End */\n");
}

bool password_check(ZipArchive *archive, ArchiveContent *content, char *password)
{
    zip_set_default_password(archive->zip, password);

    int i;
    for (i = 0; i < archive->entries_number; i++) {
        if (content[i].path[strlen(content[i].path) - 1] != '/') {
            zip_file_t *zip_file = zip_fopen_encrypted(archive->zip, content[i].path, ZIP_FL_ENC_RAW, NULL);
            if (zip_file != NULL) {
                zip_fclose(zip_file);
                return true;
            }
        }
    }

    return false;
}

/* prints out if the password is corret or wrong */
void ziply_archive_password_set(ZipArchive *archive, ArchiveContent *content, char *password)
{
    if (!archive->encrypted) {
        printf("you can open with option -o \n");
    }

    if (password_check(archive, content, password)) {
        puts("correct password");
    } else {
        printf("wrong password\n");
        exit(0);
    }
}

bool is_directory(char *path)
{
    struct stat st;
    if (stat(path, &st) == -1) {

    }

    if (st.st_mode & S_IFDIR)
        return true;

    return false;
}

void zip_archive_add_file(zip_t *zip_archive, char *path)
{
    zip_source_t *source = zip_source_file(zip_archive, path, 0, 0);
    if (zip_file_add(zip_archive, path, source, ZIP_FL_ENC_UTF_8) == -1)
        puts("zip_file_add error");
}

void zip_archive_add_directory(zip_t *zip_archive, char *path)
{
    int len = strlen(path);
    if (path[len - 1] == '/')
        path[len - 1] = 0;

    if (zip_dir_add(zip_archive, path, ZIP_FL_ENC_UTF_8) == - 1)
        puts("zip_dir_add error");

    struct dirent *dir_ptr;
    DIR *dir = opendir(path);
    char tem_path[150] = { 0 };
    while ((dir_ptr = readdir(dir)) != NULL) {
        if (dir_ptr->d_name[0] == '.')
            continue;

        strcpy(tem_path, path);
        strcat(tem_path, "/");
        strcat(tem_path, dir_ptr->d_name);
        if (is_directory(tem_path)) {
            printf("d-%s\n", tem_path);
            zip_archive_add_directory(zip_archive, tem_path);
            continue;
        }

        zip_archive_add_file(zip_archive, tem_path);
    }
}

/* include a file or a directory to the zip archive */
void ziply_archive_include(ZipArchive *archive, char *include_path)
{
    if (is_directory(include_path))
        zip_archive_add_directory(archive->zip, include_path);
    else
        zip_archive_add_file(archive->zip, include_path);
}

void ziply_archive_dictionary_set(ZipArchive *archive, char *dictionary)
{
    archive->dictionary = dictionary;
}

void brute_force(ZipArchive *archive, ArchiveContent *content, char *charset, char *password, int length, int position)
{
    int charsetLength = strlen(charset);

    if (position == length) {
        printf("%s\n", password);
        if (password_check(archive, content, password)) {
            printf("password found: %s\n", password);
            exit(0);
        }
        return;
    }

    for (int i = 0; i < charsetLength; i++) {
        password[position] = charset[i];
        brute_force(archive, content, charset, password, length, position + 1);
    }
}

/* try to brutforce the zip file with dictonary or without */
void ziply_archive_bruteforce(ZipArchive *archive, ArchiveContent *content)
{
    if (archive->dictionary == NULL) {
        char charset[] = "abcdefghijklmnopqrstuvwxyz";
        int passwordLength = 4;
        char password[passwordLength + 1];

        brute_force(archive, content, charset, password, passwordLength, 0);
    } else {
        FILE *dictionary_file = fopen(archive->dictionary, "r");
        char buffer[100];
        while (fgets(buffer, 100, dictionary_file)) {
            buffer[strlen(buffer) - 1] = 0;
            if (password_check(archive, content, buffer)) {
                printf("password was found: %s\n", buffer);
                fclose(dictionary_file);
                return;
            }
        }
        printf("password was not found");

        fclose(dictionary_file);
    }
}

/* cleanup */
void ziply_archive_close(ZipArchive *archive)
{
    zip_close(archive->zip);
    zip
    free(archive);
}


/* TUI */
void ziply_archive_content_print(ArchiveContent *content, int entries_number, char *path)
{
    int path_len = 0;
    int i = 0;
    int k;

    puts("Actions:\nextract file or directory(e), visit directory(v), go back(b), open file(o), quit(q)");

    /* print directory */
    if (path == NULL) {
        puts("./");
    } else {
        printf("%s\n", path);
        path_len = strlen(path);
    }


    /* print content */
    for (k = 1; i < entries_number; i++) {
        if (content[i].show) {
            printf("%d. %s\n", k, content[i].path + path_len);
            k++;
        }
    }
}

void ziply_browse_input(char *c, int *index)
{
    *index = 0;
    *c = 0;

    int input;
    *c = getchar();

    input = getchar();
    if (input == '\n')
        return;
    *index = input - 0x30;

    while ((input = getchar()) != '\n') {
        *index *= 10;
        *index += input - 0x30;
    }
}
