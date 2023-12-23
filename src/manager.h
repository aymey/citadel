#ifndef __MANAGER_H__
#define __MANAGER_H__

#include <stdbool.h>
#include <X11/Xlib.h>
#include <X11/extensions/Xrender.h>

bool check_shaped(Display *display, Window win);
void clip_shaped(Display *display, Window win, Picture pic);

#endif // __MANAGER_H__
