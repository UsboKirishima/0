#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jansson.h>
#include <config.h>



static config_t current_config;

/**
 * Loads configuration from JSON file
 * @param filename Path to config file
 * @return 0 on success, -1 on error
 */
int load_config(const char *filename) {
    json_error_t error;
    json_t *root = json_load_file(filename, 0, &error);
    
    if (!root) {
        fprintf(stderr, "Error loading config file: %s\n", error.text);
        return -1;
    }

    json_t *network = json_object_get(root, "network");
    if (network) {
        json_t *host = json_object_get(network, "host");
        json_t *port = json_object_get(network, "port");

        if (json_is_string(host)) {
            strncpy(current_config.host, json_string_value(host), 255);
        }
        if (json_is_integer(port)) {
            current_config.port = json_integer_value(port);
        }
    }

    json_decref(root);
    return 0;
}

/**
 * Returns pointer to current configuration
 * @return Pointer to config_t struct
 */
const config_t* get_config(void) {
    return &current_config;
}