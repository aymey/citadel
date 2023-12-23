#include "manager.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/Xdamage.h>

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

    int damage_opcode, damage_event, damage_error;
    if(!XDamageQueryExtension(display, &damage_event, &damage_error)) {
        fprintf(stderr, "Missing \"Damage\" library\n");
        return EXIT_FAILURE;
    }

    for(int i = 0; i < ScreenCount(display); i++) {
        Window root = RootWindow(display, i);
        XCompositeRedirectSubwindows(display, root, CompositeRedirectAutomatic);

        Window overlay = XCompositeGetOverlayWindow(display, root);
        allow_input_passthrough(display, overlay);
        XRenderPictureAttributes opattr;
        XWindowAttributes oa;
        XGetWindowAttributes(display, overlay, &oa);
        Picture overlay_buffer = XRenderCreatePicture(display, overlay, XRenderFindVisualFormat(display, oa.visual), None,&opattr);
        XMapWindow(display, overlay);
        XSync(display, 0);

        Window unused;
        Window *children;
        unsigned int nchildren;
        XQueryTree(display, root, &unused, &unused, &children, &nchildren);

        for(unsigned int j = 0; j < 10; j++) {
            XWindowAttributes win;
            XGetWindowAttributes(display, children[j], &win);

            XRenderPictureAttributes pattr;
            pattr.subwindow_mode = IncludeInferiors;
            XRenderPictFormat *format = XRenderFindVisualFormat(display, win.visual);
            Picture pic = XRenderCreatePicture(display, children[j],
                    format, CPSubwindowMode,
                    &pattr
                );

            bool hasAlpha = format->type == PictTypeDirect && format->direct.alphaMask;
            XRenderComposite(display, hasAlpha ? PictOpOver : PictOpSrc,
                    pic, None, overlay_buffer,
                    -win.border_width, -win.border_width,
                    0, 0,
                    win.x, win.y,
                    win.width, win.height
                );

            Damage damage = XDamageCreate(display, children[j], XDamageReportNonEmpty);
        }
    }

    while(True) {};

    return EXIT_SUCCESS;
}
