#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "dt.h"
#include "queue.h"
#include "compress.h"
#include "decompress.h"

static gboolean
ui_update (gpointer d)
{
  AppData *a = d;
  int total = atomic_load (&a->total);
  int done = atomic_load (&a->completed);
  double frac = (total > 0) ? (double) done / total : 0.0;
  char txt[32];
  snprintf (txt, sizeof (txt), "%d / %d", done, total);
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (a->progress), frac);
  gtk_progress_bar_set_text (GTK_PROGRESS_BAR (a->progress), txt);
  return G_SOURCE_REMOVE;
}

static void
set_state (AppData *a, gboolean run, const char *t)
{
  atomic_store (&a->busy, run);
  gtk_widget_set_sensitive (a->start_btn, !run);
  gtk_label_set_text (GTK_LABEL (a->status), t);
}

static void *
worker (void *d)
{
  AppData *a = d;
  set_state (a, true, "Working...");
  atomic_store (&a->completed, 0);
  TaskItem task;
  while (queue_pop (&a->queue, &task))
    {
      task.status = STATUS_RUNNING;
      DWORD res;
      if (task.mode == MODE_COMPRESS)
	res = compress (task.data.paths.in_path, task.data.paths.out_path);
      else
	res = decompress (task.data.paths.in_path, task.data.paths.out_path);
      task.status = (res == EXIT_SUCCESS) ? STATUS_DONE : STATUS_ERROR;
      atomic_fetch_add (&a->completed, 1);
      g_idle_add (ui_update, a);
      if (atomic_load (&a->completed) >= atomic_load (&a->total))
	break;
    }
  set_state (a, false, "Done");
  return NULL;
}

static void
on_start (GtkButton *btn, AppData *a)
{
  (void) btn;
  if (atomic_load (&a->busy))
    return;
  if (atomic_load (&a->total) == 0)
    {
      set_state (a, FALSE, "Add files to queue");
      return;
    }
  pthread_t tid;
  pthread_create (&tid, NULL, worker, a);
  pthread_detach (tid);
}

static void
on_picked_multiple (GObject *src, GAsyncResult *res, gpointer user_data)
{
  AppData *a = (AppData *) user_data;
  GError *err = NULL;
  GListModel *files =
    gtk_file_dialog_open_multiple_finish (GTK_FILE_DIALOG (src), res, &err);
  if (!files || err)
    {
      g_clear_error (&err);
      return;
    }
  gboolean is_comp = gtk_switch_get_active (GTK_SWITCH (a->mode_switch));
  enum Mode mode = is_comp ? MODE_COMPRESS : MODE_DECOMPRESS;
  guint count = g_list_model_get_n_items (files);
  for (guint i = 0; i < count; i++)
    {
      GFile *f = G_FILE (g_list_model_get_item (files, i));
      char *path = g_file_get_path (f);
      if (path)
	{
	  TaskItem task = {.mode = mode,.status = STATUS_PENDING };
	  g_strlcpy (task.data.paths.in_path, path, PATH_MAX);
	  char *base = g_path_get_basename (path);
	  snprintf (task.data.paths.out_path, PATH_MAX, "%s/%s.lz77",
		    g_get_home_dir (), base);
	  g_free (base);
	  queue_push (&a->queue, &task);
	  GtkWidget *row = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
	  gtk_box_append (GTK_BOX (row), gtk_label_new (path));
	  gtk_box_append (GTK_BOX (row), gtk_label_new ("Wait"));
	  gtk_list_box_insert (GTK_LIST_BOX (a->list_box), row, -1);
	  gtk_widget_set_visible (row, TRUE);
	  g_free (path);
	}
      g_object_unref (f);
    }
  atomic_store (&a->total, count);
  g_idle_add (ui_update, a);
  g_object_unref (files);
}

static void
on_add_files (GtkButton *btn, AppData *a)
{
  (void) btn;
  GtkWindow *win = GTK_WINDOW (gtk_widget_get_root (GTK_WIDGET (btn)));
  GtkFileDialog *dlg = gtk_file_dialog_new ();
  gtk_file_dialog_set_title (dlg, "Select files");
  gtk_file_dialog_open_multiple (dlg, win, NULL, on_picked_multiple, a);
}

static void
on_mode_toggle (GtkSwitch *sw, gboolean state, AppData *a)
{
  (void) sw;
  gtk_label_set_text (GTK_LABEL (a->mode_label),
		      state ? "Mode: deflate" : "Mode: decompress");
}

static gboolean
on_close (GtkWindow *w, AppData *a)
{
  (void) w;
  queue_shutdown (&a->queue);
  g_main_loop_quit ((GMainLoop *) a->loop);
  return FALSE;
}

int
main (void)
{
  gtk_init ();
  AppData app = { 0 };
  app.loop = g_main_loop_new (NULL, FALSE);
  queue_init (&app.queue);
  GtkWidget *win = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (win), "MZ77 Archiver");
  gtk_window_set_default_size (GTK_WINDOW (win), 440, 380);
  g_signal_connect (win, "close-request", G_CALLBACK (on_close), &app);
  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start (box, 15);
  gtk_widget_set_margin_end (box, 15);
  gtk_widget_set_margin_top (box, 15);
  gtk_widget_set_margin_bottom (box, 15);
  gtk_window_set_child (GTK_WINDOW (win), box);
  app.list_box = gtk_list_box_new ();
  gtk_widget_set_vexpand (app.list_box, TRUE);
  gtk_box_append (GTK_BOX (box), app.list_box);
  app.mode_label = gtk_label_new ("Mode: decompress");
  app.mode_switch = gtk_switch_new ();
  GtkWidget *mr = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 8);
  gtk_box_append (GTK_BOX (mr), app.mode_label);
  gtk_box_append (GTK_BOX (mr), app.mode_switch);
  gtk_widget_set_halign (app.mode_label, GTK_ALIGN_START);
  g_signal_connect (app.mode_switch, "state-set", G_CALLBACK (on_mode_toggle),
		    &app);
  GtkWidget *add_btn = gtk_button_new_with_label ("Add files");
  g_signal_connect (add_btn, "clicked", G_CALLBACK (on_add_files), &app);
  app.progress = gtk_progress_bar_new ();
  app.status = gtk_label_new ("Waiting for files");
  app.start_btn = gtk_button_new_with_label ("Start");
  g_signal_connect (app.start_btn, "clicked", G_CALLBACK (on_start), &app);
  gtk_box_append (GTK_BOX (box), mr);
  gtk_box_append (GTK_BOX (box), add_btn);
  gtk_box_append (GTK_BOX (box), app.progress);
  gtk_box_append (GTK_BOX (box), app.status);
  gtk_box_append (GTK_BOX (box), app.start_btn);
  gtk_window_present (GTK_WINDOW (win));
  g_main_loop_run (app.loop);
  g_main_loop_unref (app.loop);
  queue_destroy (&app.queue);
  return EXIT_SUCCESS;
}
