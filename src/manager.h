#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <stdbool.h>
#include <X11/Xlib.h>

void composite_overlay(Display *display);
unsigned long shader_value(unsigned long value);

void allow_input_passthrough(Display *display, Window overlay);

#endif // __MANAGER_H__
