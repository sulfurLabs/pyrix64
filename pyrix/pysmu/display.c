/*
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Copyright (C) 2026 sulfurLabs
 *
 * PROJECT: pyrix64 - pysmu
 * FILE: display.c
 * DESC.: Wayland and X11 native display implementation
 *
 * CREATED BY: emex
 * UPDATED BY: offihito
 *
 * This file is part of pyrix64.
 *
 * pyrix64 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pyrix64 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * See the LICENSE file for the full license text.
 * You can also find the GPL-3.0 license at:
 * https://www.gnu.org/licenses/gpl-3.0.html
 */

#define _GNU_SOURCE 1

#include <fcntl.h>
#include <poll.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include <wayland-client.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "display.h"
#include "vga.h"
#include "xdg-shell-client-protocol.h"
#include "xdg-decoration-unstable-v1-client-protocol.h"

typedef enum
{
    Pyrix64BackendNone = 0,
    Pyrix64BackendWayland,
    Pyrix64BackendX11,
    Pyrix64BackendTerminal
} Pyrix64Backend;

static Pyrix64Backend CurrentBackend = Pyrix64BackendNone;
static uint32_t *FrameBuffer = NULL;
static size_t FrameBufferSize = 0;
static bool WindowRunning = true;

// Wayland state
static struct wl_display *WlDisplay = NULL;
static struct wl_registry *WlRegistry = NULL;
static struct wl_compositor *WlCompositor = NULL;
static struct wl_shm *WlShm = NULL;
static struct xdg_wm_base *XdgWmBase = NULL;
static struct zxdg_decoration_manager_v1 *DecorationManager = NULL;
static struct zxdg_toplevel_decoration_v1 *ToplevelDecoration = NULL;
static struct wl_surface *WlSurface = NULL;
static struct xdg_surface *XdgSurface = NULL;
static struct xdg_toplevel *XdgToplevel = NULL;
static struct wl_buffer *WlBuffer = NULL;

// X11 state
static Display *X11Dpy = NULL;
static Window X11Win = 0;
static GC X11Gc = 0;
static XImage *X11Image = NULL;
static Atom X11WmDelete = 0;

static void XdgSurfaceHandleConfigure(
    void *Data,
    struct xdg_surface *Surface,
    uint32_t Serial
){
    (void)Data;
    xdg_surface_ack_configure(Surface, Serial);
}

static const struct xdg_surface_listener XdgSurfaceListener =
{
    .configure = XdgSurfaceHandleConfigure
};

static void XdgToplevelHandleConfigure(
    void *Data,
    struct xdg_toplevel *Toplevel,
    int32_t Width,
    int32_t Height,
    struct wl_array *States
){
    (void)Data;
    (void)Toplevel;
    (void)Width;
    (void)Height;
    (void)States;
}

static void XdgToplevelHandleClose(
    void *Data,
    struct xdg_toplevel *Toplevel
){
    (void)Data;
    (void)Toplevel;
    WindowRunning = false;
}

static const struct xdg_toplevel_listener XdgToplevelListener =
{
    .configure = XdgToplevelHandleConfigure,
    .close = XdgToplevelHandleClose
};

static void XdgWmBaseHandlePing(
    void *Data,
    struct xdg_wm_base *WmBase,
    uint32_t Serial
){
    (void)Data;
    xdg_wm_base_pong(WmBase, Serial);
}

static const struct xdg_wm_base_listener XdgWmBaseListener =
{
    .ping = XdgWmBaseHandlePing
};

static void RegistryHandleGlobal(
    void *Data,
    struct wl_registry *Registry,
    uint32_t Name,
    const char *Interface,
    uint32_t Version
){
    (void)Data;
    (void)Version;

    if (strcmp(Interface, wl_compositor_interface.name) == 0)
    {
        WlCompositor = wl_registry_bind(Registry, Name, &wl_compositor_interface, 4);
    }
    else if (strcmp(Interface, wl_shm_interface.name) == 0)
    {
        WlShm = wl_registry_bind(Registry, Name, &wl_shm_interface, 1);
    }
    else if (strcmp(Interface, xdg_wm_base_interface.name) == 0)
    {
        XdgWmBase = wl_registry_bind(Registry, Name, &xdg_wm_base_interface, 1);
        xdg_wm_base_add_listener(XdgWmBase, &XdgWmBaseListener, NULL);
    }
    else if (strcmp(Interface, zxdg_decoration_manager_v1_interface.name) == 0)
    {
        DecorationManager = wl_registry_bind(Registry, Name, &zxdg_decoration_manager_v1_interface, 1);
    }
}

static void RegistryHandleGlobalRemove(
    void *Data,
    struct wl_registry *Registry,
    uint32_t Name
){
    (void)Data;
    (void)Registry;
    (void)Name;
}

static const struct wl_registry_listener RegistryListener =
{
    .global = RegistryHandleGlobal,
    .global_remove = RegistryHandleGlobalRemove
};

static bool InitWayland(const char *Title)
{
    WlDisplay = wl_display_connect(NULL);

    if (WlDisplay == NULL)
    {
        return false;
    }

    WlRegistry = wl_display_get_registry(WlDisplay);
    wl_registry_add_listener(WlRegistry, &RegistryListener, NULL);
    wl_display_roundtrip(WlDisplay);

    if (WlCompositor == NULL || WlShm == NULL || XdgWmBase == NULL)
    {
        wl_display_disconnect(WlDisplay);
        WlDisplay = NULL;

        return false;
    }

    WlSurface = wl_compositor_create_surface(WlCompositor);
    XdgSurface = xdg_wm_base_get_xdg_surface(XdgWmBase, WlSurface);
    xdg_surface_add_listener(XdgSurface, &XdgSurfaceListener, NULL);

    XdgToplevel = xdg_surface_get_toplevel(XdgSurface);
    xdg_toplevel_add_listener(XdgToplevel, &XdgToplevelListener, NULL);
    xdg_toplevel_set_title(XdgToplevel, Title);

    if (DecorationManager != NULL)
    {
        ToplevelDecoration = zxdg_decoration_manager_v1_get_toplevel_decoration(DecorationManager, XdgToplevel);
        zxdg_toplevel_decoration_v1_set_mode(ToplevelDecoration, ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
    }

    wl_surface_commit(WlSurface);
    wl_display_roundtrip(WlDisplay);

    FrameBufferSize = Pyrix64DisplayWidth * Pyrix64DisplayHeight * sizeof(uint32_t);

    int Fd = memfd_create("pyrix64-shm", MFD_CLOEXEC);

    if (Fd < 0)
    {
        return false;
    }

    if (ftruncate(Fd, FrameBufferSize) < 0)
    {
        close(Fd);

        return false;
    }

    FrameBuffer = mmap(NULL, FrameBufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, Fd, 0);

    if (FrameBuffer == MAP_FAILED)
    {
        close(Fd);

        return false;
    }

    struct wl_shm_pool *Pool = wl_shm_create_pool(WlShm, Fd, FrameBufferSize);
    WlBuffer = wl_shm_pool_create_buffer(Pool, 0, Pyrix64DisplayWidth, Pyrix64DisplayHeight, Pyrix64DisplayWidth * sizeof(uint32_t), WL_SHM_FORMAT_XRGB8888);
    wl_shm_pool_destroy(Pool);
    close(Fd);

    // clear screen
    memset(FrameBuffer, 0, FrameBufferSize);

    wl_surface_attach(WlSurface, WlBuffer, 0, 0);
    wl_surface_commit(WlSurface);
    wl_display_flush(WlDisplay);

    return true;
}

static bool InitX11(const char *Title)
{
    X11Dpy = XOpenDisplay(NULL);

    if (X11Dpy == NULL)
    {
        return false;
    }

    int Screen = DefaultScreen(X11Dpy);
    Window Root = RootWindow(X11Dpy, Screen);

    X11Win = XCreateSimpleWindow(
        X11Dpy,
        Root,
        100, 100,
        Pyrix64DisplayWidth, Pyrix64DisplayHeight,
        1,
        BlackPixel(X11Dpy, Screen),
        0x000000
    );

    XStoreName(X11Dpy, X11Win, Title);
    XSelectInput(X11Dpy, X11Win, ExposureMask | KeyPressMask | StructureNotifyMask);

    X11WmDelete = XInternAtom(X11Dpy, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(X11Dpy, X11Win, &X11WmDelete, 1);

    X11Gc = XCreateGC(X11Dpy, X11Win, 0, NULL);
    XMapWindow(X11Dpy, X11Win);

    FrameBufferSize = Pyrix64DisplayWidth * Pyrix64DisplayHeight * sizeof(uint32_t);
    FrameBuffer = malloc(FrameBufferSize);

    if (FrameBuffer == NULL)
    {
        return false;
    }

    memset(FrameBuffer, 0, FrameBufferSize);

    X11Image = XCreateImage(
        X11Dpy,
        DefaultVisual(X11Dpy, Screen),
        24,
        ZPixmap,
        0,
        (char *)FrameBuffer,
        Pyrix64DisplayWidth,
        Pyrix64DisplayHeight,
        32,
        0
    );

    XFlush(X11Dpy);

    return true;
}

bool Pyrix64DisplayInit(const char *Title)
{
    Pyrix64VgaInit();

    if (getenv("WAYLAND_DISPLAY") != NULL)
    {
        if (InitWayland(Title))
        {
            CurrentBackend = Pyrix64BackendWayland;
            printf("::  display initialized using native Wayland\n");

            return true;
        }
    }

    if (getenv("DISPLAY") != NULL)
    {
        if (InitX11(Title))
        {
            CurrentBackend = Pyrix64BackendX11;
            printf("::  display initialized using native X11\n");

            return true;
        }
    }

    CurrentBackend = Pyrix64BackendTerminal;
    printf("::  display initialized using terminal fallback\n");

    return true;
}

uint32_t *Pyrix64DisplayGetFrameBuffer(void)
{
    return FrameBuffer;
}

void Pyrix64DisplayFlush(void)
{
    if (CurrentBackend == Pyrix64BackendWayland && WlSurface != NULL && WlBuffer != NULL)
    {
        wl_surface_attach(WlSurface, WlBuffer, 0, 0);
        wl_surface_damage_buffer(WlSurface, 0, 0, Pyrix64DisplayWidth, Pyrix64DisplayHeight);
        wl_surface_commit(WlSurface);
        wl_display_flush(WlDisplay);
    }
    else if (CurrentBackend == Pyrix64BackendX11 && X11Dpy != NULL)
    {
        XPutImage(X11Dpy, X11Win, X11Gc, X11Image, 0, 0, 0, 0, Pyrix64DisplayWidth, Pyrix64DisplayHeight);
        XFlush(X11Dpy);
    }
}

bool Pyrix64DisplayPoll(void)
{
    if (CurrentBackend == Pyrix64BackendWayland && WlDisplay != NULL)
    {
        wl_display_flush(WlDisplay);

        struct pollfd Pfd = {
            .fd = wl_display_get_fd(WlDisplay),
            .events = POLLIN | POLLERR | POLLHUP,
            .revents = 0
        };

        if (poll(&Pfd, 1, 0) > 0)
        {
            if (wl_display_dispatch(WlDisplay) < 0)
            {
                WindowRunning = false;
            }
        }
        else
        {
            wl_display_dispatch_pending(WlDisplay);
        }
    }
    else if (CurrentBackend == Pyrix64BackendX11 && X11Dpy != NULL)
    {
        while (XPending(X11Dpy) > 0)
        {
            XEvent Event;
            XNextEvent(X11Dpy, &Event);

            if (Event.type == ClientMessage)
            {
                if ((Atom)Event.xclient.data.l[0] == X11WmDelete)
                {
                    WindowRunning = false;
                }
            }
        }
    }

    return WindowRunning;
}

void Pyrix64DisplayFrameSync(uint32_t SleepMs)
{
    Pyrix64DisplayFlush();
    Pyrix64DisplayPoll();

    if (SleepMs > 0 && WindowRunning)
    {
        int Fd = -1;

        if (CurrentBackend == Pyrix64BackendWayland && WlDisplay != NULL)
        {
            Fd = wl_display_get_fd(WlDisplay);
        }
        else if (CurrentBackend == Pyrix64BackendX11 && X11Dpy != NULL)
        {
            Fd = ConnectionNumber(X11Dpy);
        }

        if (Fd >= 0)
        {
            struct pollfd Pfd = {
                .fd = Fd,
                .events = POLLIN,
                .revents = 0
            };
            poll(&Pfd, 1, (int)SleepMs);
        }
        else
        {
            usleep(SleepMs * 1000);
        }

        Pyrix64DisplayPoll();
    }
}

void Pyrix64DisplayWaitClose(void)
{
    if (CurrentBackend == Pyrix64BackendTerminal)
    {
        return;
    }

    printf("::  press Ctrl+C or close the window to exit...\n");

    while (WindowRunning)
    {
        if (CurrentBackend == Pyrix64BackendWayland && WlDisplay != NULL)
        {
            if (wl_display_dispatch(WlDisplay) < 0)
            {
                break;
            }
        }
        else if (CurrentBackend == Pyrix64BackendX11)
        {
            Pyrix64DisplayPoll();
            usleep(20000);
        }
        else
        {
            break;
        }
    }
}

void Pyrix64DisplayClose(void)
{
    if (CurrentBackend == Pyrix64BackendWayland)
    {
        if (WlBuffer) wl_buffer_destroy(WlBuffer);
        if (ToplevelDecoration) zxdg_toplevel_decoration_v1_destroy(ToplevelDecoration);
        if (XdgToplevel) xdg_toplevel_destroy(XdgToplevel);
        if (XdgSurface) xdg_surface_destroy(XdgSurface);
        if (WlSurface) wl_surface_destroy(WlSurface);
        if (DecorationManager) zxdg_decoration_manager_v1_destroy(DecorationManager);
        if (XdgWmBase) xdg_wm_base_destroy(XdgWmBase);
        if (WlShm) wl_shm_destroy(WlShm);
        if (WlCompositor) wl_compositor_destroy(WlCompositor);
        if (WlRegistry) wl_registry_destroy(WlRegistry);
        if (WlDisplay) wl_display_disconnect(WlDisplay);

        if (FrameBuffer && FrameBuffer != MAP_FAILED)
        {
            munmap(FrameBuffer, FrameBufferSize);
        }
    }
    else if (CurrentBackend == Pyrix64BackendX11)
    {
        if (X11Image) XDestroyImage(X11Image);
        if (X11Gc) XFreeGC(X11Dpy, X11Gc);
        if (X11Win) XDestroyWindow(X11Dpy, X11Win);
        if (X11Dpy) XCloseDisplay(X11Dpy);
    }

    CurrentBackend = Pyrix64BackendNone;
}
