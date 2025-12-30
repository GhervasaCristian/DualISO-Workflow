#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <direct.h>
#include <windows.h>
#include <sys/stat.h>

#define CONFIG_FILE "config.inf"
#define BATCH 3
#define PATH_LEN 1024
#define MAX_FILES 4096

// ==========================================
//           HELPER FUNCTIONS
// ==========================================

int folder_exists(const char *path) {
    DWORD attr = GetFileAttributesA(path);
    return (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY));
}

void make_folder(const char *path) {
    if (!folder_exists(path)) {
        _mkdir(path);
    }
}

void delete_folder(const char *path) {
    RemoveDirectoryA(path);
}

void get_prefix(const char *folderName, char *prefix) {
    char *lastUnderscore = strrchr(folderName, '_');
    if (lastUnderscore) {
        size_t len = lastUnderscore - folderName;
        strncpy(prefix, folderName, len);
        prefix[len] = '\0';
    } else {
        strcpy(prefix, folderName);
    }
}

void get_config_path(char *buffer, int size) {
    FILE *cfg = fopen(CONFIG_FILE, "r");
    int found = 0;
    if (cfg) {
        if (fgets(buffer, size, cfg)) {
            buffer[strcspn(buffer, "\r\n")] = 0;
            if (strlen(buffer) > 0) found = 1;
        }
        fclose(cfg);
    }
    if (!found) {
        printf("Config not found or empty.\nEnter base path: ");
        fgets(buffer, size, stdin);
        buffer[strcspn(buffer, "\r\n")] = 0;
        cfg = fopen(CONFIG_FILE, "w");
        if (cfg) {
            fprintf(cfg, "%s\n", buffer);
            fclose(cfg);
        }
    } else {
        printf("Loaded path from %s: %s\n", CONFIG_FILE, buffer);
    }
}

// ==========================================
//           UNPACK LOGIC
// ==========================================

void run_unpack(const char *basePath) {
    printf("\n--- STARTING UNPACK ---\n");
    char inRoot[PATH_LEN];
    snprintf(inRoot, sizeof(inRoot), "%s\\IN", basePath);

    if (!folder_exists(inRoot)) {
        printf("Error: 'IN' folder not found at: %s\n", inRoot);
        return;
    }

    WIN32_FIND_DATAA ffd;
    char searchPath[PATH_LEN];
    snprintf(searchPath, sizeof(searchPath), "%s\\*", inRoot);
    HANDLE hFind = FindFirstFileA(searchPath, &ffd);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("No folders found in IN.\n");
        return;
    }

    do {
        if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0) continue;

            char folder[PATH_LEN];
            snprintf(folder, sizeof(folder), "%s\\%s", inRoot, ffd.cFileName);

            char prefix[PATH_LEN];
            get_prefix(ffd.cFileName, prefix);

            char emptyFolder[PATH_LEN];
            snprintf(emptyFolder, sizeof(emptyFolder), "%s\\%s", folder, prefix);
            make_folder(emptyFolder);

            char **files = malloc(MAX_FILES * sizeof(char*));
            for (int i = 0; i < MAX_FILES; i++) files[i] = malloc(PATH_LEN);
            int count = 0;

            char fileSearch[PATH_LEN];
            snprintf(fileSearch, sizeof(fileSearch), "%s\\*", folder);
            WIN32_FIND_DATAA ffdFile;
            HANDLE hFileFind = FindFirstFileA(fileSearch, &ffdFile);
            if (hFileFind != INVALID_HANDLE_VALUE) {
                do {
                    if (!(ffdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                        snprintf(files[count], PATH_LEN, "%s\\%s", folder, ffdFile.cFileName);
                        count++;
                        if (count >= MAX_FILES) break;
                    }
                } while (FindNextFileA(hFileFind, &ffdFile));
                FindClose(hFileFind);
            }

            int index = 0;
            int subNum = 0;
            while (index < count) {
                char subFolder[PATH_LEN];
                snprintf(subFolder, sizeof(subFolder), "%s\\%s_%06d", folder, prefix, subNum);
                make_folder(subFolder);
                for (int k = 0; k < BATCH && index < count; k++, index++) {
                    char dstFile[PATH_LEN];
                    snprintf(dstFile, PATH_LEN, "%s\\%s", subFolder, strrchr(files[index], '\\') + 1);
                    MoveFileA(files[index], dstFile);
                }
                subNum++;
            }
            for (int i = 0; i < MAX_FILES; i++) free(files[i]);
            free(files);
            printf("Processed: %s\n", ffd.cFileName);
        }
    } while (FindNextFileA(hFind, &ffd));
    FindClose(hFind);
    printf("Unpack Complete.\n");
}

// ==========================================
//           REPACK LOGIC (PATCHED)
// ==========================================

void repack_move_files_recursive(const char *src, const char *dest) {
    char query[PATH_LEN];
    snprintf(query, sizeof(query), "%s\\*.*", src);
    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(query, &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char from[PATH_LEN], to[PATH_LEN];
            snprintf(from, sizeof(from), "%s\\%s", src, fd.cFileName);
            snprintf(to, sizeof(to), "%s\\%s", dest, fd.cFileName);
            MoveFile(from, to);
        }
    } while (FindNextFile(h, &fd));
    FindClose(h);
}

void run_repack(const char *basePath) {
    printf("\n--- STARTING REPACK ---\n");

    // PATCH: Target the OUT folder inside the base path
    char outRoot[PATH_LEN];
    snprintf(outRoot, sizeof(outRoot), "%s\\OUT", basePath);

    if (!folder_exists(outRoot)) {
        printf("Error: 'OUT' folder not found at: %s\n", outRoot);
        return;
    }

    printf("Targeting: %s\n", outRoot);

    char query[PATH_LEN];
    snprintf(query, sizeof(query), "%s\\*", outRoot);

    WIN32_FIND_DATA fd;
    HANDLE h = FindFirstFile(query, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        printf("Error: Could not open directory '%s'\n", outRoot);
        return;
    }

    do {
        if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
            strcmp(fd.cFileName, ".") != 0 &&
            strcmp(fd.cFileName, "..") != 0) {

            char parent_path[PATH_LEN];
            snprintf(parent_path, sizeof(parent_path), "%s\\%s", outRoot, fd.cFileName);

            char subQuery[PATH_LEN];
            snprintf(subQuery, sizeof(subQuery), "%s\\*", parent_path);
            WIN32_FIND_DATA subFd;
            HANDLE hSub = FindFirstFile(subQuery, &subFd);

            if (hSub != INVALID_HANDLE_VALUE) {
                do {
                     if ((subFd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                        strcmp(subFd.cFileName, ".") != 0 &&
                        strcmp(subFd.cFileName, "..") != 0) {

                        char subfolder_path[PATH_LEN];
                        snprintf(subfolder_path, sizeof(subfolder_path), "%s\\%s", parent_path, subFd.cFileName);

                        printf("Flattening %s -> into %s\n", subFd.cFileName, fd.cFileName);
                        repack_move_files_recursive(subfolder_path, parent_path);
                        delete_folder(subfolder_path);
                    }
                } while (FindNextFile(hSub, &subFd));
                FindClose(hSub);
            }
        }
    } while (FindNextFile(h, &fd));

    FindClose(h);
    printf("Repack Complete.\n");
}

// ==========================================
//           MAIN MENU
// ==========================================

int main() {
    char basePath[PATH_LEN] = {0};
    int choice = 0;

    get_config_path(basePath, sizeof(basePath));

    while (1) {
        printf("\n============================\n");
        printf("        FILE MANAGER        \n");
        printf("============================\n");
        printf("Current Path: %s\n", basePath);
        printf("1. Unpack (Processes %s\\IN)\n", basePath);
        printf("2. Repack (Processes %s\\OUT)\n", basePath); // UI updated
        printf("3. Exit\n");
        printf("Select option: ");

        if (scanf("%d", &choice) != 1) {
            while(getchar() != '\n');
            choice = 0;
        }
        getchar();

        switch (choice) {
            case 1: run_unpack(basePath); break;
            case 2: run_repack(basePath); break;
            case 3: printf("Exiting...\n"); return 0;
            default: printf("Invalid selection. Try again.\n");
        }
    }
    return 0;
}
