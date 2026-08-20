// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef _COMPAT_POLL_H
#define _COMPAT_POLL_H

#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>

/* struct pollfd is already defined by winsock2.h/ws2def.h as WSAPOLLFD */

#ifndef POLLIN
#define POLLIN      0x0100
#endif
#ifndef POLLOUT
#define POLLOUT     0x0080
#endif
#ifndef POLLERR
#define POLLERR     0x0001
#endif
#ifndef POLLHUP
#define POLLHUP     0x0002
#endif
#ifndef POLLNVAL
#define POLLNVAL    0x0008
#endif

static inline int poll(struct pollfd *fds, unsigned int nfds, int timeout) {
    fd_set r, w, e;
    FD_ZERO(&r); FD_ZERO(&w); FD_ZERO(&e);

    for (unsigned int i = 0; i < nfds; i++) {
        fds[i].revents = 0;

        /* Detect invalid sockets before adding to fd_sets */
        int err = 0;
        int errlen = sizeof(err);
        if (getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen) != 0 || err != 0) {
            fds[i].revents |= POLLNVAL;
            continue;
        }

        if (fds[i].events & POLLIN)  FD_SET(fds[i].fd, &r);
        if (fds[i].events & POLLOUT) FD_SET(fds[i].fd, &w);
        FD_SET(fds[i].fd, &e);
    }

    timeval tv;
    tv.tv_sec  = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    int ret = select(0, &r, &w, &e, timeout >= 0 ? &tv : NULL);

    if (ret <= 0) {
        return ret;
    }

    for (unsigned int i = 0; i < nfds; i++) {
        if (fds[i].revents & POLLNVAL)
            continue;

        if (FD_ISSET(fds[i].fd, &r)) fds[i].revents |= POLLIN;
        if (FD_ISSET(fds[i].fd, &w)) fds[i].revents |= POLLOUT;

        if (FD_ISSET(fds[i].fd, &e)) {
            int err = 0;
            int errlen = sizeof(err);
            getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, (char *)&err, &errlen);
            if (err == WSAECONNABORTED || err == WSAECONNRESET || err == WSAENOTCONN) {
                fds[i].revents |= POLLHUP;
            } else {
                fds[i].revents |= POLLERR;
            }
        } else if (!(fds[i].revents & (POLLIN | POLLOUT)) && (fds[i].events & (POLLIN | POLLOUT))) {
            /* Socket requested events but none were ready and no error — possible disconnect */
            char buf;
            int n = recv(fds[i].fd, &buf, 1, MSG_PEEK);
            if (n == 0) {
                fds[i].revents |= POLLHUP;
            }
        }
    }
    return ret;
}

#endif /* _WIN32 */
#endif /* _COMPAT_POLL_H */
