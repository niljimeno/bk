#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include <dirent.h>
#include <string.h>
#include <stdbool.h>

void printHelp() {
    printf("Not enough arguments! Usage: bk [origin] [destination]\n");
}

bool pathExists(char *path) {
    return (access(path, F_OK) == 0);
}

bool makeDir(char *path) {
    return (mkdir(path, 0755) && errno != EEXIST);
}

char *trimPath(char *path) {
    size_t len = strlen(path);
    if (len > 1 && path[len - 1] == '/')
        path[len - 1] = '\0';

    return path;
}

char *appendStr(char *a, char *b) {
    int breakpoint;
    int i;

    int a_len = strlen(a);
    int b_len = strlen(b);
    char *c = malloc(sizeof(char) * (a_len+b_len+1));

    for (i=0; i<a_len; i++) {
        c[i] = a[i];
    }

    breakpoint = i;

    for (i=0; i<b_len; i++) {
        c[breakpoint+i] = b[i];
    }

    c[breakpoint+i] = '\0';
    return c;
}

char *appendPath(char *a, char *b) {
    int breakpoint;
    int i;

    int a_len = strlen(a);
    int b_len = strlen(b);
    char *c = malloc(sizeof(char) * (a_len+b_len+2));

    for (i=0; i<a_len; i++) {
        c[i] = a[i];
    }

    c[i] = '/';
    breakpoint = i+1;

    for (i=0; i<b_len; i++) {
        c[breakpoint+i] = b[i];
    }

    c[breakpoint+i] = '\0';
    return c;
}

bool isDir(char *path) {
    struct stat st;
    stat(path, &st);
    return S_ISDIR(st.st_mode);
}

struct Options {
    char *origin;
    char *mirror;
};

struct Entry {
    char *value;
    struct Entry *next;
};

void printAll(struct Entry *input) {
    struct Entry *iterator = input;
    while (iterator != NULL) {
        char *value = iterator->value;
        if (strcmp(value, "") == 0) {
            value = "/";
        }

        printf("- %s\n", value);
        iterator = iterator->next;
    }
}

struct Entry *append(struct Entry *a, struct Entry *b) {
    if (b == NULL) {
        return a;
    }

    if (a == NULL) {
        return b;
    }

    struct Entry *iterator = a;
    while (iterator->next != NULL) {
        iterator = iterator->next;
    }

    iterator->next = b;
    return a;
}

struct Entry *getNewEntriesR(struct Options *op, char *current) {
    struct Entry *result = NULL;

    char *mirrorPath = appendStr(op->mirror, current);
    if (!pathExists(mirrorPath)) {
        result = malloc(sizeof(struct Entry));
        result->value = strdup(current);
        result->next = NULL;
    }
    free(mirrorPath);

    char *originPath = appendStr(op->origin, current);
    if (!isDir(originPath)) {
        free(originPath);
        return result;
    }

    DIR *dir = opendir(originPath);
    free(originPath);
    if (!dir) {
        free(result->value);
        free(result);
        return NULL;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char *newEntryPath = appendPath(current, entry->d_name);
        struct Entry *newEntry = getNewEntriesR(op, newEntryPath);
        free(newEntryPath);
        result = append(result, newEntry);
    }

    closedir(dir);

    return result;
}

struct Entry *getNewEntries(struct Options *op) {
    if (!pathExists(op->origin)) return NULL;
    return getNewEntriesR(op, "");
}

/* generated */
int classicCopy(const char *src, const char *dst) {
    FILE *in = fopen(src, "rb"), *out = fopen(dst, "wb");
    if (!in || !out) { if (in) fclose(in); if (out) fclose(out); return -1; }
    char buf[4096]; size_t n;
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, n, out) != n) { fclose(in); fclose(out); return -1; }
    int err = ferror(in);
    fclose(in); fclose(out);
    return err ? -1 : 0;
}

bool copy(char *source, char *destination) {
    if (isDir(source)) {
        makeDir(destination);
        return 0;
    }

    return classicCopy(source, destination) == 0 ? true : false;
}

void copyNewEntries(struct Options *op, struct Entry *entry) {
    while (entry != NULL) {
        char *source = appendStr(op->origin, entry->value);
        char *destination = appendStr(op->mirror, entry->value);

        printf("Copying %s to %s\n", source, destination);
        bool status = copy(source, destination);
        if (!status)
            printf("Copy failed.\n");

        free(source);
        free(destination);
        entry = entry->next;
    }
}

void removeFullR(char *path) {
    if (!isDir(path)) {
        remove(path);
        return;
    }

    DIR *d = opendir(path);
    if (!d)
        return;

    struct dirent *entry;
    while ((entry = readdir(d))) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char *pathToRemove = appendPath(path, entry->d_name);
        removeFullR(pathToRemove);
        free(pathToRemove);
    }

    remove(path);
}

void removeFull(char *path) {
    if (!pathExists(path)) {
        return;
    }

    removeFullR(path);
}

void removeOldEntries(struct Options *op, struct Entry *entry) {
    while (entry != NULL) {
        char *path = appendStr(op->mirror, entry->value);
        printf("Removing %s\n", path);
        removeFull(path);

        free(path);
        entry = entry->next;
    }
}

char *folderName(char *path) {
    char *div = strchr(path, '/');
    return div ? div + 1 : path;
}

int main(int argc, char **args) {
    if (argc < 3) {
        printHelp();
        return -1;
    }

    struct Options *options = malloc(sizeof(struct Options));
    options->origin = trimPath(args[1]);

    if (args[2][strlen(args[2])-1] == '/') {
        options->mirror = appendStr(args[2], folderName(options->origin));
    } else {
        options->mirror = args[2];
    }

    if (!pathExists(options->origin)) {
        printf("Error: origin file doesn't exist");
        return -1;
    }

    struct Entry *newEntries = getNewEntries(options);

    struct Options *deleteOptions = malloc(sizeof(struct Options));
    deleteOptions->origin = options->mirror;
    deleteOptions->mirror = options->origin;

    struct Entry *oldEntries = getNewEntries(deleteOptions);

    printf("\n[[ Copying from %s to %s ]]\n", options->origin, options->mirror);

    if (newEntries == NULL) {
        printf("\nNo new entries.\n");
    } else {
        printf("\nNew entries to be added:\n");
        printAll(newEntries);
    }

    if (oldEntries == NULL) {
        printf("\nNo old entries.\n");
    } else {
        printf("\nEntries to be removed:\n");
        printAll(oldEntries);
    }

    if (oldEntries == NULL && newEntries == NULL)
        return 0;

    printf("\nProceed? [Y/n]");
    int answer = getchar();
    if (answer != 'y' && answer != 'Y')
        return 0;

    copyNewEntries(options, newEntries);
    removeOldEntries(options, oldEntries);
}
