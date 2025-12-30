// make_placeholders.c
// Create tiny 1-byte placeholder copies of files from FolderA into FolderB
// Compile (MSVC): cl /O2 /W4 make_placeholders.c
// Compile (MinGW): gcc -O2 -Wall -o make_placeholders.exe make_placeholders.c

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>

void die(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    ExitProcess(1);
}

void print_usage(const char *exe) {
    printf("Usage: %s <FolderA> <FolderB>\n", exe);
    printf("Scans FolderA (top-level) and creates 1-byte placeholder files with the same names in FolderB.\n");
}

int ensure_dir_exists(const char *path) {
    if (CreateDirectoryA(path, NULL)) return 1;
    DWORD err = GetLastError();
    if (err == ERROR_ALREADY_EXISTS) return 1;
    // Could not create directory
    return 0;
}

int join_path(char *out, size_t outsz, const char *dir, const char *name) {
    size_t dlen = strlen(dir);
    size_t nlen = strlen(name);
    if (dlen + 1 + nlen + 1 > outsz) return 0;
    strcpy(out, dir);
    if (dlen > 0 && out[dlen-1] != '\\' && out[dlen-1] != '/') {
        out[dlen] = '\\';
        out[dlen+1] = 0;
    }
    strcat(out, name);
    return 1;
}

int main(int argc, char **argv) {
    if (argc < 3) {
        print_usage(argv[0]);
        return 1;
    }

    const char *folderA = argv[1];
    const char *folderB = argv[2];

    // ensure folderA exists
    DWORD attrA = GetFileAttributesA(folderA);
    if (attrA == INVALID_FILE_ATTRIBUTES || !(attrA & FILE_ATTRIBUTE_DIRECTORY)) {
        fprintf(stderr, "ERROR: Folder A not found or not a directory: %s\n", folderA);
        return 2;
    }

    // ensure folderB exists or create it
    if (!ensure_dir_exists(folderB)) {
        fprintf(stderr, "ERROR: Cannot create or access Folder B: %s\n", folderB);
        return 3;
    }

    // Build search pattern: folderA\*
    char pattern[MAX_PATH];
    if (!join_path(pattern, sizeof(pattern), folderA, "*")) {
        die("path too long");
    }

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        printf("No files found in %s (or could not access it).\n", folderA);
        return 0;
    }

    int created = 0;
    int skipped_dirs = 0;

    do {
        // skip directories "." and ".." and any directory entries
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            skipped_dirs++;
            continue;
        }

        // Build destination path
        char dest[MAX_PATH];
        if (!join_path(dest, sizeof(dest), folderB, fd.cFileName)) {
            fprintf(stderr, "WARN: destination path too long for %s — skipping\n", fd.cFileName);
            continue;
        }

        // Create/overwrite destination file
        HANDLE hOut = CreateFileA(dest,
                                 GENERIC_WRITE,
                                 0,
                                 NULL,
                                 CREATE_ALWAYS,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
        if (hOut == INVALID_HANDLE_VALUE) {
            DWORD e = GetLastError();
            fprintf(stderr, "WARN: failed to create %s (err %lu) — skipping\n", dest, e);
            continue;
        }

        // write a single zero byte
        DWORD written = 0;
        BYTE zero = 0;
        if (!WriteFile(hOut, &zero, 1, &written, NULL) || written != 1) {
            DWORD e = GetLastError();
            fprintf(stderr, "WARN: failed to write to %s (err %lu)\n", dest, e);
            CloseHandle(hOut);
            continue;
        }

        // Optionally, we can set the file pointer and SetEndOfFile to ensure exact size 1
        SetFilePointer(hOut, 1, NULL, FILE_BEGIN);
        SetEndOfFile(hOut);

        CloseHandle(hOut);
        created++;
        printf("[+] Created placeholder: %s (1 byte)\n", dest);

    } while (FindNextFileA(h, &fd));

    FindClose(h);

    printf("Done. Placeholders created: %d. Directories skipped: %d\n", created, skipped_dirs);
    return 0;
}
