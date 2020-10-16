/*
 * Copyright (C) 2019 ~ 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     huangjie <huangjie@uniontech.com>
 *
 * Maintainer: huangjie <huangjie@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "sleepwatcher.h"

extern "C"
{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #include <unistd.h>
    #include <sys/ioctl.h>
    #include <sys/select.h>
    #include <sys/inotify.h>
    #include <signal.h>
    #include "core/inotify/event_queue.h"
    #include "core/inotify/inotify_utils.h"
}


int keep_running = 0;
extern int open_inotify_fd ();
extern int close_inotify_fd (int);
extern queue_t queue_create ();
/* Signal handler that simply resets a flag to cause termination */
void signal_handler (int signum)
{
  keep_running = 0;
}

sleepwatcher* pWatcher = nullptr;
extern "C"
{
    void file_modifyed(int id)
    {
        (void)id;
        if(pWatcher)
            emit pWatcher->filechanged(true);
    }
}


sleepwatcher::sleepwatcher(const QString &path ,QObject *parent) : wpath(QStringList()<<path),QThread(parent)
{
    pWatcher = this;
}

void sleepwatcher::run()
{
    /* This is the file descriptor for the inotify watch */
    int inotify_fd;

    keep_running = 1;

    /* Set a ctrl-c signal handler */
    if (signal (SIGINT, signal_handler) == SIG_IGN)
      {
        /* Reset to SIG_IGN (ignore) if that was the prior state */
        signal (SIGINT, SIG_IGN);
      }

    /* First we open the inotify dev entry */
    inotify_fd = open_inotify_fd();
    if (inotify_fd > 0)
      {

        /* We will need a place to enqueue inotify events,
           this is needed because if you do not read events
           fast enough, you will miss them. This queue is
           probably too small if you are monitoring something
           like a directory with a lot of files and the directory
           is deleted.
         */
        queue_t q;
        q = queue_create();

        /* This is the watch descriptor returned for each item we are
           watching. A real application might keep these for some use
           in the application. This sample only makes sure that none of
           the watch descriptors is less than 0.
         */
        int wd;


        /* Watch all events (IN_ALL_EVENTS) for the directories and
           files passed in as arguments.
           Read the article for why you might want to alter this for
           more efficient inotify use in your app.
         */
        int index;
        wd = 0;
        printf("\n");
        for (index = 0; (index < wpath.size()) && (wd >= 0); index++)
        {
            std::string tp =  wpath.at(index).toStdString();
            const char* tpath = tp.c_str();
            wd = watch_dir (inotify_fd, tpath, IN_MODIFY);
            /*wd = watch_dir (inotify_fd, argv[index], IN_ALL_EVENTS & ~(IN_CLOSE | IN_OPEN) ); */
        }

        if (wd > 0)
        {
            /* Wait for events and process them until a
                   termination condition is detected
                 */
            process_inotify_events (q, inotify_fd);
        }
        printf ("\nTerminating\n");

        /* Finish up by closing the fd, destroying the queue,
           and returning a proper code
         */
        close_inotify_fd (inotify_fd);
        queue_destroy (q);
    }
}

