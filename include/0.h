#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <linux/input.h>
#include <sys/ioctl.h>
#include <stddef.h>

char* find_keyboard_device(void);
extern ssize_t read_event(int fd, void *buffer, size_t size);

#endif

