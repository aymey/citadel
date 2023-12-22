#include <stdlib.h>
#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>

// https://wingolog.org/archives/2008/07/26/so-you-want-to-build-a-compositor
void allow_input_passthrough(Display *display, Window overlay) {
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);

    XFixesSetWindowShapeRegion(display, overlay, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(display, overlay, ShapeInput, 0, 0, region);

    XFixesDestroyRegion(display, region);
}

int main(void) {
    Display *display = XOpenDisplay(NULL);
    if(!display) {
        fprintf(stderr, "Failed to open display\n");
        return EXIT_FAILURE;
    }

    int composite_opcode, composite_event, composite_error;
    if(!XQueryExtension(display, "Composite", &composite_opcode, &composite_event, &composite_error)) {
        fprintf(stderr, "Missing \"Composite\" library");
        return EXIT_FAILURE;
    }

    Window root = DefaultRootWindow(display);
    Window window = XCreateSimpleWindow(display, root, 0, 0, 100, 100, 10, 10, 0);
    XSelectInput(display, window, StructureNotifyMask);
    XMapWindow(display, window);
    XSync(display, 0);

    XCompositeRedirectSubwindows(display, window, CompositeRedirectManual);
    XSelectInput(display, root, SubstructureNotifyMask);
    XDamageCreate(display, root, XDamageReportRawRectangles);
    XFixesSelectCursorInput(display, window, XFixesDisplayCursorNotifyMask);

    Window overlay = XCompositeGetOverlayWindow(display, root);
    allow_input_passthrough(display, overlay);

    XEvent e;
    while(True) {
        XNextEvent(display, &e);

        switch(e.type) {
            case MapNotify:
                printf("test\n");
                break;
        }
    }

    XCloseDisplay(display);

    return 0;
}
