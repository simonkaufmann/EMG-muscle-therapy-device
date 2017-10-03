#include "project.h"
#include <gtk/gtk.h>

#define TOP_SPACE		50
#define BOTTOM_SPACE	10
#define LEFT_SPACE		20
#define RIGHT_SPACE		20

#define CMD_YES_WIDTH	((SCREENX-LEFT_SPACE-RIGHT_SPACE)/2.5)
#define CMD_YES_HEIGHT	(SCREENY/5)

#define CMD_NO_WIDTH	CMD_YES_WIDTH
#define CMD_NO_HEIGHT	CMD_YES_HEIGHT

#define CMD_YES_POSX	LEFT_SPACE
#define CMD_YES_POSY	SCREENY/2

#define CMD_NO_POSX		CMD_YES_POSX+CMD_YES_WIDTH+CMD_YES_WIDTH/2
#define CMD_NO_POSY		CMD_YES_POSY

#define LBL_SHUTDOWN_POSX	SCREENX/4+15
#define LBL_SHUTDOWN_POSY	TOP_SPACE

#define LBL_IS_SHUTING_DOWN_POSX	SCREENX/6.5
#define LBL_IS_SHUTING_DOWN_POSY	SCREENY/2-20


/* function declaration */

static void cmd_yes_clicked(GtkWidget*, gpointer);
static void cmd_no_clicked(GtkWidget*, gpointer);

/* widget declaration */

GtkWidget* window_shutdown;
GtkWidget* fixed_shutdown;
GtkWidget* cmd_yes;
GtkWidget* cmd_no;
GtkWidget* lbl_shutdown;
GtkWidget* lbl_is_shuting_down;

void window_shutdown_init()	{
	
	window_shutdown = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_shutdown), SCREENX, SCREENY);

	fixed_shutdown = gtk_fixed_new();

	cmd_yes = gtk_button_new_with_label("Yes");
	g_signal_connect(G_OBJECT(cmd_yes), "clicked", G_CALLBACK(cmd_yes_clicked), NULL);

	cmd_no = gtk_button_new_with_label("No");
	g_signal_connect(G_OBJECT(cmd_no), "clicked", G_CALLBACK(cmd_no_clicked), NULL);

	lbl_shutdown = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(lbl_shutdown), "<span font=\'Liberation Sans 22\'>Shutdown?</span>");

	lbl_is_shuting_down = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(lbl_is_shuting_down), "<span font=\'Liberation Sans 22\'>Is shutting down...</span>");

	gtk_widget_set_size_request(cmd_yes, CMD_YES_WIDTH, CMD_YES_HEIGHT);
	gtk_widget_set_size_request(cmd_no, CMD_NO_WIDTH, CMD_NO_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_shutdown), cmd_yes, CMD_YES_POSX, CMD_YES_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_shutdown), cmd_no, CMD_NO_POSX, CMD_NO_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_shutdown), lbl_shutdown, LBL_SHUTDOWN_POSX, LBL_SHUTDOWN_POSY);

	gtk_container_add(GTK_CONTAINER(window_shutdown), fixed_shutdown);
}

void window_shutdown_show()	{
	gtk_widget_show_all(window_shutdown);
}

void window_shutdown_hide()	{
	gtk_widget_hide_all(window_shutdown);
}

static void cmd_yes_clicked(GtkWidget* widget, gpointer data)	{
	gtk_widget_hide(cmd_yes);
	gtk_widget_hide(cmd_no);
	gtk_widget_hide(lbl_shutdown);

	gtk_fixed_put(GTK_FIXED(fixed_shutdown), lbl_is_shuting_down, LBL_IS_SHUTING_DOWN_POSX, LBL_IS_SHUTING_DOWN_POSY);
	gtk_widget_show(lbl_is_shuting_down);

	system("sudo shutdown -h now");
}

static void cmd_no_clicked(GtkWidget* widget, gpointer data)	{

	window_shutdown_hide();
	window_main_show();
}

