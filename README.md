# 0 Keylogger

An advanced keylogger with both kernel and userspace mode support. Implements stealth functionality and netlink socket communication.

## Features

- Kernel mode (stealth)
- Userspace mode with assembly support
- Netlink socket communication
- Optional debug logging
- Automatic keyboard device detection
- Kernel module hiding
- Separate receiver with human-readable output
- Session logging with timestamps

## Requirements

- Linux kernel headers
- NASM assembler
- GCC compiler
- Make

## Installation

1. Clone the repository:
```bash
git clone https://github.com/UsboKirishima/0
cd 0
```

2. Build the project:

To compile everything (recommended):
```bash
make
```

Specific components:
```bash
make kernel      # Kernel module only
make userspace   # Userspace logger only
make receiver    # Receiver only
```

## Usage

### Kernel Mode (Stealth)

1. Load the kernel module (requires root):
```bash
sudo insmod 0.ko
```

2. Start the receiver (can run as normal user):
```bash
./receiver [optional_log_file]
```

3. Or start the keylogger in kernel mode:
```bash
sudo ./0 --kernel
```

4. To remove the module:
```bash
sudo rmmod 0
```

### Userspace Mode

Simply run:
```bash
./0
```

The program will automatically detect the keyboard device.

### Receiver

The receiver provides human-readable output and session logging:

```bash
# Default logging to keylog.txt
./receiver

# Custom log file
./receiver /path/to/logfile.txt
```

Log format example:
```
--- Session started at Wed Mar 13 10:00:00 2024 ---
[2024-03-13 10:00:05] h
[2024-03-13 10:00:05] e
[2024-03-13 10:00:05] l
[2024-03-13 10:00:05] l
[2024-03-13 10:00:05] o
[2024-03-13 10:00:06] [Enter]
--- Session ended at Wed Mar 13 10:00:10 2024 ---
```

## Debug

To enable debug logging, compile with the DEBUG flag:
```bash
make CFLAGS="-DDEBUG"
```

Logs will be saved using slog.

## Project Structure

```
.
├── src/
│   ├── 0.c              # Main file
│   ├── receiver.c       # Human-readable output receiver
│   ├── read_event.asm   # Assembly implementation
│   └── slog.c           # Logger
├── include/
│   ├── 0.h             # Main header
│   └── slog.h          # Logger header
└── Makefile
```

## Security Notes

- Kernel module requires root privileges
- Kernel mode is hidden from loaded modules list
- Data is transmitted locally via netlink socket
- Receiver can run as normal user
- Log files are created with current user permissions

## License

This project is released under GPL v3 license.

## Author

- 333revenge

## Disclaimer

This software was created for educational purposes only. Misuse of this software may constitute a criminal offense in some jurisdictions. The author assumes no liability for misuse of this software.

## Contributing

1. Fork the project
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## Known Issues

- Kernel module might require recompilation when kernel is updated
- Some keyboard layouts might not be detected correctly in userspace mode
- Receiver must be restarted if kernel module is reloaded

## Support

For bug reports or feature requests, please open an issue on GitHub.
