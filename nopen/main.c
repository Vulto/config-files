/*
 * nopen - small file opener based on MIME type (Unix-specific)
 * Uses: libmagic, execvp, isatty, environment variables
 * Not portable to non-POSIX systems
 */

#include "config.h"

#define BUFFER_SIZE 4096  // Fixed size; PATH_MAX can be unreliable in strict modes

static const char *filename;
static bool displayMimeType = false;

[[noreturn]] static void Run(const char *filename, const char *prog) {
    const char *argv[] = { prog, filename, NULL };

    /* Match by both full path and basename */
    const char *base = prog;
    const char *slash = strrchr(prog, '/');
    if (slash && slash[1]) base = slash + 1;

    /* Look up interface (enum) from fileTypes[]; stop at sentinel (application==NULL) */
    bool is_gui = false;
    for (size_t i = 0; fileTypes[i].application != NULL; ++i) {
        const char *app = fileTypes[i].application;
        if (strcmp(app, prog) == 0 || strcmp(app, base) == 0) {
            /* interface is an enum; compare against GUI constant */
            if (fileTypes[i].interface == GUI) {
                is_gui = true;
            }
            break;
        }
    }

    /* GUI: detach when attached to a terminal; TUI or unknown: run foreground */
    if (is_gui && (isatty(STDIN_FILENO) || isatty(STDOUT_FILENO) || isatty(STDERR_FILENO))) {
        pid_t pid = fork();
        if (pid < 0) { perror("fork"); _exit(EXIT_FAILURE); }
        if (pid > 0) { _exit(EXIT_SUCCESS); }  /* parent exits, freeing the shell */

        if (setsid() < 0) { /* best-effort */ }

        pid = fork();
        if (pid < 0) { _exit(EXIT_FAILURE); }
        if (pid > 0) { _exit(EXIT_SUCCESS); }  /* first child exits */

        /* Detach stdio (C23-friendly) */
        (void)freopen("/dev/null", "r", stdin);
        (void)freopen("/dev/null", "w", stdout);
        (void)freopen("/dev/null", "w", stderr);
    }

    execvp(prog, (char *const *)argv);
    perror("execvp");
    _exit(EXIT_FAILURE);
}

[[noreturn]] static void fail(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

const char *readFromStdin(void) {
    char buffer[BUFFER_SIZE];
    size_t total = 0;
    ssize_t bytesRead;

    do {
        bytesRead = read(STDIN_FILENO, buffer + total, BUFFER_SIZE - total - 1);
        if (bytesRead < 0) fail("read");
        total += (size_t)bytesRead;
    } while (bytesRead > 0 && total < BUFFER_SIZE - 1);

    if (total >= BUFFER_SIZE - 1) {
        fprintf(stderr, "Input path too long (max %d bytes)\n", BUFFER_SIZE - 1);
        exit(EXIT_FAILURE);
    }

    buffer[total] = '\0';
    buffer[strcspn(buffer, "\n")] = '\0';

    // Basic security: refuse absolute paths and parent dir traversal from stdin
    if (buffer[0] == '/' || strstr(buffer, "..")) {
        fprintf(stderr, "Refusing potentially unsafe path from stdin: %s\n", buffer);
        exit(EXIT_FAILURE);
    }

    return strdup(buffer);
}

const char *getMimeType(const char *filename, magic_t magic) {
    const char *mime_type = magic_file(magic, filename);
    if (!mime_type) {
        fprintf(stderr, "magic_file failed: %s\n", magic_error(magic));
        magic_close(magic);
        exit(EXIT_FAILURE);
    }
    return mime_type;
}


[[noreturn]] static void printUsage(const char *programName) {
    fprintf(stderr, "Usage: %s [-d] [filename]\n", programName);
    exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
    int opt;
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                displayMimeType = true;
                break;
            default:
                printUsage(argv[0]);
        }
    }

    if (optind < argc) {
        filename = argv[optind];
    } else if (!displayMimeType && isatty(STDIN_FILENO)) {
        printUsage(argv[0]);
    } else {
        filename = readFromStdin();
    }

    magic_t magic = magic_open(MAGIC_MIME_TYPE);
    if (!magic) fail("magic_open");

    if (magic_load(magic, NULL) != 0) {
        fprintf(stderr, "Cannot load magic database: %s\n", magic_error(magic));
        magic_close(magic);
        exit(EXIT_FAILURE);
    }

    if (displayMimeType) {
        const char *mime_type = getMimeType(filename, magic);
        printf("MIME type for %s: %s\n", filename, mime_type);
    } else {
        const char *mime_type = getMimeType(filename, magic);
        const char *application = findProg(mime_type);

        char mime_lower[256];
        strncpy(mime_lower, mime_type, sizeof(mime_lower) - 1);
        mime_lower[sizeof(mime_lower) - 1] = '\0';
        for (char *p = mime_lower; *p; p++)
            *p = (char)tolower((unsigned char)*p);

        if (strcmp(mime_lower, "application/x-executable") == 0 ||
            strcmp(mime_lower, "application/x-pie-executable") == 0) {
            const char *argv[] = {filename, NULL};
            execvp(filename, (char *const *)argv);
            perror("execvp");
            fprintf(stderr, "Could not execute %s\n", filename);
            exit(EXIT_FAILURE);
        } else {
            Run(filename, application);
        }
    }

    magic_close(magic);

    if (optind >= argc && !isatty(STDIN_FILENO)) {
        free((void *)filename);
    }

    return EXIT_SUCCESS;
}
