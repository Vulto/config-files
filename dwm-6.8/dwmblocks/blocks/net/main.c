// main.c — Linux-only, simple sysfs-based net indicator (C23)
// Mirrors the POSIX shell script logic: read /sys/class/net only.

// --- icons (edit to taste) ---
static const char *WIFI_UP   = " ";
static const char *WIFI_DOWN = "";
static const char *ETH_UP    = "  ";
static const char *ETH_DOWN  = "";
// -----------------------------

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

static int path_is_dir(const char *path) {
    struct stat st;
    return (stat(path, &st) == 0) && S_ISDIR(st.st_mode);
}

static int read_file_trim(const char *path, char *buf, size_t bufsz) {
    FILE *f = fopen(path, "r");
    if (!f) return 0;
    size_t n = fread(buf, 1, bufsz - 1, f);
    fclose(f);
    if (n == 0) { buf[0] = '\0'; return 1; }
    buf[n] = '\0';
    // trim trailing newline/spaces
    while (n > 0 && (buf[n-1] == '\n' || buf[n-1] == '\r' || buf[n-1] == ' ' || buf[n-1] == '\t')) {
        buf[--n] = '\0';
    }
    return 1;
}

static int is_up(const char *ifname) {
    char path[256], s[32];
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", ifname);
    if (!read_file_trim(path, s, sizeof(s))) return 0;
    return strcmp(s, "up") == 0;
}

static int has_carrier(const char *ifname) {
    char path[256], s[32];
    snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", ifname);
    if (!read_file_trim(path, s, sizeof(s))) return 0;
    return strcmp(s, "1") == 0;
}

static int is_wireless(const char *ifname) {
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", ifname);
    return path_is_dir(path);
}

static int name_is_ethernet(const char *ifname) {
    return (strncmp(ifname, "en", 2) == 0) || (strncmp(ifname, "eth", 3) == 0);
}

int main(void) {
    int wifi_connected = 0;
    int eth_connected  = 0;

    DIR *root = opendir("/sys/class/net");
    if (!root) {
        // If sysfs is missing, print down icons and exit gracefully.
        fputs(WIFI_DOWN, stdout);
        fputs(ETH_DOWN, stdout);
        return 0;
    }

    // First pass: Wi-Fi
    {
        struct dirent *de;
        while ((de = readdir(root)) != NULL) {
            const char *ifname = de->d_name;
            if (ifname[0] == '.') continue; // skip . and ..
            if (!is_wireless(ifname)) continue;
            if (is_up(ifname)) { wifi_connected = 1; break; }
        }
        rewinddir(root);
    }

    // Second pass: Ethernet
    {
        struct dirent *de;
        while ((de = readdir(root)) != NULL) {
            const char *ifname = de->d_name;
            if (ifname[0] == '.') continue;
            if (strcmp(ifname, "lo") == 0) continue;
            if (!name_is_ethernet(ifname)) continue;
            if (has_carrier(ifname) && is_up(ifname)) { eth_connected = 1; break; }
        }
    }

    closedir(root);

    // Output (no newline, matching your script behavior)
    fputs(wifi_connected ? WIFI_UP : WIFI_DOWN, stdout);
    fputs(eth_connected  ? ETH_UP   : ETH_DOWN,  stdout);

    return 0;
}

