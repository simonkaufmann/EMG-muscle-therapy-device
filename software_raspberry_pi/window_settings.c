#include <gtk/gtk.h>
#include "project.h"

#define TOP_SPACE 		10
#define BOTTOM_SPACE 	10
#define LEFT_SPACE		10
#define RIGHT_SPACE 	10

#define WIDGET_SPACE 	6
#define BUTTON_SPACE 	WIDGET_SPACE

#define NUMBER_BUTTONS	4
#define BUTTON_HEIGHT	(SCREENY-TOP_SPACE-BOTTOM_SPACE-LBL_SETTINGS_HEIGHT-WIDGET_SPACE-CMD_BACK_HEIGHT)/NUMBER_BUTTONS-4*BUTTON_SPACE/NUMBER_BUTTONS /* 4 is the real number_buttons ?  (because NUMBER_BUTTONS is just a value to calculate some division for the buttons) */
#define BUTTON_WIDTH	(SCREENX-LEFT_SPACE-RIGHT_SPACE)

#define LBL_SETTINGS_HEIGHT 33 // find it out by trying out!
#define LBL_SETTINGS_WIDTH  no_value // don't really know the exact value

#define CMD_REFERENCE_SIGNAL_HEIGHT 	2*BUTTON_HEIGHT//+BUTTON_SPACE
#define CMD_REFERENCE_SIGNAL_WIDTH		BUTTON_WIDTH/2-BUTTON_SPACE

#define CMD_DIFFICULTY_HEIGHT			2*BUTTON_HEIGHT//+BUTTON_SPACE
#define CMD_DIFFICULTY_WIDTH			BUTTON_WIDTH/2-BUTTON_SPACE

#define CMD_CALIBRATION_HEIGHT			2*BUTTON_HEIGHT//+BUTTON_SPACE /* calibration instead of sensitivity -> sensitivity is now at difficulty! */
#define CMD_CALIBRATION_WIDTH			BUTTON_WIDTH/2-BUTTON_SPACE

#define CMD_MUSIC_HEIGHT				2*BUTTON_HEIGHT//+BUTTON_SPACE
#define CMD_MUSIC_WIDTH					BUTTON_WIDTH/2-BUTTON_SPACE

//#define CMD_BACK_HEIGHT					1.5*BUTTON_HEIGHT
//#define CMD_BACK_WIDTH					BUTTON_WIDTH/4
//
//#define CMD_HOME_HEIGHT					1.5*BUTTON_HEIGHT
//#define CMD_HOME_WIDTH					BUTTON_WIDTH/4

#define LBL_SETTINGS_POSX 			LEFT_SPACE
#define LBL_SETTINGS_POSY 			TOP_SPACE

#define CMD_REFERENCE_SIGNAL_POSX 	LEFT_SPACE
#define CMD_REFERENCE_SIGNAL_POSY 	LBL_SETTINGS_POSY+LBL_SETTINGS_HEIGHT+WIDGET_SPACE

#define CMD_DIFFICULTY_POSX			LEFT_SPACE+BUTTON_WIDTH/2
#define CMD_DIFFICULTY_POSY			CMD_REFERENCE_SIGNAL_POSY//CMD_REFERENCE_SIGNAL_POSY+CMD_REFERENCE_SIGNAL_HEIGHT+BUTTON_SPACE

#define CMD_CALIBRATION_POSX		LEFT_SPACE+BUTTON_WIDTH/2
#define CMD_CALIBRATION_POSY		CMD_DIFFICULTY_POSY+CMD_DIFFICULTY_HEIGHT+BUTTON_SPACE

#define CMD_MUSIC_POSX				LEFT_SPACE
#define CMD_MUSIC_POSY				CMD_CALIBRATION_POSY//CMD_CALIBRATION_POSY+CMD_CALIBRATION_HEIGHT+BUTTON_SPACE

//#define CMD_BACK_POSX				LEFT_SPACE
//#define CMD_BACK_POSY				CMD_MUSIC_POSY+CMD_MUSIC_HEIGHT+BUTTON_SPACE
//
//#define CMD_HOME_POSX				SCREENX/2-CMD_HOME_WIDTH/2//LEFT_SPACE+CMD_BACK_WIDTH+BUTTON_SPACE
//#define CMD_HOME_POSY				CMD_BACK_POSY

// function declarations
static void cmd_back_clicked(GtkWidget*, gpointer);
static void cmd_home_clicked(GtkWidget*, gpointer);
static void cmd_reference_signal_clicked(GtkWidget*, gpointer);
static void cmd_difficulty_clicked(GtkWidget*, gpointer);
static void cmd_calibration_clicked(GtkWidget*, gpointer);
static void cmd_music_clicked(GtkWidget*, gpointer);

// variable declarations
GtkWidget* window_settings;
GtkWidget* fixed_settings;
GtkWidget* lbl_settings;
GtkWidget* cmd_back;
GtkWidget* cmd_home;
GtkWidget* cmd_reference_signal;
GtkWidget* cmd_music;
GtkWidget* cmd_calibration;
GtkWidget* cmd_difficulty;

void window_settings_init()	{

//	g_print("BACK-height:%d\n", (int)(CMD_BACK_HEIGHT));
//	g_print("BACK-positionx:%d\n", CMD_BACK_POSX);
//	g_print("BACK-positiony:%d\n", (int)(CMD_BACK_POSY));
//	g_print("HOME-positionx: %d\n", CMD_HOME_POSX);
//	g_print("HOME-positiony: %d\n", CMD_HOME_POSY);
//	g_print("BACK-size: %d\n", CMD_BACK_WIDTH);
//	g_print("HOME-size: %d\n", CMD_HOME_WIDTH);
	
	window_settings = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_settings), SCREENX, SCREENY);

	fixed_settings = gtk_fixed_new();

	lbl_settings = gtk_label_new("Settings");
	gtk_label_set_markup(GTK_LABEL(lbl_settings), "<span font=\'Liberation Sans 22\'>Settings</span>");

	cmd_reference_signal = gtk_button_new_with_label("Reference Signal");
	g_signal_connect(G_OBJECT(cmd_reference_signal), "clicked", G_CALLBACK(cmd_reference_signal_clicked), NULL);

	cmd_difficulty = gtk_button_new_with_label("Difficulty");
	g_signal_connect(G_OBJECT(cmd_difficulty), "clicked", G_CALLBACK(cmd_difficulty_clicked), NULL);

	cmd_calibration = gtk_button_new_with_label("Calibration");
	g_signal_connect(G_OBJECT(cmd_calibration), "clicked", G_CALLBACK(cmd_calibration_clicked), NULL);

	cmd_music = gtk_button_new_with_label("Music");
	g_signal_connect(G_OBJECT(cmd_music), "clicked", G_CALLBACK(cmd_music_clicked), NULL);

	cmd_back = gtk_button_new_with_label("Back");
	g_signal_connect(G_OBJECT(cmd_back), "clicked", G_CALLBACK(cmd_back_clicked), NULL);

	cmd_home = gtk_button_new_with_label("Home");
	g_signal_connect(G_OBJECT(cmd_home), "clicked", G_CALLBACK(cmd_home_clicked), NULL);

	gtk_widget_set_size_request(cmd_reference_signal, CMD_REFERENCE_SIGNAL_WIDTH, CMD_REFERENCE_SIGNAL_HEIGHT);
	gtk_widget_set_size_request(cmd_difficulty, CMD_DIFFICULTY_WIDTH, CMD_DIFFICULTY_HEIGHT);
	gtk_widget_set_size_request(cmd_calibration, CMD_CALIBRATION_WIDTH, CMD_CALIBRATION_HEIGHT);
	gtk_widget_set_size_request(cmd_music, CMD_MUSIC_WIDTH, CMD_MUSIC_HEIGHT);
	gtk_widget_set_size_request(cmd_back, CMD_BACK_WIDTH, CMD_BACK_HEIGHT);
	gtk_widget_set_size_request(cmd_home, CMD_HOME_WIDTH, CMD_HOME_HEIGHT);

	//gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_back, 10, 10);	
	gtk_fixed_put(GTK_FIXED(fixed_settings), lbl_settings, LBL_SETTINGS_POSX, LBL_SETTINGS_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_reference_signal, CMD_REFERENCE_SIGNAL_POSX, CMD_REFERENCE_SIGNAL_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_difficulty, CMD_DIFFICULTY_POSX, CMD_DIFFICULTY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_calibration, CMD_CALIBRATION_POSX, CMD_CALIBRATION_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_music, CMD_MUSIC_POSX, CMD_MUSIC_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_back, CMD_BACK_POSX, CMD_BACK_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_settings), cmd_home, CMD_HOME_POSX, CMD_HOME_POSY);

	gtk_container_add(GTK_CONTAINER(window_settings), fixed_settings);
}

void window_settings_show()	{
	gtk_widget_show_all(window_settings);
}

void window_settings_hide()	{
	gtk_widget_hide(window_settings);
}

static void cmd_back_clicked(GtkWidget* widget, gpointer data)	{
	window_settings_hide();
	window_main_show();
}

static void cmd_home_clicked(GtkWidget* widget, gpointer data)	{
	window_settings_hide();
	window_main_show();
}

static void cmd_reference_signal_clicked(GtkWidget* widget, gpointer data)	{
	/* only when going back to an old window -> then hide own window! */
	//window_settings_hide();
	window_reference_signal_hide();
	window_reference_signal_show();
}

static void cmd_difficulty_clicked(GtkWidget* widget, gpointer data)	{
	//window_settings_hide();
	window_difficulty_hide();
	window_difficulty_show();
}

static void cmd_calibration_clicked(GtkWidget* widget, gpointer data)	{
	window_calibration_hide();
	window_calibration_show();
	//window_signal_init(); /* why has this function to be called here again? Otherwise there won't be received correct uart values... but why? */
	window_signal_uart_start();
}

static void cmd_music_clicked(GtkWidget* widget, gpointer data)	{
	window_music_hide();
	window_music_show();
}
