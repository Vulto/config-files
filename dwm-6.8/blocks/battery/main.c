#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define CONFIG_BLOCK_BATTERY
#include "../../config.h"

int main(int argc, char *argv[])
{
    int showPercentage = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-t") == 0) {
            showPercentage = 1;
        } else if (strcmp(argv[i], "-h") == 0) {
            puts(
                "Usage: battery_monitor [options]\n"
                "Options:\n"
                "  -t       Show battery percentage\n"
                "  -h       Show this help message"
            );
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            return EXIT_FAILURE;
        }
    }

    FILE *capacityFile = fopen(CapacityPath, "r");
    if (!capacityFile) {
        perror("Unable to read battery capacity");
        return EXIT_FAILURE;
    }

    int capacity;

    if (fscanf(capacityFile, "%d", &capacity) != 1) {
        fclose(capacityFile);
        fprintf(stderr, "Unable to parse battery capacity\n");
        return EXIT_FAILURE;
    }

    fclose(capacityFile);

    FILE *statusFile = fopen(StatusPath, "r");
    if (!statusFile) {
        perror("Unable to read battery status");
        return EXIT_FAILURE;
    }

    char status[16];

    if (!fgets(status, sizeof(status), statusFile)) {
        fclose(statusFile);
        fprintf(stderr, "Unable to read battery status\n");
        return EXIT_FAILURE;
    }

    fclose(statusFile);

    status[strcspn(status, "\r\n")] = '\0';

    char *blockButton = getenv("BLOCK_BUTTON");

    if (blockButton && atoi(blockButton) == 1)
        system(BAT_CLICK);

    if (showPercentage) {
        printf("%d%%\n", capacity);
        return EXIT_SUCCESS;
    }

    if (strcmp(status, "Full") == 0) {
        puts(BAT_FULL);
    } else if (
        strcmp(status, "Charging") == 0 ||
        strcmp(status, "Not charging") == 0
    ) {
        puts(BAT_CHARGE);
    } else if (capacity >= BAT_LEVEL[3]) {
        puts(BAT_ICON[4]);
    } else if (capacity >= BAT_LEVEL[2]) {
        puts(BAT_ICON[3]);
    } else if (capacity >= BAT_LEVEL[1]) {
        puts(BAT_ICON[2]);
    } else if (capacity >= BAT_LEVEL[0]) {
        puts(BAT_ICON[1]);
    } else {
        puts(BAT_ICON[0]);
        system(BAT_CRITICAL);
    }

    return EXIT_SUCCESS;
}
