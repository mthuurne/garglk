/* TODO: add mouse down event */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include "glk.h"
#include "garglk.h"

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

static GtkWidget *frame;
static GtkWidget *canvas;
static GtkWidget *filedlog;
static char *filename;

static int timerid = -1;
static int timeouts = 0;

static int timeout(void *data)
{
	timeouts ++;
	return TRUE;
}

void glk_request_timer_events(glui32 millisecs)
{
	if (timerid != -1)
	{
		gtk_timeout_remove(timerid);
		timerid = -1;
	}

	if (millisecs)
	{
		timerid = gtk_timeout_add(millisecs, timeout, NULL);
	}
}

void winabort(const char *fmt, ...)
{
	va_list ap;
	char buf[256];
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	va_end(ap);
	// XXX MessageBoxA(NULL, buf, "Fatal error", MB_ICONERROR);
	fprintf(stderr, "fatal: %s\n", buf);
	fflush(stderr);
	abort();
}

static void onokay(GtkFileSelection *widget, void *data)
{
	strcpy(filename, gtk_file_selection_get_filename(GTK_FILE_SELECTION(filedlog)));
	gtk_widget_destroy(filedlog);
	filedlog = NULL;
	gtk_main_quit(); /* un-recurse back to normal loop */
}

static void oncancel(GtkFileSelection *widget, void *data)
{
	strcpy(filename, "");
	gtk_widget_destroy(filedlog);
	filedlog = NULL;
	gtk_main_quit(); /* un-recurse back to normal loop */
}

void winopenfile(char *prompt, char *buf, int len)
{
	char realprompt[256];
	sprintf(realprompt, "Open: %s", prompt);
	filedlog = gtk_file_selection_new(realprompt);
	if (strlen(buf))
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filedlog), buf);
	gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(filedlog));
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedlog)->ok_button),
		"clicked", onokay, NULL);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedlog)->cancel_button),
		"clicked", oncancel, NULL);
	filename = buf;
	gtk_widget_show(filedlog);
	gtk_main(); /* recurse... */
}

void winsavefile(char *prompt, char *buf, int len)
{
	char realprompt[256];
	sprintf(realprompt, "Save: %s", prompt);
	filedlog = gtk_file_selection_new(realprompt);
	if (strlen(buf))
		gtk_file_selection_set_filename(GTK_FILE_SELECTION(filedlog), buf);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedlog)->ok_button),
		"clicked", onokay, NULL);
	gtk_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(filedlog)->cancel_button),
		"clicked", oncancel, NULL);
	filename = buf;
	gtk_widget_show(filedlog);
	gtk_main(); /* recurse... */
}

static void onresize(GtkWidget *widget, GtkAllocation *event, void *data)
{
	int newwid = event->width;
	int newhgt = event->height;

	if (newwid == gli_image_w && newhgt == gli_image_h)
		return;

	gli_image_w = newwid;
	gli_image_h = newhgt;

	gli_image_s = ((gli_image_w * 3 + 3) / 4) * 4;
	if (gli_image_rgb)
		free(gli_image_rgb);
	gli_image_rgb = malloc(gli_image_s * gli_image_h);

	gli_force_redraw = 1;

	gli_windows_size_change();
}

static void onexpose(GtkWidget *widget, GdkEventExpose *event, void *data)
{
	int x0 = event->area.x;
	int y0 = event->area.y;
	int w = event->area.width;
	int h = event->area.height;

	if (x0 < 0) x0 = 0;
	if (y0 < 0) y0 = 0;
	if (x0 + w > gli_image_w) w = gli_image_w - x0;
	if (y0 + h > gli_image_h) h = gli_image_h - y0;
	if (w < 0) return;
	if (h < 0) return;

	gli_windows_redraw();

	gdk_draw_rgb_image(canvas->window, canvas->style->black_gc,
		x0, y0, w, h,
		GDK_RGB_DITHER_NONE,
		gli_image_rgb + y0 * gli_image_s + x0 * 3,
		gli_image_s);
}

static void onbutton(GtkWidget *widget, GdkEventButton *event, void *data)
{
	gli_input_handle_click(event->x, event->y);
}

static void onkeypress(GtkWidget *widget, GdkEventKey *event, void *data)
{
	int key = event->keyval;

	switch (key)
	{
	case GDK_Return: gli_input_handle_key(keycode_Return); break;
	case GDK_BackSpace: gli_input_handle_key(keycode_Delete); break;
	case GDK_Tab: gli_input_handle_key(keycode_Tab); break;
	case GDK_Prior: gli_input_handle_key(keycode_PageUp); break;
	case GDK_Next: gli_input_handle_key(keycode_PageDown); break;
	case GDK_Home: gli_input_handle_key(keycode_Home); break;
	case GDK_End: gli_input_handle_key(keycode_End); break;
	case GDK_Left: gli_input_handle_key(keycode_Left); break;
	case GDK_Right: gli_input_handle_key(keycode_Right); break;
	case GDK_Up: gli_input_handle_key(keycode_Up); break;
	case GDK_Down: gli_input_handle_key(keycode_Down); break;
	case GDK_Escape: gli_input_handle_key(keycode_Escape); break;
	case GDK_F1: gli_input_handle_key(keycode_Func1); break;
	case GDK_F2: gli_input_handle_key(keycode_Func2); break;
	case GDK_F3: gli_input_handle_key(keycode_Func3); break;
	case GDK_F4: gli_input_handle_key(keycode_Func4); break;
	case GDK_F5: gli_input_handle_key(keycode_Func5); break;
	case GDK_F6: gli_input_handle_key(keycode_Func6); break;
	case GDK_F7: gli_input_handle_key(keycode_Func7); break;
	case GDK_F8: gli_input_handle_key(keycode_Func8); break;
	case GDK_F9: gli_input_handle_key(keycode_Func9); break;
	case GDK_F10: gli_input_handle_key(keycode_Func10); break;
	case GDK_F11: gli_input_handle_key(keycode_Func11); break;
	case GDK_F12: gli_input_handle_key(keycode_Func12); break;
	default:
		if (key >= 32 && key <= 255)
			gli_input_handle_key(key);
	}
}

static void onquit(GtkWidget *widget, void *data)
{
	/* forced exit by wm */
	exit(0);
}

void wininit(int *argc, char **argv)
{
	gtk_init(argc, &argv);
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());
}

void winopen(void)
{
	GdkGeometry geom;
	int defw;
	int defh;

	geom.min_width  = gli_wmarginx * 2 + gli_cellw * 0;
	geom.min_height = gli_wmarginy * 2 + gli_cellh * 0;
	geom.max_width  = gli_wmarginx * 2 + gli_cellw * 255;
	geom.max_height = gli_wmarginy * 2 + gli_cellh * 250;
	geom.width_inc = gli_cellw;
	geom.height_inc = gli_cellh;

	defw = gli_wmarginx * 2 + gli_cellw * gli_cols;
	defh = gli_wmarginy * 2 + gli_cellh * gli_rows;

	frame = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	GTK_WIDGET_SET_FLAGS(frame, GTK_CAN_FOCUS);
	gtk_widget_set_events(frame, GDK_BUTTON_PRESS_MASK);
	gtk_signal_connect(GTK_OBJECT(frame), "button_press_event", onbutton, NULL);
	gtk_signal_connect(GTK_OBJECT(frame), "key_press_event", onkeypress, NULL);
	gtk_signal_connect(GTK_OBJECT(frame), "destroy", onquit, "WM destroy");

	canvas = gtk_drawing_area_new();
	gtk_signal_connect(GTK_OBJECT(canvas), "size_allocate", onresize, NULL);
	gtk_signal_connect(GTK_OBJECT(canvas), "expose_event", onexpose, NULL);
	gtk_container_add(GTK_CONTAINER(frame), canvas);

	wintitle();

	gtk_window_set_geometry_hints(GTK_WINDOW(frame),
		GTK_WIDGET(frame), &geom,
		GDK_HINT_MIN_SIZE
		| GDK_HINT_MAX_SIZE
		/* | GDK_HINT_RESIZE_INC */
		);
	gtk_window_set_default_size(GTK_WINDOW(frame), defw, defh);

	gtk_widget_show(canvas);
	gtk_widget_show(frame);

	gtk_widget_grab_focus(frame);
}

void wintitle(void)
{
	char buf[256];
	if (strlen(gli_story_name))
		sprintf(buf, "%s - %s", gli_program_name, gli_story_name);
	else
		sprintf(buf, "%s", gli_program_name);
	gtk_window_set_title(GTK_WINDOW(frame), buf);
}

void winrepaint(int x0, int y0, int x1, int y1)
{
	/* and pray that gtk+ is smart enough to coalesce... */
	gtk_widget_queue_draw_area(canvas, x0, y0, x1-x0, y1-y0);
}

void gli_select(event_t *event, int block)
{
    gli_curevent = event;
    gli_event_clearevent(event);

    gli_input_guess_focus();

    if (block)
    {
	while (gli_curevent->type == evtype_None && !timeouts)
	    gtk_main_iteration();
    }

    else
    {
	while (gtk_events_pending() && !timeouts)
	    gtk_main_iteration();
    }

    if (gli_curevent->type == evtype_None && timeouts)
    {
	gli_event_store(evtype_Timer, NULL, 0, 0);
	timeouts = 0;
    }

    gli_curevent = NULL;
}

