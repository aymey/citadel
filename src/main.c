#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>

#include "manager.h"

int main(void) {
    Display *display = XOpenDisplay(NULL);
    if(!display) {
        fprintf(stderr, "Failed to open display\n");
        return EXIT_FAILURE;
    }

    int composite_opcode, composite_event, composite_error;
    if(!XQueryExtension(display, "Composite", &composite_opcode, &composite_event, &composite_error) || !XCompositeQueryExtension(display, &composite_event, &composite_error)
            || !XCompositeQueryExtension(display, &composite_event, &composite_error)) {
        fprintf(stderr, "Missing \"Composite\" library\n");
        return EXIT_FAILURE;
    }

    composite_overlay(display);

    XEvent e;
    while(True) {
        composite_overlay(display);

        if(XPending(display)) {
            XNextEvent(display, &e);

            switch(e.type) {
                case ConfigureNotify:
                case Expose:
                case CreateNotify:
                    break;
            }
        }
    }

    return EXIT_SUCCESS;
}
