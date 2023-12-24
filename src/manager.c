#include "manager.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/extensions/Xfixes.h>
#include <X11/extensions/Xrender.h>
#include <X11/extensions/shape.h>
#include <X11/extensions/Xcomposite.h>

void composite_overlay(Display *display) {
    for(int i = 0; i < ScreenCount(display); i++) {
        Window root = RootWindow(display, i);
        XCompositeRedirectSubwindows(display, root, CompositeRedirectAutomatic);

        Window overlay = XCompositeGetOverlayWindow(display, root);
        XSelectInput(display, overlay, SubstructureNotifyMask | ExposureMask);
        XClearWindow(display, overlay);
        allow_input_passthrough(display, overlay);

        XWindowAttributes oa;
        XGetWindowAttributes(display, overlay, &oa);

        XImage *img = XGetImage(display, overlay, 0, 0, oa.width, oa.height, AllPlanes, XYPixmap);
        XGCValues xgcv;
        GC gc = XCreateGC(display, overlay, 0, &xgcv);

        XMapWindow(display, overlay);
        XSync(display, 0);


        Window unused;
        Window *children;
        unsigned int nchildren;
        XQueryTree(display, root, &unused, &unused, &children, &nchildren);

        for(unsigned int j = 0; j < nchildren; j++) {
            XWindowAttributes win;
            XGetWindowAttributes(display, children[j], &win);

            for(int x = 0; x < win.width; x++) {
                for(int y = 0; y < win.height; y++) {
                    if(win.x + x > oa.width || win.x + x < 0) break;
                    if(win.y + y > oa.height || win.y + y < 0) break;

                    XPutPixel(img,
                            win.x+x, win.y+y,
                            shader_value(XGetPixel(img, win.x+x, win.y+y))
                        );
                }
            }
        }

        XPutImage(display, overlay, gc, img, 0, 0, 0, 0, oa.width, oa.height);
    }
}

unsigned long shader_value(unsigned long value) {
    return value*2;
}

// https://wingolog.org/archives/2008/07/26/so-you-want-to-build-a-compositor
// TODO: fix cursor lagging behind on masked click
void allow_input_passthrough(Display *display, Window overlay) {
    XserverRegion region = XFixesCreateRegion(display, NULL, 0);

    XFixesSetWindowShapeRegion(display, overlay, ShapeBounding, 0, 0, 0);
    XFixesSetWindowShapeRegion(display, overlay, ShapeInput, 0, 0, region);

    XFixesDestroyRegion(display, region);
}
