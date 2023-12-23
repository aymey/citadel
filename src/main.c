#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

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

    int render_opcode, render_event, render_error;
    if(!XRenderQueryExtension(display, &render_event, &render_error)) {
        fprintf(stderr, "Missing \"Render\" library\n");
        return EXIT_FAILURE;
    }

    int damage_opcode, damage_event = 0, damage_error;
    if(!XDamageQueryExtension(display, &damage_event, &damage_error)) {
        fprintf(stderr, "Missing \"Damage\" library\n");
        return EXIT_FAILURE;
    }

    composite_overlay(display);

    XEvent e;
    while(True) {
        if(XPending(display)) {
            XNextEvent(display, &e);

            if(e.type == damage_event + XDamageNotify) {
                printf("damage notify\n");

                XDamageNotifyEvent dmg = *(XDamageNotifyEvent *) &e;
                XDamageSubtract(display, dmg.damage, None, None);
                composite_overlay(display);
            }
            // else if(e.type == shape_event + ShapeNotify) {}
            else {
                switch(e.type) {
                    case ConfigureNotify:
                    case Expose:
                    case CreateNotify:
                        composite_overlay(display);
                        break;
                }
            }
        }
    }

    return EXIT_SUCCESS;
}
