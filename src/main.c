#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>

int main(void) {
    Display *display = XOpenDisplay(NULL);
    if(!display) {
        fprintf(stderr, "Failed to open display\n");
        return EXIT_FAILURE;
    }

    int composite_opcode, composite_event, composite_error;
    if(!XQueryExtension(display, "Composite", &composite_opcode, &composite_event, &composite_error)
            || !XCompositeQueryExtension(display, &composite_event, &composite_error)) {
        fprintf(stderr, "Missing \"Composite\" library");
        return EXIT_FAILURE;
    }

    for(int i = 0; i < ScreenCount(display); i++) {
        Window root = RootWindow(display, i);
        XCompositeRedirectSubwindows(display, root, CompositeRedirectAutomatic);

        Window unused;
        Window *children;
        unsigned int nchildren;
        XQueryTree(display, root, &unused, &unused, &children, &nchildren);

        for(unsigned int j = 0; j < nchildren; j++) {
            XWindowAttributes win;
            XGetWindowAttributes(display, children[j], &win);

            XRenderPictureAttributes pattr;
            pattr.subwindow_mode = IncludeInferiors;
            XRenderPictFormat *format = XRenderFindVisualFormat(display, win.visual);
            Picture pic = XRenderCreatePicture(display, children[j], format, CPSubwindowMode, &pattr);
            // Pixmap pix = XCreatePixmap(display, children[j], win.width, win.height, win.depth);

            bool hasAlpha = format->type == PictTypeDirect && format->direct.alphaMask;
            XRenderComposite(display, hasAlpha ? PictOpOver : PictOpSrc,
                    pic, None, pic,
                    0, 0,
                    0, 0,
                    win.x, win.y,
                    win.width, win.height
                );

            // XFreePixmap(display, pix);
        }
    }

    return EXIT_SUCCESS;
}
