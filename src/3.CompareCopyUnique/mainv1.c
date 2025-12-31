// compares.c
// Compile with MSVC: cl /O2 /W4 compares.c
// Or with MinGW: gcc -O2 -Wall -municode -o compares.exe compares.c

#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

typedef struct {
    char *basename;   // lowercased basename (no extension)
    char *fullpath;   // full path to the file
} FileEntry;

typedef struct {
    FileEntry *arr;
    size_t size;
    size_t capacity;
} DynArray;

static const char *exts[] = { "cr2", "dng" };
static const size_t ext_count = sizeof(exts)/sizeof(exts[0]);
int verbose = 1;

void die(const char *msg) {
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) die("Out of memory");
    return p;
}

void dyn_init(DynArray *d) {
    d->arr = NULL; d->size = 0; d->capacity = 0;
}

void dyn_push(DynArray *d, const char *basename, const char *fullpath) {
    if (d->size == d->capacity) {
        size_t newcap = d->capacity ? d->capacity * 2 : 64;
        d->arr = (FileEntry*)realloc(d->arr, newcap * sizeof(FileEntry));
        if (!d->arr) die("Out of memory");
        d->capacity = newcap;
    }
    d->arr[d->size].basename = _strdup(basename);
    d->arr[d->size].fullpath = _strdup(fullpath);
    d->size++;
}

void dyn_free(DynArray *d) {
    if (!d) return;
    for (size_t i = 0; i < d->size; ++i) {
        free(d->arr[i].basename);
        free(d->arr[i].fullpath);
    }
    free(d->arr);
    d->arr = NULL;
    d->size = d->capacity = 0;
}

static int stricmp_local(const char *a, const char *b) {
    while (*a && *b) {
        char ca = tolower((unsigned char)*a);
        char cb = tolower((unsigned char)*b);
        if (ca != cb) return (int)(unsigned char)ca - (int)(unsigned char)cb;
        a++; b++;
    }
    return (int)(unsigned char)tolower((unsigned char)*a) - (int)(unsigned char)tolower((unsigned char)*b);
}

int has_valid_ext(const char *name) {
    // find last dot
    const char *dot = strrchr(name, '.');
    if (!dot) return 0;
    const char *ext = dot + 1;
    for (size_t i = 0; i < ext_count; ++i) {
        if (stricmp_local(ext, exts[i]) == 0) return 1;
    }
    return 0;
}

void extract_basename_lower(const char *filename, char *out, size_t outsz) {
    // filename is just name.ext (no path). We copy name part lowercased.
    const char *dot = strrchr(filename, '.');
    size_t len = dot ? (size_t)(dot - filename) : strlen(filename);
    if (len >= outsz) len = outsz - 1;
    for (size_t i = 0; i < len; ++i) out[i] = (char)tolower((unsigned char)filename[i]);
    out[len] = 0;
}

void join_path(char *dst, size_t dstsz, const char *dir, const char *name) {
    size_t dlen = strlen(dir);
    if (dlen + 1 + strlen(name) + 1 > dstsz) die("Path too long");
    strcpy(dst, dir);
    // ensure backslash separator
    if (dlen > 0 && (dst[dlen-1] != '\\' && dst[dlen-1] != '/')) {
        dst[dlen] = '\\';
        dst[dlen+1] = 0;
        strcat(dst, name);
    } else {
        strcat(dst, name);
    }
}

void scan_folder(const char *folder, DynArray *out) {
    // Build search pattern "folder\*"
    char pattern[MAX_PATH];
    if (folder[strlen(folder)-1] == '\\' || folder[strlen(folder)-1] == '/')
        snprintf(pattern, sizeof(pattern), "%s*", folder);
    else
        snprintf(pattern, sizeof(pattern), "%s\\*", folder);

    WIN32_FIND_DATAA fd;
    HANDLE h = FindFirstFileA(pattern, &fd);
    if (h == INVALID_HANDLE_VALUE) {
        // folder may be empty or nothing matched - that's okay
        if (verbose) printf("[V] Scanning '%s' -> no files or can't access\n", folder);
        return;
    }
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue; // skip directories
        // check extension
        if (!has_valid_ext(fd.cFileName)) continue;
        // extract basename
        char basename[260];
        extract_basename_lower(fd.cFileName, basename, sizeof(basename));
        // build full path
        char full[MAX_PATH];
        join_path(full, sizeof(full), folder, fd.cFileName);
        // add
        dyn_push(out, basename, full);
        if (verbose) printf("[V] Found: %s (base=%s)\n", full, basename);
    } while (FindNextFileA(h, &fd));
    FindClose(h);
}

int cmp_entry_basename(const void *p1, const void *p2) {
    const FileEntry *a = (const FileEntry*)p1;
    const FileEntry *b = (const FileEntry*)p2;
    return stricmp_local(a->basename, b->basename);
}

void ensure_dir(const char *path) {
    // Try CreateDirectory; if exists it's fine
    if (!CreateDirectoryA(path, NULL)) {
        DWORD e = GetLastError();
        if (e == ERROR_ALREADY_EXISTS) return;
        // If parent doesn't exist, try to create parents (simple approach)
        // For brevity, we just try to fail gracefully
        if (e != ERROR_ALREADY_EXISTS) {
            char buf[512];
            snprintf(buf, sizeof(buf), "Failed to create directory '%s' (err %lu)", path, e);
            die(buf);
        }
    }
}

void copy_only_unique_basenames(const DynArray *A, const DynArray *B, const char *folderC, int *copiedCount) {
    // Build sets of basenames for A and B by sorting the arrays (we expect the arrays are already sorted)
    // We'll walk unique basenames in A and B using two-pointer merge technique.

    // Make arrays of pointers to entries for easier sorting without copying memory
    FileEntry *Acopy = NULL, *Bcopy = NULL;
    Acopy = (FileEntry*)malloc(A->size * sizeof(FileEntry));
    Bcopy = (FileEntry*)malloc(B->size * sizeof(FileEntry));
    if (!Acopy || !Bcopy) die("Out of memory");

    for (size_t i=0;i<A->size;i++) Acopy[i] = A->arr[i];
    for (size_t i=0;i<B->size;i++) Bcopy[i] = B->arr[i];

    qsort(Acopy, A->size, sizeof(FileEntry), cmp_entry_basename);
    qsort(Bcopy, B->size, sizeof(FileEntry), cmp_entry_basename);

    // We'll find unique basenames: iterate through sorted arrays and build unique name lists (in-place)
    size_t ia = 0, ib = 0;
    // iterate unique basenames via pointers
    while (ia < A->size || ib < B->size) {
        char *nameA = (ia < A->size) ? Acopy[ia].basename : NULL;
        char *nameB = (ib < B->size) ? Bcopy[ib].basename : NULL;

        int cmp;
        if (nameA && nameB) cmp = stricmp_local(nameA, nameB);
        else if (nameA) cmp = -1;
        else cmp = 1;

        if (cmp == 0) {
            // basename exists in both -> skip all entries with this basename in both arrays
            char *cur = nameA;
            // skip A entries with this basename
            while (ia < A->size && stricmp_local(Acopy[ia].basename, cur) == 0) ia++;
            // skip B entries with this basename
            while (ib < B->size && stricmp_local(Bcopy[ib].basename, cur) == 0) ib++;
        } else if (cmp < 0) {
            // nameA only in A -> copy all entries from A with this basename
            char curname[512];
            strncpy(curname, nameA, sizeof(curname)); curname[sizeof(curname)-1]=0;
            if (verbose) printf("[V] Unique basename in A: %s\n", curname);
            while (ia < A->size && stricmp_local(Acopy[ia].basename, curname) == 0) {
                // copy Acopy[ia].fullpath to folderC
                char dest[MAX_PATH];
                const char *src = Acopy[ia].fullpath;
                const char *p = strrchr(src, '\\');
                const char *fname = p ? p+1 : src;
                join_path(dest, sizeof(dest), folderC, fname);
                if (CopyFileA(src, dest, FALSE)) {
                    (*copiedCount)++;
                    if (verbose) printf("[V] Copied from A: %s -> %s\n", src, dest);
                } else {
                    DWORD e = GetLastError();
                    fprintf(stderr, "WARN: Copy failed %s -> %s (err %lu)\n", src, dest, e);
                }
                ia++;
            }
        } else {
            // nameB only in B -> copy all entries from B with this basename
            char curname[512];
            strncpy(curname, nameB, sizeof(curname)); curname[sizeof(curname)-1]=0;
            if (verbose) printf("[V] Unique basename in B: %s\n", curname);
            while (ib < B->size && stricmp_local(Bcopy[ib].basename, curname) == 0) {
                char dest[MAX_PATH];
                const char *src = Bcopy[ib].fullpath;
                const char *p = strrchr(src, '\\');
                const char *fname = p ? p+1 : src;
                join_path(dest, sizeof(dest), folderC, fname);
                if (CopyFileA(src, dest, FALSE)) {
                    (*copiedCount)++;
                    if (verbose) printf("[V] Copied from B: %s -> %s\n", src, dest);
                } else {
                    DWORD e = GetLastError();
                    fprintf(stderr, "WARN: Copy failed %s -> %s (err %lu)\n", src, dest, e);
                }
                ib++;
            }
        }
    }

    free(Acopy);
    free(Bcopy);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("Usage: %s <FolderA> <FolderB> [FolderC]\n", argv[0]);
        return 1;
    }
    const char *folderA = argv[1];
    const char *folderB = argv[2];
    const char *folderC = (argc >= 4) ? argv[3] : "UNMATCHED";

    if (argc >= 5) {
        if (stricmp_local(argv[4], "-q") == 0) verbose = 0;
    }

    if (verbose) {
        printf("[V] A='%s'\n[V] B='%s'\n[V] C='%s'\n", folderA, folderB, folderC);
        printf("[V] Extensions considered:");
        for (size_t i=0;i<ext_count;i++) printf(" %s", exts[i]);
        printf("\n");
    }

    // create target folder if needed
    if (!CreateDirectoryA(folderC, NULL)) {
        DWORD e = GetLastError();
        if (e != ERROR_ALREADY_EXISTS) {
            fprintf(stderr, "ERROR: cannot create target folder '%s' (err %lu)\n", folderC, e);
            return 2;
        }
    }

    DynArray A, B;
    dyn_init(&A); dyn_init(&B);

    scan_folder(folderA, &A);
    scan_folder(folderB, &B);

    if (verbose) {
        printf("[V] Found %zu candidates in A, %zu in B\n", A.size, B.size);
    }

    // copy unique basenames
    int copied = 0;
    copy_only_unique_basenames(&A, &B, folderC, &copied);

    printf("Done. Files copied into '%s': %d\n", folderC, copied);

    dyn_free(&A);
    dyn_free(&B);
    return 0;
}
