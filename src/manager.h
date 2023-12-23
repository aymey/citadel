#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

void composite_overlay(Display *display);

void allow_input_passthrough(Display *display, Window overlay);

bool is_shaped(Display *display, Window win);
void clip_shaped(Display *display, Window win, Picture pic);

// void damage

#endif // __MANAGER_H__
