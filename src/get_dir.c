#include "get_dir.h"
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

static bool is_dir_valid(char *path);

void get_firmware_dir(char *path, int *len){
    printf("Enter the firmware directory path: ");
    scanf("%s",path);
    *len = strlen(path);
    printf("Scanning directory %s\n",path);
    if(is_dir_valid(path)){
        printf("All files found in the directory\n");
    }else{
        printf("Some files not found in the directory\n");
        *len = 0;
    }
}

bool is_dir_valid(char *path){
    char buff[256];
    struct stat buffer;
    sprintf(buff,"%s/axlbin.bin",path);
    if(stat(buff, &buffer)==-1){
        printf("axlbin.bin not found in the directory\n");
        return false;
    }else{
        printf("axlbin.bin found\n");
    }
    sprintf(buff,"%s/BIN-NW-Manager.ino.bin",path);
    if(stat(buff, &buffer)==-1){
        printf("BIN-NW-Manager.ino.bin not found in the directory\n");
        return false;
    }else{
        printf("BIN-NW-Manager.ino.bin found\n");
    }
    sprintf(buff,"%s/esp_partition/BIN-NW-Manager.ino.bootloader.bin",path);
    if(stat(buff,&buffer)==-1){
        printf("BIN-NW-Manager.ino.bootloader.bin not found in the directory\n");
        return false;
    }else{
        printf("BIN-NW-Manager.ino.bootloader.bin found\n");
    }
    sprintf(buff,"%s/esp_partition/BIN-NW-Manager.ino.partitions.bin",path);
    if(stat(buff,&buffer)==-1){
        printf("BIN-NW-Manager.ino.partitions.bin not found in the directory\n");
        return false;
    }else{
        printf("BIN-NW-Manager.ino.partitions.bin found\n");
    }
    sprintf(buff,"%s/esp_partition/boot_app0.bin",path);
    if(stat(buff,&buffer)==-1){
        printf("boot_app0.bin not found in the directory\n");
        return false;
    }else{
        printf("boot_app0.bin found\n");
    }
    return true;
}
