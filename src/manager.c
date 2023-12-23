#include "manager.h"

#include <stdio.h>
#include <stdlib.h>

#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>

// https://wingolog.org/archives/2008/07/26/so-you-want-to-build-a-compositor
// TODO: fix cursor lagging behind on masked click
void allow_input_passthrough(Display *display, Window overlay) {
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);

    XFixesSetWindowShapeRegion(display, overlay, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(display, overlay, ShapeInput, 0, 0, region);

    XFixesDestroyRegion(display, region);
}

// TODO: maybe should propogate so we can exit safely after freeing and stuff? instead of just exit()ing
void clip_shaped(Display *display, Window win, Picture pic) {
    int fixes_opcode, fixes_event, fixes_error;
    if(!XQueryExtension(display, "Fixes", &fixes_opcode, &fixes_event, &fixes_error)
            || !XFixesQueryExtension(display, &fixes_event, &fixes_error)) {
        fprintf(stderr, "Missing \"Fixes\" library\n");
        exit(EXIT_FAILURE);
    }

    XserverRegion region = XFixesCreateRegionFromWindow(display, win, WindowRegionBounding);

    // offset from window
    XWindowAttributes attr;
    XGetWindowAttributes(display, win, &attr);
    XFixesTranslateRegion(display, region, -attr.x, -attr.y);
    XFixesSetPictureClipRegion(display, pic, 0, 0, region);
    XFixesDestroyRegion(display, region);

    int shape_event, shape_error;
    if(!XShapeQueryExtension(display, &shape_event, &shape_error)) {
        fprintf(stderr, "Missing \"shape\" library\n");
        exit(EXIT_FAILURE);
    }

    XShapeSelectInput(display, win, ShapeNotifyMask);
}
