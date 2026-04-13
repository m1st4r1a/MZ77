#include <config.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>
#include <pthread.h>
#include <gtk/gtk.h>
#include <glib.h>
#include "dt.h"
#include "compress.h"
#include "decompress.h"

static gboolean
ui_update (gpointer d)
{
  AppData *a = d;
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (a->progress),
				 (double) atomic_load (&a->progress_pct) /
				 100.0);
  return G_SOURCE_REMOVE;
}

static void
set_state (AppData *a, gboolean run, const char *t)
{
  atomic_store (&a->busy, run);
  gtk_widget_set_sensitive (a->run_btn, !run);
  gtk_label_set_text (GTK_LABEL (a->status), t);
}

static gboolean
on_mode_toggle (GtkSwitch *sw, gboolean state, gpointer data)
{
  (void) sw;
  GtkLabel *lbl = GTK_LABEL (data);
  gtk_label_set_text (lbl, state ? "Mode: compress" : "Mode: decompress");
  return false;
}


/* ── Background Worker ── */
static void *
worker (void *d)
{
  AppData *a = d;
  // Check switch state to decide mode
  gboolean is_compress = gtk_switch_get_active (GTK_SWITCH (a->mode_switch));
  const char *action = is_compress ? "Compressing..." : "Decompressing...";
  set_state (a, TRUE, action);
  atomic_store (&a->progress_pct, 0);
  g_idle_add (ui_update, a);
  // Simulation Loop
  for (int i = 0; i <= 100; i += 5)
    {
      atomic_store (&a->progress_pct, i);
      g_idle_add (ui_update, a);
      g_usleep (20000);
    }
  // Execute correct function based on mode
  DWORD res;
  if (is_compress)
    res = compress (a->in_path, a->out_path);
  else
    res = decompress (a->in_path, a->out_path);
  atomic_store (&a->progress_pct, 100);
  g_idle_add (ui_update, a);
  set_state (a, FALSE, res == EXIT_SUCCESS ? "✅ Done" : "❌ Failed");
  return NULL;
}

static void
on_run (GtkButton *btn, AppData *a)
{
  (void) btn;
  if (atomic_load (&a->busy))
    return;
  if (!a->in_path[0] || !a->out_path[0])
    {
      set_state (a, FALSE, "⚠️ Select both paths");
      return;
    }
  pthread_t tid;
  pthread_create (&tid, NULL, worker, a);
  pthread_detach (tid);
}

static void
on_picked (GObject *src, GAsyncResult *res, gpointer d)
{
  PickerData *pd = d;
  if (!pd)
    return;

  GError *err = NULL;
  GFile *file = pd->is_input
    ? gtk_file_dialog_open_finish (GTK_FILE_DIALOG (src), res, &err)
    : gtk_file_dialog_save_finish (GTK_FILE_DIALOG (src), res, &err);
  if (file)
    {
      char *path = g_file_get_path (file);
      if (path)
	{
	  if (pd->is_input)
	    {
	      g_strlcpy (pd->app->in_path, path, PATH_MAX);
	      gtk_label_set_text (GTK_LABEL (pd->app->in_label), path);
	    }
	  else
	    {
	      g_strlcpy (pd->app->out_path, path, PATH_MAX);
	      gtk_label_set_text (GTK_LABEL (pd->app->out_label), path);
	    }
	  g_free (path);
	}
      g_object_unref (file);
    }
  else if (err)
    {
      g_printerr ("Dialog error: %s\n", err->message);
      g_error_free (err);
    }
  g_free (pd);
}

static void
on_pick (GtkButton *btn, PickerData *pd)
{
  (void) btn;
  GtkFileDialog *dlg = gtk_file_dialog_new ();
  if (pd->is_input)
    {
      gtk_file_dialog_set_title (dlg, "Select Input");
      gtk_file_dialog_open (dlg, pd->win, NULL, on_picked, pd);
    }
  else
    {
      gtk_file_dialog_set_title (dlg, "Select Output");
      gtk_file_dialog_set_initial_name (dlg, "output.lz77");
      gtk_file_dialog_save (dlg, pd->win, NULL, on_picked, pd);
    }
}

static gboolean
on_close (GtkWindow *w, gpointer d)
{
  (void) w;
  g_main_loop_quit ((GMainLoop *) d);
  return FALSE;
}

int
main (void)
{
  gtk_init ();
  AppData app = { 0 };
  GMainLoop *loop = g_main_loop_new (NULL, FALSE);

  GtkWidget *win = gtk_window_new ();
  gtk_window_set_title (GTK_WINDOW (win), "MZ77 Archiver");
  gtk_window_set_default_size (GTK_WINDOW (win), 400, 320);	// Taller for switch
  g_signal_connect (win, "close-request", G_CALLBACK (on_close), loop);

  GtkWidget *box = gtk_box_new (GTK_ORIENTATION_VERTICAL, 10);
  gtk_widget_set_margin_start (box, 15);
  gtk_widget_set_margin_end (box, 15);
  gtk_widget_set_margin_top (box, 15);
  gtk_widget_set_margin_bottom (box, 15);
  gtk_window_set_child (GTK_WINDOW (win), box);

  /* Input Row */
  app.in_label = gtk_label_new ("No input selected");
  GtkWidget *ib = gtk_button_new_with_label ("Pick Input");
  GtkWidget *ir = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append (GTK_BOX (ir), app.in_label);
  gtk_box_append (GTK_BOX (ir), ib);
  gtk_widget_set_hexpand (app.in_label, TRUE);

  PickerData *pi = g_new0 (PickerData, 1);
  pi->app = &app;
  pi->win = GTK_WINDOW (win);
  pi->is_input = TRUE;
  g_signal_connect (ib, "clicked", G_CALLBACK (on_pick), pi);

  /* Output Row */
  app.out_label = gtk_label_new ("No output selected");
  GtkWidget *ob = gtk_button_new_with_label ("Pick Output");
  GtkWidget *or = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_box_append (GTK_BOX (or), app.out_label);
  gtk_box_append (GTK_BOX (or), ob);
  gtk_widget_set_hexpand (app.out_label, TRUE);

  PickerData *po = g_new0 (PickerData, 1);
  po->app = &app;
  po->win = GTK_WINDOW (win);
  po->is_input = FALSE;
  g_signal_connect (ob, "clicked", G_CALLBACK (on_pick), po);

  /* Mode Switch Row */
  app.mode_label = gtk_label_new ("Mode: Decompress");	// Default label
  app.mode_switch = gtk_switch_new ();	// Default OFF (Decompress)
  GtkWidget *mr = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 5);
  gtk_widget_set_halign (app.mode_label, GTK_ALIGN_START);
  gtk_box_append (GTK_BOX (mr), app.mode_label);
  gtk_box_append (GTK_BOX (mr), app.mode_switch);

  // Update label when toggled
  g_signal_connect (app.mode_switch, "state-set", G_CALLBACK (on_mode_toggle),
		    app.mode_label);

  /* Controls */
  app.progress = gtk_progress_bar_new ();
  app.status = gtk_label_new ("Ready");
  app.run_btn = gtk_button_new_with_label ("Start");
  g_signal_connect (app.run_btn, "clicked", G_CALLBACK (on_run), &app);

  /* Assemble UI */
  gtk_box_append (GTK_BOX (box), ir);
  gtk_box_append (GTK_BOX (box), or);
  gtk_box_append (GTK_BOX (box), mr);	// Add Mode Row
  gtk_box_append (GTK_BOX (box), app.progress);
  gtk_box_append (GTK_BOX (box), app.status);
  gtk_box_append (GTK_BOX (box), app.run_btn);

  gtk_window_present (GTK_WINDOW (win));
  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  return EXIT_SUCCESS;
}
