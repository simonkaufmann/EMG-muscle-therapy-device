#include <gtk/gtk.h>
#include "project.h"
#include <stdlib.h>
#include <pthread.h> //thread programming

#define NUMBER_BUTTONS 4 //one button with size of two buttons and two other buttons

#define BUTTON_SPACE 4
#define IMAGE_SPACE 20
#define BUTTON_RIGHT_SPACE 10
#define IMAGE_TOP_SPACE 15
#define TOP_SPACE 10
#define BOTTOM_SPACE 10
#define LEFT_SPACE 5
#define RIGHT_SPACE 2

#define BUTTON_HEIGHT_WINDOW_MAIN (SCREENY-BOTTOM_SPACE-TOP_SPACE)/NUMBER_BUTTONS-BUTTON_SPACE
#define BUTTON_WIDTH_WINDOW_MAIN  SCREENX*5/8-LEFT_SPACE-RIGHT_SPACE

#define CMD_START_POSX 				LEFT_SPACE
//#define CMD_DIFFICULTY_POSX 		LEFT_SPACE
//#define CMD_REFERENCE_SIGNAL_POSX 	LEFT_SPACE
#define CMD_MUSIC_POSX 				LEFT_SPACE
#define CMD_SHUTDOWN_POSX			LEFT_SPACE

#define CMD_START_POSY				TOP_SPACE
//#define CMD_DIFFICULTY_POSY			CMD_START_POSY+BUTTON_SPACE+BUTTON_HEIGHT_WINDOW_MAIN
//#define CMD_REFERENCE_SIGNAL_POSY	CMD_DIFFICULTY_POSY+BUTTON_SPACE+BUTTON_HEIGHT_WINDOW_MAIN
#define CMD_MUSIC_POSY				CMD_START_POSY+2*BUTTON_SPACE+2*BUTTON_HEIGHT_WINDOW_MAIN
#define CMD_SHUTDOWN_POSY			CMD_MUSIC_POSY+BUTTON_SPACE+BUTTON_HEIGHT_WINDOW_MAIN

#define IMAGE_KINDERN_EINE_CHANCE_PIXELX		500 //get this information from the picture file!!
#define IMAGE_KINDERN_EINE_CHANCE_PIXELY		507 //same here 
#define IMAGE_KINDERN_EINE_CHANCE_PROPORTION 	IMAGE_KINDERN_EINE_CHANCE_PIXELY/IMAGE_KINDERN_EINE_CHANCE_PIXELX

#define IMAGE_HTLINN_PIXELX		741
#define IMAGE_HTLINN_PIXELY		743
#define IMAGE_HTLINN_PROPORTION	IMAGE_HTLINN_PIXELY/IMAGE_HTLINN_PIXELX

#define IMAGE_KINDERN_EINE_CHANCE_WIDTH	SCREENX-BUTTON_WIDTH_WINDOW_MAIN-LEFT_SPACE-RIGHT_SPACE-BUTTON_RIGHT_SPACE
#define IMAGE_HTLINN_WIDTH				IMAGE_KINDERN_EINE_CHANCE_WIDTH

#define IMAGE_KINDERN_EINE_CHANCE_HEIGHT 	IMAGE_KINDERN_EINE_CHANCE_PROPORTION*IMAGE_KINDERN_EINE_CHANCE_WIDTH
#define IMAGE_HTLINN_HEIGHT					IMAGE_HTLINN_PROPORTION*IMAGE_HTLINN_WIDTH

#define IMAGE_KINDERN_EINE_CHANCE_POSX	LEFT_SPACE+BUTTON_WIDTH_WINDOW_MAIN+BUTTON_RIGHT_SPACE
#define IMAGE_HTLINN_POSX				IMAGE_KINDERN_EINE_CHANCE_POSX

#define IMAGE_KINDERN_EINE_CHANCE_POSY 	IMAGE_HTLINN_POSY+IMAGE_HTLINN_HEIGHT+IMAGE_SPACE
#define IMAGE_HTLINN_POSY				IMAGE_TOP_SPACE

// function declarations
static void cmd_settings_clicked(GtkWidget*, gpointer);
static void cmd_start_clicked(GtkWidget*, gpointer);
static void cmd_shutdown_clicked(GtkWidget*, gpointer);
static void window_main_destroy(GtkWidget*, gpointer);

// variable declarations
GtkWidget* window_main;
GtkWidget* fixed_main;

GtkWidget* cmd_start;
GtkWidget* cmd_settings;
//GtkWidget* cmd_reference_signal;
//GtkWidget* cmd_difficulty;
GtkWidget* cmd_shutdown;

GtkWidget* image_kindern_eine_chance;
GtkWidget* image_htlinn;

void window_main_init()	{
	window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_main), SCREENX, SCREENY);
	g_signal_connect(G_OBJECT(window_main), "destroy", G_CALLBACK(window_main_destroy), NULL);

	fixed_main = gtk_fixed_new();

	cmd_start = gtk_button_new_with_label("Start");
	g_signal_connect(G_OBJECT(cmd_start), "clicked", G_CALLBACK(cmd_start_clicked), NULL);

	//cmd_difficulty = gtk_button_new_with_label("Difficulty");

	//cmd_reference_signal = gtk_button_new_with_label("Reference Signal");
	
	cmd_settings = gtk_button_new_with_label("Settings");
	g_signal_connect(G_OBJECT(cmd_settings), "clicked", G_CALLBACK(cmd_settings_clicked), NULL);

	cmd_shutdown = gtk_button_new_with_label("Shutdown");
	g_signal_connect(G_OBJECT(cmd_shutdown), "clicked", G_CALLBACK(cmd_shutdown_clicked), NULL);

	char convert[1000];
	snprintf(convert, 1000, "convert -verbose /home/pi/gtk_drawing/kindern_eine_chance_orig.png -scale %dx%d! /home/pi/gtk_drawing/kindern_eine_chance.png", IMAGE_KINDERN_EINE_CHANCE_WIDTH, IMAGE_KINDERN_EINE_CHANCE_HEIGHT);
	g_print("system: %s", convert);
	system(convert);
	snprintf(convert, 1000, "convert -verbose /home/pi/gtk_drawing/logo_htlinn_orig.png -scale %dx%d! /home/pi/gtk_drawing/logo_htlinn.png", IMAGE_HTLINN_WIDTH, IMAGE_HTLINN_HEIGHT);
	g_print("sysetm: %s", convert);
	system(convert);

	image_kindern_eine_chance = gtk_image_new_from_file("/home/pi/gtk_drawing/kindern_eine_chance.png");
	image_htlinn = gtk_image_new_from_file("/home/pi/gtk_drawing/logo_htlinn.png");

	gtk_widget_set_size_request(cmd_start, BUTTON_WIDTH_WINDOW_MAIN, 2*BUTTON_HEIGHT_WINDOW_MAIN+BUTTON_SPACE); 
	//gtk_widget_set_size_request(cmd_difficulty, BUTTON_WIDTH_WINDOW_MAIN, BUTTON_HEIGHT_WINDOW_MAIN); 
	//gtk_widget_set_size_request(cmd_reference_signal, BUTTON_WIDTH_WINDOW_MAIN, BUTTON_HEIGHT_WINDOW_MAIN); 
	gtk_widget_set_size_request(cmd_settings, BUTTON_WIDTH_WINDOW_MAIN, BUTTON_HEIGHT_WINDOW_MAIN); 
	gtk_widget_set_size_request(cmd_shutdown, BUTTON_WIDTH_WINDOW_MAIN, BUTTON_HEIGHT_WINDOW_MAIN); 

	gtk_widget_set_size_request(image_htlinn, IMAGE_HTLINN_WIDTH, IMAGE_HTLINN_HEIGHT);
	gtk_widget_set_size_request(image_kindern_eine_chance, IMAGE_KINDERN_EINE_CHANCE_WIDTH, IMAGE_KINDERN_EINE_CHANCE_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_main), cmd_start, CMD_START_POSX, CMD_START_POSY);
	//gtk_fixed_put(GTK_FIXED(fixed_main), cmd_difficulty, CMD_DIFFICULTY_POSX, CMD_DIFFICULTY_POSY);
	//gtk_fixed_put(GTK_FIXED(fixed_main), cmd_reference_signal, CMD_REFERENCE_SIGNAL_POSX, CMD_REFERENCE_SIGNAL_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_main), cmd_settings, CMD_MUSIC_POSX, CMD_MUSIC_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_main), cmd_shutdown, CMD_SHUTDOWN_POSX, CMD_SHUTDOWN_POSY);

	gtk_fixed_put(GTK_FIXED(fixed_main), image_htlinn, IMAGE_HTLINN_POSX, IMAGE_HTLINN_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_main), image_kindern_eine_chance, IMAGE_KINDERN_EINE_CHANCE_POSX, IMAGE_KINDERN_EINE_CHANCE_POSY);

	gtk_container_add(GTK_CONTAINER(window_main), fixed_main);
	
}

void window_main_show()	{
	gtk_widget_show_all(window_main);
	//gdk_window_raise(window_main);
}

void window_main_hide()		{
	gtk_widget_hide_all(window_main);
}

static void cmd_settings_clicked(GtkWidget* widget, gpointer data)	{
	window_settings_hide();
	window_settings_show();
	//window_main_hide();
}

static void cmd_start_clicked(GtkWidget* widget, gpointer data)	{
	window_signal_hide();
	window_signal_show();
	//window_main_hide();
	//window_signal_init();
	window_signal_uart_start();
	//window_signal_main();
}

static void cmd_shutdown_clicked(GtkWidget* widget, gpointer data)	{
	window_shutdown_hide();
	window_shutdown_show();
}

static void window_main_destroy(GtkWidget* widget, gpointer data)	{
	g_print("killed\n");
	system("rm /tmp/cmd");
	system("killall omxplayer.bin");
	exit(0);
}

