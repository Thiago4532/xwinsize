#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <errno.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#define panic(fmt, ...) do { \
    fprintf(stderr, "xwinsize: " fmt "\n", ##__VA_ARGS__); \
    exit(1); \
} while(0)

Display* dpy;
Atom XA_UTF8_STRING;
Atom PROP_NET_WM_NAME, PROP_WM_NAME;

void setup_display(int argc, char* argv[]) {
    dpy = XOpenDisplay(NULL);
    if (!dpy)
        exit(1);

    XA_UTF8_STRING = XInternAtom(dpy, "UTF8_STRING", False);
    if (!XA_UTF8_STRING)
        exit(1);

    PROP_NET_WM_NAME = XInternAtom(dpy, "_NET_WM_NAME", False);
    if (!PROP_NET_WM_NAME)
        exit(1);

    PROP_WM_NAME = XInternAtom(dpy, "WM_NAME", False);
    if (!PROP_WM_NAME)
        exit(1);
}

void close_display() {
    if (dpy)
        XCloseDisplay(dpy);
    dpy = NULL;
}

void usage(bool error) {
    FILE* out = error ? stderr : stdout;

    fprintf(out, "usage: xwinsize <window option> [options ...]\n");
    fprintf(out, "You must specify one of the following window options:\n");
    fprintf(out, "    --name <window_name>: Select the window with the specified name\n");
    fprintf(out, "    --id <window_id>: Select the window with the specified id\n");
    fprintf(out, "    --root: Select the root window\n");
    fprintf(out, "\n");
    fprintf(out, "The following options are optional:\n");
    fprintf(out, "    --monitor|-m: Keep monitoring the window size forever, printing it everytime it changes\n");
    fprintf(out, "    --help|-h: Print the command line usage\n");
    exit(1);
}

// Same thing as panic, but print usage before exiting the program
#define die(fmt, ...) do { \
    fprintf(stderr, "xwinsize: " fmt "\n\n", ##__VA_ARGS__); \
    usage(true); \
} while(0)

Window select_window_by_id(char* id) {
    if (!id)
        die("id must receive a parameter");

    char* endptr;
    size_t id_len = strlen(id);

    Window win;
    if (id_len > 1 && id[0] == '0' && (id[1] == 'x' || id[1] == 'X')) { // reading hexadecimal
        win = strtoul(id + 2, &endptr, 16);
    } else {
        win = strtoul(id, &endptr, 10);
    }

    if (*endptr)
        die("id: invalid number");
    if (win == 0)
        die("id: id must not be 0");

    return win;
}

Window get_window_by_name(Window win, const char* name, size_t len) {
    char* window_name;
    
    Atom prop_type;
    int prop_format;
    unsigned long length, bytes_after;
    unsigned char* data;

    if (XGetWindowProperty(dpy, win, PROP_NET_WM_NAME, 0, 8192, False,
                XA_UTF8_STRING, &prop_type, &prop_format, &length, &bytes_after, &data) != Success)
            return 0;

    if (prop_type == XA_UTF8_STRING) {
        if (length == len && memcmp(data, name, length) == 0)
            return win;
    }

    if (XGetWindowProperty(dpy, win, PROP_WM_NAME, 0, 8192, False,
                AnyPropertyType, &prop_type, &prop_format, &length, &bytes_after, &data) != Success)
            return 0;

    if (prop_type == XA_STRING) {
        if (length == len && memcmp(data, name, len) == 0)
            return win;
    }

    Window *children, dummy;
    unsigned int nchildren;

    if (!XQueryTree(dpy, win, &dummy, &dummy, &children, &nchildren))
        return 0;

    Window w = 0;
    for (int i = 0; i < nchildren; i++) {
        w = get_window_by_name(children[i], name, len);
        if (w != 0)
            break;
    }

    if (children)
        XFree(children);
    return w;
}

Window select_window_by_name(char* name) {
    if (!name)
        die ("name must receive a parameter");

    Window win = get_window_by_name(XDefaultRootWindow(dpy), name, strlen(name));
    if (win == 0)
        die("name: failed to find a window with the specified name");
    return win;
}

int main(int argc, char* argv[]) {
    if (setvbuf(stdout, NULL, _IOLBF, 0) != 0)
        panic("setvbuf: %s", strerror(errno));

    setup_display(argc, argv);

    bool monitor = false;
    Window win = 0;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--id") == 0) {
            if (win != 0)
                die("you must select only one window");
            win = select_window_by_id(argv[++i]);
        } else if (strcmp(argv[i], "--name") == 0) {
            if (win != 0)
                die("you must select only one window");
            win = select_window_by_name(argv[++i]);
        } else if (strcmp(argv[i], "--root") == 0) {
            if (win != 0)
                die("you must select only one window");
            win = XDefaultRootWindow(dpy);
        } else if (strcmp(argv[i], "--monitor") == 0 || strcmp(argv[i], "-m") == 0) {
            monitor = true;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(false);
        } else {
            die("unrecognized argument: %s", argv[i]);
        }
    }

    if (win == 0)
        die ("you must specify a window");

    XWindowAttributes attr;
    XGetWindowAttributes(dpy, win, &attr);
    printf("%d %d %d %d\n", attr.x, attr.y, attr.width, attr.height);

    if (!monitor) {
        close_display();
        return 0;
    }

    XSelectInput(dpy, win, StructureNotifyMask);

    XEvent ev;
    while (true) {
        XNextEvent(dpy, &ev);
        if (ev.type == UnmapNotify || ev.type == DestroyNotify)
            break;

        if (ev.type == ConfigureNotify) {
            XWindowAttributes attr;
            if (!XGetWindowAttributes(dpy, win, &attr)) {
                close_display();
                return 1;
            }

            printf("%d %d %d %d\n", attr.x, attr.y, attr.width, attr.height);
        }
    }

    close_display();
    return 0;
}
