#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <cjson/cJSON.h>
#include "src/get_dir.h"
#include "src/packet_serial.h"
#include <libserialport.h>
#include <curl/curl.h>

#ifdef _WIN32
    #include <windows.h>
    #define cross_sleep(s) Sleep((s) * 1000)
#else
    #include <unistd.h>
    #define cross_sleep(s) sleep(s)
#endif


char dir_path[256];
int dir_path_len = 0;

struct sp_port **ports;
static struct sp_port *active_port = NULL;

reliable_packeter_t pkt;

/* Set by on_packet_received() callback; read + cleared in the main loop */
static volatile bool     rx_data_ready = false;
static volatile size_t   rx_data_len   = 0;
static uint8_t           rx_buf[4096];

/* Called automatically by packeter_update() when a full, CRC-verified
 * packet is assembled. data points into rx_buf. */
static void on_packet_received(const uint8_t *data, size_t len) {
    /* data already lives in rx_buf — just null-terminate for printf */
    if (len < sizeof(rx_buf)) {
        rx_buf[len] = '\0';
    }
    rx_data_len   = len;
    rx_data_ready = true;
}

enum {
    STATE_CONNECTION_SYNC,
    STATE_GET_IP,
    STATE_ENTER_BOOT_MODE,
    STATE_HALT
};

/* ============================================================================
 * libserialport glue callbacks for packet_serial
 * ============================================================================ */
static void sp_tx(const uint8_t *data, size_t len) {
    if (active_port)
        sp_blocking_write(active_port, data, len, 200);
}

static int sp_available(void) {
    if (!active_port) return 0;
    int n = sp_input_waiting(active_port);
    return (n > 0) ? n : 0;
}

static uint8_t sp_read_byte(void) {
    uint8_t b = 0;
    if (active_port)
        sp_blocking_read(active_port, &b, 1, 50);
    return b;
}

static uint32_t sp_millis(void) {
#ifdef _WIN32
    return (uint32_t)GetTickCount64();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
#endif
}

void set_dir();
void print_release_notes();
void flash(char flash_mode);
void update();
int get_usb_ports();
bool ota_esp(char *ip);
bool flash_esp();
bool flash_stm_dfu();


int main(int argc, char *argv[]){

    char mode = 0;

    // for(int i=0; i< argc; i++)
    //     printf("argc[%d] = %s\n",i,argv[i]);

    if(argc == 1){
        printf("Missing arguments\n");
        printf("Use --help for more information\n");
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1],"--help")==0){
        printf("Usage: axlbin [options]\n");
        printf("Options:\n");
        printf("  --esp       Flash ESP\n");
        printf("  --stm       Flash STM\n");
        printf("  --help      Show help\n");
        exit(EXIT_SUCCESS);
    }

    set_dir();
    print_release_notes();
    if(argc > 2){
        if(strcmp(argv[1],"flash") == 0){
            mode = 1;
        }
        else if(strcmp(argv[1],"update") == 0){
            mode = 2;
        }
        else {
            printf("Invalid argument\n");
            exit(EXIT_FAILURE);
        }

        

        if(mode==1){
            if(argc==3){       
                if(strcmp(argv[2],"--esp")==0){
                    flash(1);
                }
                else if(strcmp(argv[2],"--stm")==0){
                    flash(2);
                }
                else if(strcmp(argv[2],"--multi")==0){
                    while(1){
                        flash(0);
                        printf("\n\nDo you want to flash another device [Y/N]: ");
                        char in;
                        scanf(" %c",&in);
                        if(in=='Y' || in=='y'){
                            continue;
                        }
                        else{
                            break;
                        }
                    }
                }
                else{
                    printf("Invalid argument\n");
                    exit(EXIT_FAILURE);
                }
            }
        }
        
        else if(mode == 2){
            if(argc == 3){
                if(strcmp(argv[2],"--multi")==0){
                    while(1){
                        update();
                        printf("\n\nDo you want to update another device [Y/N]: ");
                        char in;
                        scanf(" %c",&in);
                        if(in=='Y' || in=='y'){
                            continue;
                        }
                        else{
                            break;
                        }
                    }
                }
            }
        }
    }
    else if(argc == 2){
        flash(0);
        return 0;
    }
    else{
        printf("Invalid argument\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}

void set_dir(){
    while(1){
        get_firmware_dir(dir_path,&dir_path_len);
        if(dir_path_len!=0){
            printf("Path = %s\n",dir_path);
            break;
        }
        else{
            printf("\nInavlid path, Press enter Y to try again or N to exit: ");
            char in;
            scanf(" %c",&in);
            if(in=='Y'){
                continue;
            }
            else{
                break;
            }
        }
    }
}

void print_release_notes(){
    FILE *fptr;
    char buff[512];
    char file_path[512];
    sprintf(file_path,"%s/changelog.txt",dir_path);
    fptr = fopen(file_path,"r");
    if(fptr==NULL){
        printf("changelog.txt not found\n");
        return;
    }
    printf("\n\n");
    while(fgets(buff,sizeof(buff),fptr)!=NULL){
        printf("%s",buff);
    }
    printf("\n\n");
    fclose(fptr);
}

int get_usb_ports(){
    int count = 0;
    int err = sp_list_ports(&ports);
    if(err!=SP_OK){
        printf("No USB ports found\n");
        return 0;
    }
    for (int i = 0; ports[i] != NULL; i++) {
        printf("[%d] %s - %s\n", i, sp_get_port_name(ports[i]),sp_get_port_description(ports[i]));
        count = i;
    }
    // sp_free_port_list(ports);
    return count+1;
}

void flash(char flash_mode){
    if(flash_mode==1){
            flash_esp();
        }
    else if(flash_mode==2){
            flash_stm_dfu();
        }
    else {
        flash_esp();
        flash_stm_dfu();
    }
}

void send_cmds(char *msg){
    packet_status_t status = packeter_send(&pkt,
                                                (const uint8_t *)msg,
                                                strlen(msg));
    if(status == PACKET_OK){
        printf("Packet sent OK\n");
    } else if(status == PACKET_TIMEOUT){
        printf("Packet TIMEOUT (no ACK)\n");
    } else {
        printf("Packet error: %d\n", status);
    }
}

void update(){
    int port_count = get_usb_ports();
    int usb_choice = 0;
    char ip_address[16];
    
    if(port_count!=0){
        printf("Enter the port number of device: ");
        scanf("%d",&usb_choice);
        printf("\n\n");
        if(usb_choice < 0 || usb_choice >= port_count){
            printf("Invalid choice\n");
            return;
        }
    }

    active_port = ports[usb_choice];
    sp_open(active_port, SP_MODE_READ_WRITE);
    sp_set_baudrate(active_port, 9600);
    sp_set_bits(active_port, 8);
    sp_set_parity(active_port, SP_PARITY_NONE);
    sp_set_stopbits(active_port, 1);
    sp_set_flowcontrol(active_port, SP_FLOWCONTROL_NONE);


    packeter_init(&pkt, 500);
    packeter_set_callbacks(&pkt, sp_tx, sp_available, sp_read_byte, sp_millis);
    packeter_set_receive_buffer(&pkt, rx_buf, sizeof(rx_buf) - 1);
    packeter_on_receive(&pkt, on_packet_received);

    printf("Starting packet send loop..\n");

    int current_state = STATE_CONNECTION_SYNC;
    while(1){
        switch(current_state){
            case STATE_CONNECTION_SYNC:
                send_cmds("{\"type\":\"SYS\",\"cmd\":\"connection_sync\"}");
                break;
            case STATE_GET_IP:
                send_cmds("{\"type\":\"SYS\",\"cmd\":\"ip_address\"}");
                break;
            case STATE_ENTER_BOOT_MODE:
                send_cmds("{\"type\":\"SYS\",\"cmd\":\"enter_boot\"}");
                break;
            case STATE_HALT:

            default:
                break;
        }
        

        while(1){
            packeter_update(&pkt);
            if(!rx_data_ready)
                continue;
            rx_data_ready = false;
            
            cJSON *root = cJSON_Parse((char *)rx_buf);
            printf("%s\n",rx_buf);
            if (root == NULL) {
                printf("Error parsing JSON.\n");
                return;
            }
            cJSON *cmd = cJSON_GetObjectItemCaseSensitive(root, "cmd");
             if (cJSON_IsString(cmd) && (cmd->valuestring != NULL)) {
                // printf("cmd = %s\n",cmd->valuestring);
                if(strcmp(cmd->valuestring, "ack_connection_sync") == 0){
                    current_state = STATE_GET_IP;
                    break;
                }
                else if(strcmp(cmd->valuestring, "ack_ip_address") == 0){
                    cJSON *ip = cJSON_GetObjectItemCaseSensitive(root, "ip_address");
                    if (cJSON_IsString(ip) && (ip->valuestring != NULL)) {
                        printf("ip = %s\n",ip->valuestring);
                        sprintf(ip_address,"%s",ip->valuestring);
                        if(strcmp(ip_address,"0.0.0.0")==0){
                            printf("No IP Address assigned\nIs the Network LED Off??\nDo you want to try again? [Y/N]: Or Enter Custom WiFi Credentials [C]: ");
                            char choice;
                            scanf(" %c",&choice);
                            if(choice == 'Y' || choice == 'y'){
                                printf("Retrying......\n");
                                current_state = STATE_GET_IP;
                                break;
                            }
                            else if(choice == 'C' || choice == 'c'){
                                char ssid[50];
                                char password[50];
                                printf("Enter WiFi SSId: ");
                                scanf("%s",ssid);
                                printf("Enter WiFi Password: ");
                                scanf("%s",password);
                                char send_buf[256];
                                sprintf(send_buf,"{\"type\":\"RFID\",\"cmd\":\"config\",\"config_data\":{\"network_settings\":{\"lan\":true,\"wifi\":{\"ssid\":\"%s\",\"security\":\"\",\"status\":true,\"password\":\"%s\"}}}}",ssid,password);
                                send_cmds(send_buf);
                                printf("Trying connecting to WiFi.........");
                                cross_sleep(10);
                                current_state = STATE_GET_IP;
                                break;
                            }
                            else{
                                exit(0);
                            }
                        }
                        else{
                            printf("Trying connecting to server.........\n");
                            char url[64];
                            sprintf(url, "http://%s/api/config", ip_address);
                            printf("Requesting %s\n", url);

                            CURL *curl = curl_easy_init();
                            long status_code = 0;
                            if (curl) {
                                // discard response body — we only need the status code
                                curl_easy_setopt(curl, CURLOPT_URL, url);
                                curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
                                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                                    (curl_write_callback)fwrite); // sink to /dev/null below
                                FILE *devnull = fopen(
                                    #ifdef _WIN32
                                    "NUL"
                                    #else
                                    "/dev/null"
                                    #endif
                                    , "wb");
                                curl_easy_setopt(curl, CURLOPT_WRITEDATA, devnull);
                                CURLcode res = curl_easy_perform(curl);
                                if (res == CURLE_OK)
                                    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
                                curl_easy_cleanup(curl);
                                if (devnull) fclose(devnull);
                            }

                            printf("HTTP Status Code: %ld\n\n", status_code);
                            if (status_code == 200) {
                                printf("Successfully connected to the server\n");
                                current_state = STATE_ENTER_BOOT_MODE;
                                break;
                            }
                            else{
                                printf("Failed to connect to the server\nIs this PC and the device in same network??\nDo you want to try again? [Y/N]: ");
                                char choice;
                                scanf(" %c",&choice);
                                if(choice == 'Y' || choice == 'y'){
                                    printf("Retrying......\n");
                                    current_state = STATE_GET_IP;
                                    break;
                                }
                                else{
                                    exit(0);
                                }
                            }
                            
                        }
                    }
                }
                else if(strcmp(cmd->valuestring, "ack_enter_boot") == 0){
                    current_state = STATE_HALT;
                    break;
                }
            }
            
        }
        packeter_update(&pkt);
        if(current_state == STATE_HALT){
            break;
        }
    }

    sp_close(active_port);

    cross_sleep(2);
    printf("Flashing STM32....\n");
    while(!flash_stm_dfu()){
        printf("Flashing STM32 Failed. Do you want to retry? [Y/N]: ");
        char choice;
        scanf(" %c",&choice);
        if(choice == 'Y' || choice == 'y'){
            printf("Retrying.....\n");
            continue;
        }
        else{
            exit(0);
        }
    }
    cross_sleep(2);
    while(!ota_esp(ip_address)){
        printf("OTA Failed. Do you want to retry? [Y/N]: ");
        char choice;
        scanf(" %c",&choice);
        if(choice == 'Y' || choice == 'y'){
            printf("Retrying.....\n");
            continue;
        }
        else{
            exit(0);
        }
    }
    
}

static int ota_progress_cb(void *clientp, curl_off_t dltotal, curl_off_t dlnow,
                            curl_off_t ultotal, curl_off_t ulnow)
{
    (void)clientp; (void)dltotal; (void)dlnow;
    if (ultotal <= 0) return 0;

    int percent = (int)(ulnow * 100 / ultotal);
    int bar_width = 40;
    int filled = bar_width * percent / 100;

    printf("\r  [");
    for (int i = 0; i < bar_width; i++)
        printf(i < filled ? "=" : (i == filled ? ">" : " "));
    printf("] %3d%%  (%ld / %ld KB)", percent,
           (long)(ulnow / 1024), (long)(ultotal / 1024));
    fflush(stdout);
    return 0;
}

bool ota_esp(char *ip) {
    char url[64];
    char file_path[512];
    FILE *file;

    sprintf(file_path, "%s/BIN-NW-Manager.ino.bin", dir_path);
    file = fopen(file_path, "rb");
    if (file == NULL) {
        printf("File not found\n");
        return false;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *file_buffer = malloc(file_size);
    if (!file_buffer) {
        printf("malloc failed\n");
        fclose(file);
        return false;
    }
    fread(file_buffer, 1, file_size, file);
    fclose(file);

    sprintf(url, "http://%s/update", ip);
    printf("Uploading firmware to: %s  (%ld bytes)\n", url, file_size);

    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("Failed to init curl\n");
        free(file_buffer);
        return false;
    }

    curl_mime *form = curl_mime_init(curl);
    curl_mimepart *field = curl_mime_addpart(form);
    curl_mime_name(field, "update");
    curl_mime_filename(field, "firmware.bin");
    curl_mime_type(field, "application/octet-stream");
    curl_mime_data(field, file_buffer, file_size);


    struct curl_slist *header_list = NULL;
    header_list = curl_slist_append(header_list, "X-Password: axl@#$admin");
    header_list = curl_slist_append(header_list, "Expect:");

    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
    curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, ota_progress_cb);

    CURLcode res = curl_easy_perform(curl);

    free(file_buffer);
    curl_mime_free(form);
    curl_slist_free_all(header_list);
    printf("\n");

    if (res != CURLE_OK && res != CURLE_RECV_ERROR) {
        printf("Upload failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }

    long status_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status_code);
    curl_easy_cleanup(curl);

    printf("HTTP Status Code: %ld\n", status_code);

    if (status_code == 200) {
        printf("Firmware uploaded successfully! Device is rebooting...\n");
        return true;
    } else {
        printf("Upload failed (server returned %ld)\n", status_code);
        return false;
    }
}


bool flash_esp(){
    char bootloader_path[512];
    char partitions_path[512];
    char app_path[512];
    char boot_app0_path[512];

    int port_count = get_usb_ports();
    int usb_choice = 0;

    while(1){
        if(port_count!=0){
            printf("Enter the port number of esp programmer: ");
            scanf("%d",&usb_choice);
            printf("\n\n");
            if(usb_choice < 0 || usb_choice >=port_count){
                printf("Invalid choice\n");
                return false;
            }
        }

        printf("Starting ESP programming..\n");

        sprintf(bootloader_path,"%s/esp_partition/BIN-NW-Manager.ino.bootloader.bin",dir_path);
        sprintf(partitions_path,"%s/esp_partition/BIN-NW-Manager.ino.partitions.bin",dir_path);
        sprintf(app_path,"%s/BIN-NW-Manager.ino.bin",dir_path);
        sprintf(boot_app0_path,"%s/esp_partition/boot_app0.bin",dir_path);

        char esp_cmd[4096];
        sprintf(esp_cmd, "esptool --chip esp32 --port %s --baud 921600 write-flash 0x1000 %s 0x8000 %s 0xe000 %s 0x10000 %s",
        sp_get_port_name(ports[usb_choice]),
        bootloader_path,
        partitions_path,
        boot_app0_path,
        app_path
        );
        // printf("Command: %s\n",esp_cmd);
        int ret = system(esp_cmd);
        if(ret!=0){
            printf("Failed to flash the firmware\n");
            printf("Do you want to try again [Y/N]:");
            char c;
            scanf(" %c",&c);
            if(c=='Y' || c=='y'){
                continue;
            }
            else{
                exit(EXIT_FAILURE);
            }
        }
        else{
            break;
        }
    }
    return true;
}

bool flash_stm_dfu(){
    char file_path[512];
    sprintf(file_path, "%s/axlbin.bin", dir_path);

    char stm_cmd[2048];
    sprintf(stm_cmd, "STM32_Programmer_CLI -c port=USB1 -w %s 0x08000000 -v",file_path);

    while(1){

    int ret = system(stm_cmd);
    if(ret!=0){
        printf("Failed to flash the firmware\n");
        printf("Do you want to try again [Y/N]:");
        char c;
        scanf(" %c",&c);
        if(c=='Y' || c=='y'){
            continue;
        }
        else{
            exit(EXIT_FAILURE);
        }
    }
    else{
        break;
    }
}

    return true;
}