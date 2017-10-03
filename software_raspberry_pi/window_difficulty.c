#include <gtk/gtk.h>
#include "project.h"

#define TOP_SPACE 		10
#define BOTTOM_SPACE 	20
#define LEFT_SPACE		10
#define RIGHT_SPACE 	10

#define WIDGET_SPACE 	6
#define BUTTON_SPACE 	WIDGET_SPACE

#define NUMBER_BUTTONS	4
#define BUTTON_HEIGHT	(SCREENY-TOP_SPACE-BOTTOM_SPACE-LBL_DIFFICULTY_HEIGHT-WIDGET_SPACE-CMD_BACK_HEIGHT-10)/NUMBER_BUTTONS-4*BUTTON_SPACE/NUMBER_BUTTONS /* 4 is the real number_buttons ?  (because NUMBER_BUTTONS is just a value to calculate some division for the buttons) */  /* -10 for optical reasons so that cmd_back and hscroll_duty is not so nearby!) */

/* NOTE: the word DIFFICULTY has changed to SENSITIVITY
 *       the word SENSITIVITY has changed to Audio tolerance
 */
#define BUTTON_WIDTH	(SCREENX-LEFT_SPACE-RIGHT_SPACE)

#define LBL_DIFFICULTY_HEIGHT 28 // find it out by trying out!
#define LBL_DIFFICULTY_WIDTH  no_value // don't really know the exact value

#define LBL_DIFFICULTY_POSX		LEFT_SPACE
#define LBL_DIFFICULTY_POSY		TOP_SPACE

#define DRAW_AREA_DIFFICULTY_HEIGHT 	2*BUTTON_HEIGHT+BUTTON_SPACE
#define DRAW_AREA_DIFFICULTY_WIDTH	BUTTON_WIDTH

#define HSCROLL_DIFFICULTY_HEIGHT BUTTON_HEIGHT
#define HSCROLL_DIFFICULTY_WIDTH	BUTTON_WIDTH-LBL_HSCROLL_DIFFICULTY_WIDTH

#define HSCROLL_SENSITIVITY_HEIGHT	BUTTON_HEIGHT
#define HSCROLL_SENSITIVITY_WIDTH	BUTTON_WIDTH-LBL_HSCROLL_SENSITIVITY_WIDTH

#define LBL_HSCROLL_DIFFICULTY_WIDTH 	SCREENX/4
#define LBL_HSCROLL_SENSITIVITY_WIDTH		SCREENX/4

#define LBL_DIFFICULTY_POSX	LEFT_SPACE
#define LBL_DIFFICULTY_POSY	TOP_SPACE

#define DRAW_AREA_DIFFICULTY_POSX	LEFT_SPACE
#define DRAW_AREA_DIFFICULTY_POSY	TOP_SPACE+LBL_DIFFICULTY_POSY+LBL_DIFFICULTY_HEIGHT+WIDGET_SPACE

#define LBL_HSCROLL_DIFFICULTY_POSX		LEFT_SPACE
#define LBL_HSCROLL_DIFFICULTY_POSY		DRAW_AREA_DIFFICULTY_POSY+DRAW_AREA_DIFFICULTY_HEIGHT+BUTTON_SPACE+4

#define HSCROLL_DIFFICULTY_POSX	LBL_HSCROLL_DIFFICULTY_POSX+LBL_HSCROLL_DIFFICULTY_WIDTH
#define HSCROLL_DIFFICULTY_POSY	LBL_HSCROLL_DIFFICULTY_POSY-4

#define LBL_HSCROLL_SENSITIVITY_POSX		LEFT_SPACE
#define LBL_HSCROLL_SENSITIVITY_POSY		HSCROLL_DIFFICULTY_POSY+HSCROLL_DIFFICULTY_HEIGHT+BUTTON_SPACE+4

#define HSCROLL_SENSITIVITY_POSX	LBL_HSCROLL_SENSITIVITY_POSX+LBL_HSCROLL_SENSITIVITY_WIDTH
#define HSCROLL_SENSITIVITY_POSY	LBL_HSCROLL_SENSITIVITY_POSY-4


#define HSCROLL_SENSITIVITY_MIN_VALUE 	0
#define HSCROLL_SENSITIVITY_MAX_VALUE	SENSITIVITY_MAX

#define HSCROLL_DIFFICULTY_MIN_VALUE	0
#define HSCROLL_DIFFICULTY_MAX_VALUE	DIFFICULTY_MAX

#define LBL_EXPLANATION_POSX DRAW_AREA_DIFFICULTY_POSX
#define LBL_EXPLANATION_POSY DRAW_AREA_DIFFICULTY_POSY

/* function definitions */
static void cmd_home_clicked(GtkWidget*, gpointer);
static void cmd_back_clicked(GtkWidget*, gpointer);
static void draw_area_difficulty_expose_event(GtkWidget*, GdkEventExpose*, gpointer);
static void hscroll_difficulty_value_changed(GtkRange*, gpointer);
static void hscroll_sensitivity_value_changed(GtkRange*, gpointer);

/* widget declarations */
GtkWidget* window_difficulty;
GtkWidget* fixed_difficulty;
GtkWidget* lbl_difficulty;

GtkWidget* draw_area_difficulty;
GdkPixmap* pixmap_difficulty;
GtkWidget* lbl_hscroll_difficulty;
GtkWidget* hscroll_sensitivity;
GtkWidget* lbl_hscroll_sensitivity;
GtkWidget* hscroll_difficulty;
GtkWidget* lbl_explanation;
GtkObject* adj_sensitivity;
GtkObject* adj_difficulty;

GtkWidget* cmd_back;
GtkWidget* cmd_home;

void window_difficulty_init(void)	{

	window_difficulty = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_difficulty), SCREENX, SCREENY);

	fixed_difficulty = gtk_fixed_new();

	lbl_difficulty = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(lbl_difficulty), "<span font=\'Liberation Sans 22\'>Difficulty</span>");

	cmd_back = gtk_button_new_with_label("Back");
	g_signal_connect(G_OBJECT(cmd_back), "clicked", G_CALLBACK(cmd_back_clicked), NULL);

	cmd_home = gtk_button_new_with_label("Home");
	g_signal_connect(G_OBJECT(cmd_home), "clicked", G_CALLBACK(cmd_home_clicked), NULL);

	draw_area_difficulty = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (draw_area_difficulty), "expose_event",
                    G_CALLBACK (draw_area_difficulty_expose_event), NULL);

	
	adj_sensitivity = gtk_adjustment_new(5, HSCROLL_SENSITIVITY_MIN_VALUE, HSCROLL_SENSITIVITY_MAX_VALUE, 1, 1, 0);

	hscroll_sensitivity = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj_sensitivity));
	g_signal_connect(G_OBJECT(hscroll_sensitivity), "value-changed", G_CALLBACK(hscroll_sensitivity_value_changed), NULL);

	lbl_hscroll_sensitivity = gtk_label_new("Tolerance");

	adj_difficulty = gtk_adjustment_new(5, HSCROLL_DIFFICULTY_MIN_VALUE, HSCROLL_DIFFICULTY_MAX_VALUE, 1, 1, 0);

	hscroll_difficulty = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj_difficulty));
	g_signal_connect(G_OBJECT(hscroll_difficulty), "value-changed", G_CALLBACK(hscroll_difficulty_value_changed), NULL);

	lbl_hscroll_difficulty = gtk_label_new("Difficulty");

	lbl_explanation = gtk_label_new("Difficulty:   Set how strong the muscle \n                  has to be contracted\nTolerance: Set the tolerance of the audio\n                  feedback");



	gtk_widget_set_size_request(hscroll_difficulty, HSCROLL_DIFFICULTY_WIDTH, HSCROLL_DIFFICULTY_HEIGHT);
	gtk_widget_set_size_request(hscroll_sensitivity, HSCROLL_SENSITIVITY_WIDTH, HSCROLL_SENSITIVITY_HEIGHT);
	gtk_widget_set_size_request(cmd_home, CMD_HOME_WIDTH, CMD_HOME_HEIGHT);
	gtk_widget_set_size_request(cmd_back, CMD_BACK_WIDTH, CMD_BACK_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_difficulty), lbl_difficulty, LBL_DIFFICULTY_POSX, LBL_DIFFICULTY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), cmd_home, CMD_HOME_POSX, CMD_HOME_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), cmd_back, CMD_BACK_POSX, CMD_BACK_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), hscroll_difficulty, HSCROLL_DIFFICULTY_POSX, HSCROLL_DIFFICULTY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), hscroll_sensitivity, HSCROLL_SENSITIVITY_POSX, HSCROLL_SENSITIVITY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), lbl_hscroll_difficulty, LBL_HSCROLL_DIFFICULTY_POSX, LBL_HSCROLL_DIFFICULTY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), lbl_hscroll_sensitivity, LBL_HSCROLL_SENSITIVITY_POSX, LBL_HSCROLL_SENSITIVITY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_difficulty), lbl_explanation, LBL_EXPLANATION_POSX, LBL_EXPLANATION_POSY);

	gtk_container_add(GTK_CONTAINER(window_difficulty), fixed_difficulty);

}

void window_difficulty_show(void)	{
	gtk_widget_show_all(window_difficulty);
}

void window_difficulty_hide(void)	{
	gtk_widget_hide_all(window_difficulty);
}

static void cmd_home_clicked(GtkWidget* widget, gpointer data)	{
	window_main_show();
	window_settings_hide();
	window_difficulty_hide();
}

static void cmd_back_clicked(GtkWidget* widget, gpointer data)	{
	window_settings_show();
	window_difficulty_hide();
}

static void hscroll_difficulty_value_changed(GtkRange* range, gpointer data)	{
	int val = gtk_range_get_value(range);
	window_signal_set_difficulty(val);
}

static void hscroll_sensitivity_value_changed(GtkRange* range, gpointer data)	{
	int val = gtk_range_get_value(range);
	window_signal_set_sensitivity(val);
}

static void draw_area_difficulty_expose_event(GtkWidget* widget, GdkEventExpose* event, gpointer data)	{

}
