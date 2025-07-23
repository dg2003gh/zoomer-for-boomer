#ifndef SCREENSHOT_WAYLAND_H
#define SCREENSHOT_WAYLAND_H

#ifdef __cplusplus
extern "C" {
#endif

#define _POSIX_C_SOURCE 200112L

#include "wlr-screencopy-unstable-v1-client-protocol.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <png.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <wayland-client-protocol.h>

int wl_screenshot(char path[]);

#ifdef __cplusplus
}
#endif

#endif // SCREENSHOT_WAYLAND_H
