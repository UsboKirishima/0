/**
 * @file config.h
 * @brief Configuration file for 0 Keylogger
 */

#ifndef CONFIG_H
#define CONFIG_H

// Network configuration
#define DEFAULT_HOST "127.0.0.1"  // Default host for UDP communication
#define DEFAULT_PORT 8888         // Default port for UDP communication

// Buffer sizes
#define MAX_PAYLOAD 1024         // Maximum payload size for network packets
#define MAX_PATH 256            // Maximum path length
#define MAX_HOSTNAME 64         // Maximum hostname length

// Netlink configuration
#define NETLINK_USER 31         // Netlink protocol number

// Debug configuration
#ifdef DEBUG
    #define DEBUG_LOG(fmt, ...) fprintf(stderr, "[DEBUG] " fmt "\n", ##__VA_ARGS__)
#else
    #define DEBUG_LOG(fmt, ...) ((void)0)
#endif

// Version information
#define VERSION_MAJOR 1
#define VERSION_MINOR 0
#define VERSION_PATCH 0

// Feature flags
#define ENABLE_STEALTH 1        // Enable stealth mode in kernel
#define ENABLE_LOGGING 1        // Enable file logging
#define ENABLE_UDP 1           // Enable UDP communication

typedef struct {
    char host[MAX_HOSTNAME];    // UDP host address
    int port;                   // UDP port number
    int stealth_mode;          // Stealth mode enabled flag
    int logging_enabled;       // Logging enabled flag
    char log_file[MAX_PATH];   // Log file path
    char log_format[256];      // Log entry format
    char timestamp_format[64]; // Timestamp format
    int debug_enabled;         // Debug mode flag
    int debug_verbose;         // Verbose debug output flag
} config_t;

int load_config(const char *filename);
const config_t* get_config(void);

#endif // CONFIG_H 