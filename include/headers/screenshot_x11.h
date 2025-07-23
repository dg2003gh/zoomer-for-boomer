#ifndef SCREENSHOT_X11_H
#define SCREENSHOT_X11_H

#ifdef __cplusplus
extern "C" {
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>

int x11_screenshot(char path[]);

#ifdef __cplusplus
}
#endif

#endif // SCREENSHOT_X11_H
