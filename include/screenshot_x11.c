#include "headers/screenshot_x11.h"

static inline unsigned char extract_component(unsigned long pixel,
                                              unsigned long mask) {
  int shift = 0;
  while ((mask & 1) == 0) {
    mask >>= 1;
    pixel >>= 1;
    shift++;
  }
  return (pixel & mask);
}

void write_image(XImage *image, const char *path) {
  if (!image) {
    fprintf(stderr, "Failed to get image from screen.\n");
    exit(1);
  }

  FILE *f = fopen(path, "wb");
  if (!f) {
    perror("fopen");
    exit(1);
  }

  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png) {
    fclose(f);
    fprintf(stderr, "png_create_write_struct failed.\n");
    exit(1);
  }

  png_infop info = png_create_info_struct(png);
  if (!info) {
    png_destroy_write_struct(&png, NULL);
    fclose(f);
    fprintf(stderr, "png_create_info_struct failed.\n");
    exit(EXIT_FAILURE);
  }

  if (setjmp(png_jmpbuf(png))) {
    png_destroy_write_struct(&png, &info);
    fclose(f);
    fprintf(stderr, "Error during PNG creation.\n");
    exit(EXIT_FAILURE);
  }

  png_init_io(png, f);

  png_set_IHDR(png, info, image->width, image->height, 8, PNG_COLOR_TYPE_RGB,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);

  png_write_info(png, info);

  png_bytep row = (png_bytep)malloc(3 * image->width);
  for (int y = 0; y < image->height; ++y) {
    for (int x = 0; x < image->width; ++x) {
      unsigned long pixel = XGetPixel(image, x, y);
      int x3 = x * 3;
      row[x3] = extract_component(pixel, image->red_mask);
      row[x3 + 1] = extract_component(pixel, image->green_mask);
      row[x3 + 2] = extract_component(pixel, image->blue_mask);
    }
    png_write_row(png, row);
  }

  free(row);

  png_write_end(png, NULL);
  png_destroy_write_struct(&png, &info);
  fclose(f);
}

int x11_screenshot(char path[]) {
  Display *display = XOpenDisplay(NULL);
  if (!display) {
    fprintf(stderr, "Cannot open display.\n");
    return EXIT_FAILURE;
  }

  int screen = DefaultScreen(display);
  Window root = RootWindow(display, screen);

  int width = DisplayWidth(display, screen);
  int height = DisplayHeight(display, screen);

  XImage *image =
      XGetImage(display, root, 0, 0, width, height, AllPlanes, ZPixmap);

  write_image(image, path);

  XDestroyImage(image);
  XCloseDisplay(display);

  return EXIT_SUCCESS;
}
