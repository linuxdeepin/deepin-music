#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/select.h>

#include <sys/inotify.h>

#include "event_queue.h"
#include "inotify_utils.h"

extern int keep_running;
static int watched_items;
extern void file_modifyed(int id);
/* Create an inotify instance and open a file descriptor
   to access it */
int open_inotify_fd ()
{
  int fd;

  watched_items = 0;
  fd = inotify_init ();

  if (fd < 0)
    {
      perror ("inotify_init () = ");
    }
  return fd;
}


/* Close the open file descriptor that was opened with inotify_init() */
int close_inotify_fd (int fd)
{
  int r;

  if ((r = close (fd)) < 0)
    {
      perror ("close (fd) = ");
    }

  watched_items = 0;
  return r;
}

/* This method does the work of determining what happened,
   then allows us to act appropriately
*/
void handle_event (queue_entry_t event)
{
  /* If the event was associated with a filename, we will store it here */
  char *cur_event_filename = NULL;
  char *cur_event_file_or_dir = NULL;
  /* This is the watch descriptor the event occurred on */
  int cur_event_wd = event->inot_ev.wd;
  int cur_event_cookie = event->inot_ev.cookie;
  unsigned long flags;

  if (event->inot_ev.len)
    {
      cur_event_filename = event->inot_ev.name;
    }
  if ( event->inot_ev.mask & IN_ISDIR )
    {
      cur_event_file_or_dir = "Dir";
    }
  else 
    {
      cur_event_file_or_dir = "File";
    }
  flags = event->inot_ev.mask & 
    ~(IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED );

  /* Perform event dependent handler routines */
  /* The mask is the magic that tells us what file operation occurred */
  switch (event->inot_ev.mask & 
	  (IN_ALL_EVENTS | IN_UNMOUNT | IN_Q_OVERFLOW | IN_IGNORED))
    {
      /* File was accessed */
    case IN_ACCESS:
      printf ("ACCESS: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* File was modified */
    case IN_MODIFY:
      printf ("MODIFY: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      file_modifyed(cur_event_wd);
      break;

      /* File changed attributes */
    case IN_ATTRIB:
      printf ("ATTRIB: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* File open for writing was closed */
    case IN_CLOSE_WRITE:
      printf ("CLOSE_WRITE: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* File open read-only was closed */
    case IN_CLOSE_NOWRITE:
      printf ("CLOSE_NOWRITE: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* File was opened */
    case IN_OPEN:
      printf ("OPEN: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* File was moved from X */
    case IN_MOVED_FROM:
      printf ("MOVED_FROM: %s \"%s\" on WD #%i. Cookie=%d\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
              cur_event_cookie);
      break;

      /* File was moved to X */
    case IN_MOVED_TO:
      printf ("MOVED_TO: %s \"%s\" on WD #%i. Cookie=%d\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd, 
              cur_event_cookie);
      break;

      /* Subdir or file was deleted */
    case IN_DELETE:
      printf ("DELETE: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* Subdir or file was created */
    case IN_CREATE:
      printf ("CREATE: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* Watched entry was deleted */
    case IN_DELETE_SELF:
      printf ("DELETE_SELF: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* Watched entry was moved */
    case IN_MOVE_SELF:
      printf ("MOVE_SELF: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* Backing FS was unmounted */
    case IN_UNMOUNT:
      printf ("UNMOUNT: %s \"%s\" on WD #%i\n",
	      cur_event_file_or_dir, cur_event_filename, cur_event_wd);
      break;

      /* Too many FS events were received without reading them
         some event notifications were potentially lost.  */
    case IN_Q_OVERFLOW:
      printf ("Warning: AN OVERFLOW EVENT OCCURRED: \n");
      break;

      /* Watch was removed explicitly by inotify_rm_watch or automatically
         because file was deleted, or file system was unmounted.  */
    case IN_IGNORED:
      watched_items--;
      printf ("IGNORED: WD #%d\n", cur_event_wd);
      printf("Watching = %d items\n",watched_items); 
      break;

      /* Some unknown message received */
    default:
      printf ("UNKNOWN EVENT \"%X\" OCCURRED for file \"%s\" on WD #%i\n",
	      event->inot_ev.mask, cur_event_filename, cur_event_wd);
      break;
    }
  /* If any flags were set other than IN_ISDIR, report the flags */
  if (flags & (~IN_ISDIR))
    {
      flags = event->inot_ev.mask;
      printf ("Flags=%lX\n", flags);
    }
}

void handle_events (queue_t q)
{
  queue_entry_t event;
  while (!queue_empty (q))
    {
      event = queue_dequeue (q);
      handle_event (event);
      free (event);
    }
}

int read_events (queue_t q, int fd)
{
  char buffer[16384];
  size_t buffer_i;
  struct inotify_event *pevent;
  queue_entry_t event;
  ssize_t r;
  size_t event_size, q_event_size;
  int count = 0;

  r = read (fd, buffer, 16384);
  if (r <= 0)
    return r;
  buffer_i = 0;
  while (buffer_i < r)
    {
      /* Parse events and queue them. */
      pevent = (struct inotify_event *) &buffer[buffer_i];
      event_size =  offsetof (struct inotify_event, name) + pevent->len;
      q_event_size = offsetof (struct queue_entry, inot_ev.name) + pevent->len;
      event = malloc (q_event_size);
      memmove (&(event->inot_ev), pevent, event_size);
      queue_enqueue (event, q);
      buffer_i += event_size;
      count++;
    }

  printf ("\n%d events queued\n", count);
  return count;
}

int event_check (int fd)
{
  fd_set rfds;
  FD_ZERO (&rfds);
  FD_SET (fd, &rfds);
  /* Wait until an event happens or we get interrupted 
     by a signal that we catch */
  return select (FD_SETSIZE, &rfds, NULL, NULL, NULL);
}

int process_inotify_events (queue_t q, int fd)
{
  while (keep_running && (watched_items > 0))
    {
      if (event_check (fd) > 0)
	{
	  int r;
      r = read_events (q, fd);
	  if (r < 0)
	    {
	      break;
	    }
	  else
	    {
	      handle_events (q);
	    }
	}
    }
  return 0;
}

int watch_dir (int fd, const char *dirname, unsigned long mask)
{
  int wd;
  wd = inotify_add_watch (fd, dirname, mask);
  if (wd < 0)
    {
      printf ("Cannot add watch for \"%s\" with event mask %lX", dirname,
	      mask);
      fflush (stdout);
      perror (" ");
    }
  else
    {
      watched_items++;
      printf ("Watching %s WD=%d\n", dirname, wd);
      printf ("Watching = %d items\n", watched_items); 
    }
  return wd;
}

int ignore_wd (int fd, int wd)
{
  int r;
  r = inotify_rm_watch (fd, wd);
  if (r < 0)
    {
      perror ("inotify_rm_watch(fd, wd) = ");
    }
  else 
    {
      watched_items--;
    }
  return r;
}
