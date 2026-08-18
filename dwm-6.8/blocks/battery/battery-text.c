#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include<string.h>

#define CAPACITY_DIR "/sys/class/power_supply/BAT0/capacity"

int main() {
    FILE* capacity_ptr;
    int capacity;

    capacity_ptr = fopen(CAPACITY_DIR, "r");

    if(capacity_ptr == NULL) {
        printf("Unable to read file 'status'\n");
        return(EXIT_FAILURE);
    }

    fscanf(capacity_ptr, "%3d", &capacity);
    fclose(capacity_ptr);

    printf( "battery: %d%%", capacity );
    exit(EXIT_SUCCESS);
}
