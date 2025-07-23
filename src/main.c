#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/headers/screenshot_wayland.h"

#define Font X11Font
#include "../include/headers/screenshot_x11.h"
#undef Font

#define TRANSPARENT (Color){0, 0, 0, 0}
#define DARKOVERLAY (Color){0, 0, 0, 200}

#define TERM_RESET "\033[0m"
#define TERM_BOLD_YELLOW "\033[1;33m"
#define TERM_CYAN "\033[0;36m"
#define TERM_BLUE "\033[0;34m"

#define MAX_POINTS 4096

typedef struct {
  Vector2 pos;
  int life;
} FadingPoint;

void raylibInit(int isScreenshot, char image_path[], float factor,
                unsigned int flags);
void inputHandler(Vector2 *position, float *zoom, float *factor);
static void help();

int main(int argc, char *argv[]) {
  char option;
  float factor = 0.6f;
  int colors = 1;
  char *imagePath = "/tmp/fzb.tmp.png";

  while ((option = getopt(argc, argv, "hcs:p:")) != -1) {
    switch (option) {
    case 'h':
      help();
      break;
    case 's':
      factor = atof(optarg);
      break;
    case 'c':
      colors = 0;
      break;
    case 'p':
      imagePath = optarg;
      break;
    default:
      help();
      break;
    }
  }

  const char *session_type = getenv("XDG_SESSION_TYPE");
  const char *wayland_display = getenv("WAYLAND_DISPLAY");
  const char *x11_display = getenv("DISPLAY");

  int isScreenshot = 1;
  unsigned int flags = FLAG_BORDERLESS_WINDOWED_MODE | FLAG_WINDOW_TOPMOST |
                       FLAG_WINDOW_UNDECORATED | FLAG_WINDOW_TRANSPARENT;

  if (session_type) {
    if (strcmp(session_type, "wayland") == 0 || wayland_display) {
      printf("You're running on Wayland...\n");
      // just stole it from sway ngl
      isScreenshot = wl_screenshot(imagePath);
      flags |= FLAG_FULLSCREEN_MODE;
    } else if (strcmp(session_type, "x11") == 0 || x11_display) {
      printf("You're running on X11...\n");
      x11_screenshot(imagePath);
    }
  }

  raylibInit(isScreenshot, imagePath, factor, flags);

  return EXIT_SUCCESS;
}

void raylibInit(int isScreenshot, char imagePath[], float factor,
                unsigned int flags) {

  SetConfigFlags(flags);

  InitWindow(GetMonitorWidth(0), GetMonitorHeight(0), "Zoomer for boomer");
  SetTargetFPS(60);

  if (isScreenshot == EXIT_FAILURE)
    exit(EXIT_FAILURE);

  Texture2D image = LoadTexture(imagePath);

  float zoom = 1.0f;
  Vector2 position = {0, 0};

  Vector2 mouseInit = GetMousePosition();
  FadingPoint mouseDraw[MAX_POINTS];
  int mouseDrawCount = 0;
  int dragging = 0;

  HideCursor();

  while (!WindowShouldClose()) {
    float oldZoom = zoom;
    Vector2 mouseNow = GetMousePosition();
    float wheel = GetMouseWheelMove();

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
      dragging = 1;
      mouseInit = mouseNow;

      if (!IsKeyDown(KEY_LEFT_CONTROL) && mouseDrawCount < MAX_POINTS)
        mouseDraw[mouseDrawCount++] = (FadingPoint){{-1, -1}, 0}; // trail break
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
      dragging = 0;
    }

    if (!IsKeyDown(KEY_LEFT_CONTROL) && dragging &&
        mouseDrawCount < MAX_POINTS) {
      mouseDraw[mouseDrawCount++] = (FadingPoint){mouseNow, 255};
    }

    if (wheel != 0.0f) {
      zoom += wheel * factor;
      if (zoom < 0.1f)
        zoom = 0.1f;

      Vector2 mouse = mouseNow;
      position.x = mouse.x - (mouse.x - position.x) * (zoom / oldZoom);
      position.y = mouse.y - (mouse.y - position.y) * (zoom / oldZoom);
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && dragging) {
      Vector2 delta = Vector2Subtract(mouseNow, mouseInit);
      position = Vector2Add(position, delta);
      mouseInit = mouseNow;
    }

    inputHandler(&position, &zoom, &factor);

    // Fade logic
    for (int point = 0; point < mouseDrawCount; point++) {
      if (mouseDraw[point].life > 0) {
        mouseDraw[point].life -= 3;
        if (mouseDraw[point].life < 0)
          mouseDraw[point].life = 0;
      }
    }

    // Clear old points
    int newCount = 0;
    for (int point = 0; point < mouseDrawCount; point++) {
      if (mouseDraw[point].life > 0 ||
          (mouseDraw[point].pos.x == -1 && mouseDraw[point].pos.y == -1)) {
        mouseDraw[newCount++] = mouseDraw[point];
      }
    }
    mouseDrawCount = newCount;

    BeginDrawing();
    ClearBackground(TRANSPARENT);

    // Screenshot
    DrawTextureEx(image, position, 0, zoom, WHITE);

    // Focus mode
    if (IsKeyDown(KEY_F)) {
      RenderTexture2D mask = LoadRenderTexture(image.width, image.height);
      BeginTextureMode(mask);

      ClearBackground(DARKOVERLAY);

      DrawCircleV(mouseNow, 100, WHITE);
      EndTextureMode();

      BeginBlendMode(BLEND_MULTIPLIED);
      DrawTextureRec(mask.texture,
                     (Rectangle){0, 0, image.width, -image.height},
                     (Vector2){0, 0}, WHITE);
      EndBlendMode();
    }

    // Mouse indicator
    DrawCircle(mouseNow.x, mouseNow.y, 5.0f, RED);

    // Draw lines
    for (int point = 1; point < mouseDrawCount; point++) {
      if (mouseDraw[point - 1].pos.x == -1 && mouseDraw[point - 1].pos.y == -1)
        continue;
      if (mouseDraw[point].pos.x == -1 && mouseDraw[point].pos.y == -1)
        continue;

      Color fadeColor = RED;
      fadeColor.a = mouseDraw[point].life;
      DrawLineEx(mouseDraw[point - 1].pos, mouseDraw[point].pos, 5.0f,
                 fadeColor);
    }

    EndDrawing();
  }

  ShowCursor();
  UnloadTexture(image);
  CloseWindow();
}

void inputHandler(Vector2 *position, float *zoom, float *factor) {
  int key;

  while ((key = GetCharPressed()) != 0) {
    switch (key) {
    case KEY_ZERO:
      *position = (Vector2){0, 0};
      *zoom = 1.0f;
      break;
    case KEY_MINUS:
      *zoom -= *factor;
      break;
    case KEY_EQUAL:
      *zoom += *factor;
      break;
    }
  }

  if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_L))
    position->x -= *factor * 2;
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_H))
    position->x += *factor * 2;
  if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_K))
    position->y += *factor * 2;
  if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J))
    position->y -= *factor * 2;
}

static void help() {
  const char *msg = TERM_BOLD_YELLOW
      "USAGE:" TERM_RESET " zfb [OPTIONS] [FILE_PATH]\n\n"

      TERM_BOLD_YELLOW "DESCRIPTION:" TERM_RESET "\n" TERM_BLUE
      "  A zoom tool for your desktop." TERM_RESET "\n\n"

      TERM_BOLD_YELLOW "OPTIONS:" TERM_RESET "\n"
      "  " TERM_CYAN "h" TERM_RESET "                 " TERM_BLUE
      "shows this message." TERM_RESET "\n"
      "  " TERM_CYAN "s NUMBER" TERM_RESET "          " TERM_BLUE
      "zoom speed [DEFAULT 0.6]." TERM_RESET "\n"
      "  " TERM_CYAN "p PATH" TERM_RESET "            " TERM_BLUE
      "save path [DEFAULT /tmp/fzb.tmp.png]." TERM_RESET "\n\n"

      TERM_BOLD_YELLOW "KEYBIND:" TERM_RESET "\n"
      "  " TERM_CYAN "ESC" TERM_RESET "               " TERM_BLUE
      "exit program." TERM_RESET "\n"
      "  " TERM_CYAN "0" TERM_RESET "                 " TERM_BLUE
      "reset zoom." TERM_RESET "\n"
      "  " TERM_CYAN "+" TERM_RESET "                 " TERM_BLUE
      "zoom in." TERM_RESET "\n"
      "  " TERM_CYAN "-" TERM_RESET "                 " TERM_BLUE
      "zoom out." TERM_RESET "\n"
      "  " TERM_CYAN "MOUSE WHEEL" TERM_RESET "       " TERM_BLUE
      "zoom in/out." TERM_RESET "\n"
      "  " TERM_CYAN "F          " TERM_RESET "       " TERM_BLUE
      "focus mode." TERM_RESET "\n"

      "  " TERM_CYAN "CTRL + MOUSE LEFT" TERM_RESET " " TERM_BLUE
      "move around." TERM_RESET "\n"
      "  " TERM_CYAN "ARROWS or HJKL" TERM_RESET "    " TERM_BLUE
      "move around." TERM_RESET "\n\n";

  printf("%s", msg);
  exit(EXIT_SUCCESS);
}
