#include <X11/Xlib.h>
#include <X11/Xos.h>
#include <X11/Xutil.h>
#include <X11/extensions/XI.h>
#include <X11/extensions/XInput.h>
#include <X11/extensions/XInput2.h>

#include <stdio.h>
#include <stdlib.h>

#include <iostream>

Display* dis;
int screen;
Window win;
GC gc;
int xi_opcode;

/* here are our X routines declared! */
void init_x();
void close_x();
void redraw();
void setupXI2();

int main()
{
    XEvent event; /* the XEvent declaration !!! */
    XGenericEventCookie* cookie = reinterpret_cast<XGenericEventCookie*>(&event);
    KeySym key; /* a dealie-bob to handle KeyPress Events */
    char text[255]; /* a char buffer for KeyPress Events */

    init_x();

    Atom touchpad_atom = XInternAtom(dis, XI_TOUCHPAD, False);
    int dev_count = 0;
    XDeviceInfo* dev_info = XListInputDevices(dis, &dev_count);

    /* look for events forever... */
    while (1) {
        /* get the next event and stuff it into our event variable.
		   Note:  only events we set the mask for are detected!
		*/
        XNextEvent(dis, &event);

        if (event.type == Expose && event.xexpose.count == 0) {
            /* the window was exposed redraw it! */
            redraw();
        }
        if (event.type == KeyPress && XLookupString(&event.xkey, text, 255, &key, 0) == 1) {
            /* use the XLookupString routine to convert the invent
		   KeyPress data into regular text.  Weird but necessary...
		*/
            if (text[0] == 'q') {
                close_x();
            }
            printf("You pressed the %c key!\n", text[0]);
        }
        if (event.type == ButtonPress) {
            /* tell where the mouse Button was Pressed */
            int x = event.xbutton.x,
                y = event.xbutton.y;

            strcpy(text, "X is FUN!");
            XSetForeground(dis, gc, rand() % event.xbutton.x % 255);
            XDrawString(dis, win, gc, x, y, text, strlen(text));
        }

        if (XGetEventData(dis, cookie))
            switch (cookie->evtype) {
            case XI_TouchBegin:
            case XI_TouchUpdate:
            case XI_TouchEnd: {
                auto xiev = static_cast<XIDeviceEvent*>(cookie->data);
                XDrawString(dis, win, gc, xiev->event_x, xiev->event_y, "O", 1);
                break;
            }
            default: {
                std::cout << event.xgeneric.evtype << "\n";
            }
            }
    }

    return 0;
}

void init_x()
{
    /* get the colors black and white (see section for details) */
    dis = XOpenDisplay((char*)0);
    screen = DefaultScreen(dis);
    unsigned long black = BlackPixel(dis, screen);
    unsigned long white = WhitePixel(dis, screen);

    win = XCreateSimpleWindow(dis, DefaultRootWindow(dis), 0, 0,
        300, 300, 5, black, white);
    XSetStandardProperties(dis, win, "Howdy", "Hi", None, NULL, 0, NULL);
    XSelectInput(dis, win, ExposureMask | ButtonPressMask | KeyPressMask | PointerMotionMask);

    setupXI2();

    gc = XCreateGC(dis, win, 0, 0);
    XSetBackground(dis, gc, white);
    XSetForeground(dis, gc, black);
    XClearWindow(dis, win);
    XMapRaised(dis, win);

    /* check XInput extension */
    {
        int ev;
        int err;

        if (!XQueryExtension(dis, "XInputExtension", &xi_opcode, &ev, &err)) {
            printf("X Input extension not available.\n");
            exit(1);
        }
    }
};

void setupXI2()
{
    unsigned char mask[XIMaskLen(XI_LASTEVENT)];
    memset(mask, 0, sizeof(mask));

    // XISetMask(mask, XI_Enter);
    // XISetMask(mask, XI_Leave);
    // XISetMask(mask, XI_FocusIn);
    // XISetMask(mask, XI_FocusOut);

    XISetMask(mask, XI_TouchBegin);
    XISetMask(mask, XI_TouchUpdate);
    XISetMask(mask, XI_TouchEnd);

    // XISetMask(mask, XI_ButtonPress);
    // XISetMask(mask, XI_ButtonRelease);
    // XISetMask(mask, XI_Motion);

    // XISetMask(mask, XI_HierarchyChanged);
    // XISetMask(mask, XI_DeviceChanged);

    XIEventMask evmask;
    evmask.deviceid = XIAllDevices;
    evmask.mask_len = sizeof(mask);
    evmask.mask = mask;
    XISelectEvents(dis, win, &evmask, 1);
    // can skip this?
    XFlush(dis);
}

void close_x()
{
    XFreeGC(dis, gc);
    XDestroyWindow(dis, win);
    XCloseDisplay(dis);
    exit(1);
};

void redraw()
{
    XClearWindow(dis, win);
};