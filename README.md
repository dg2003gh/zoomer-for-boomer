# 🖥️ zfb – Zoomer for Boomer

`zfb` is a lightweight zoomer tool for Linux desktops using Wayland or X11. It allows you to zoom in/out, move around, and draw on the screen. Built with Raylib and Wayland.

## Inspired by boomer by tsoding

![boomer](https://github.com/tsoding/boomer)

## ✨ Features

- Written in a real language, C, and not some weirdo lang like Rust or Nix  
- Zoom in/out on your desktop  
- Pan with mouse or arrow keys  
- Draw with mouse (pencil mode)  
- Focus highlight around mouse  
- Keyboard shortcuts and mouse gestures  

## 🧰 Requirements

- C compiler (`gcc`, `clang`, etc.)  
- [Raylib](https://www.raylib.com/)  
- Wayland client libraries  
- libpng  

Install dependencies on Debian-based systems:

```bash
sudo apt install build-essential raylib-dev libwayland-dev libpng-dev
