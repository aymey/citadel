#include "manager.h"

#include <X11/X.h>
#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>
#include <X11/extensions/Xdamage.h>

void composite_overlay(Display *display) {
    for(int i = 0; i < ScreenCount(display); i++) {
        Window root = RootWindow(display, i);
        XCompositeRedirectSubwindows(display, root, CompositeRedirectAutomatic);

        Window overlay = XCompositeGetOverlayWindow(display, root);
        allow_input_passthrough(display, overlay);
        XRenderPictureAttributes opattr;
        XClearWindow(display, overlay);
        XWindowAttributes oa;
        XGetWindowAttributes(display, overlay, &oa);
        Picture overlay_buffer = XRenderCreatePicture(display, overlay, XRenderFindVisualFormat(display, oa.visual), None,&opattr);
        XMapWindow(display, overlay);
        XSync(display, 0);

        Window unused;
        Window *children;
        unsigned int nchildren;
        XQueryTree(display, root, &unused, &unused, &children, &nchildren);

        // TODO: for some reason alot of browser stuff make XRenderCreatePicture() error (like "firefox", "chromium clipboard")
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

            XRenderFreePicture(display, pic);
        }
    }
}

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
