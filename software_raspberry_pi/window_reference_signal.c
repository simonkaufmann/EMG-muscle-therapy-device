#include "project.h"
#include <gtk/gtk.h>

#define TOP_SPACE 		10
#define BOTTOM_SPACE 	20
#define LEFT_SPACE		10
#define RIGHT_SPACE 	10

#define WIDGET_SPACE 	6
#define BUTTON_SPACE 	WIDGET_SPACE

#define NUMBER_BUTTONS	4
#define BUTTON_HEIGHT	(SCREENY-TOP_SPACE-BOTTOM_SPACE-LBL_REFERENCE_SIGNAL_HEIGHT-WIDGET_SPACE-CMD_BACK_HEIGHT-10)/NUMBER_BUTTONS-BUTTON_SPACE /* -10 for optical reasons so that cmd_back and hscroll_duty is not so nearby!) */
#define BUTTON_WIDTH	(SCREENX-LEFT_SPACE-RIGHT_SPACE)

#define LBL_REFERENCE_SIGNAL_HEIGHT 28 // find it out by trying out!
#define LBL_REFERENCE_SIGNAL_WIDTH  no_value // don't really know the exact value

#define DRAW_AREA_REFERENCE_SIGNAL_HEIGHT 	2*BUTTON_HEIGHT+BUTTON_SPACE
#define DRAW_AREA_REFERENCE_SIGNAL_WIDTH	BUTTON_WIDTH

#define HSCROLL_SPEED_HEIGHT BUTTON_HEIGHT
#define HSCROLL_SPEED_WIDTH	BUTTON_WIDTH-LBL_SPEED_WIDTH

#define HSCROLL_DUTY_HEIGHT	BUTTON_HEIGHT
#define HSCROLL_DUTY_WIDTH	BUTTON_WIDTH-LBL_DUTY_WIDTH

#define LBL_SPEED_WIDTH 	SCREENX/4
#define LBL_DUTY_WIDTH		SCREENX/4

#define LBL_REFERENCE_SIGNAL_POSX	LEFT_SPACE
#define LBL_REFERENCE_SIGNAL_POSY	TOP_SPACE

#define DRAW_AREA_REFERENCE_SIGNAL_POSX	LEFT_SPACE
#define DRAW_AREA_REFERENCE_SIGNAL_POSY	TOP_SPACE+LBL_REFERENCE_SIGNAL_POSY+LBL_REFERENCE_SIGNAL_HEIGHT+WIDGET_SPACE

#define LBL_SPEED_POSX		LEFT_SPACE
#define LBL_SPEED_POSY		DRAW_AREA_REFERENCE_SIGNAL_POSY+DRAW_AREA_REFERENCE_SIGNAL_HEIGHT+BUTTON_SPACE

#define HSCROLL_SPEED_POSX	LBL_SPEED_POSX+LBL_SPEED_WIDTH
#define HSCROLL_SPEED_POSY	LBL_SPEED_POSY-3

#define LBL_DUTY_POSX		LEFT_SPACE
#define LBL_DUTY_POSY		HSCROLL_SPEED_POSY+HSCROLL_SPEED_HEIGHT+BUTTON_SPACE

#define HSCROLL_DUTY_POSX	LBL_DUTY_POSX+LBL_DUTY_WIDTH
#define HSCROLL_DUTY_POSY	LBL_DUTY_POSY-3


#define HSCROLL_SPEED_MIN_VALUE		1
#define HSCROLL_SPEED_MAX_VALUE		SPEED_MAX

#define HSCROLL_DUTY_MIN_VALUE		1
#define HSCROLL_DUTY_MAX_VALUE		DUTY_MAX



// function declarations
static void init_gc(void);
static void draw_area_reference_signal_expose_event(GtkWidget*, GdkEventExpose*, gpointer);
static void cmd_back_clicked(GtkWidget*, gpointer);
static void cmd_home_clicked(GtkWidget*, gpointer);
static void hscroll_speed_value_changed(GtkRange*, gpointer);
static void hscroll_duty_value_changed(GtkRange*, gpointer);
static void draw_signal(void);

// variable declarations

GtkWidget* window_reference_signal;
GtkWidget* fixed_reference_signal;
GtkWidget* lbl_reference_signal;
GtkWidget* draw_area_reference_signal;
GdkPixmap* pixmap_reference_signal;
GtkWidget* lbl_speed;
GtkWidget* hscroll_speed;
GtkWidget* lbl_duty;
GtkWidget* hscroll_duty;
GtkObject* adj_speed;
GtkObject* adj_duty;
GtkWidget* cmd_back;
GtkWidget* cmd_home;

GdkGC* gc;
GdkGC* gcgr; // green
GdkGC* gcbl; // gc black line 

GtkAllocation draw_area_size;

GdkColor red, black, green;


//speed, duty cycle

	
void window_reference_signal_init()	{

	
	//define some colors
	// colors have to get allocated with gtk_collormap_alloc()
	red.red = 65535;
	red.blue = 0;
	red.green = 0;

	gdk_color_parse("black", &black);

	green.green = 40000;
	green.blue = 0;
	green.red = 0;


	window_reference_signal = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_reference_signal), SCREENX, SCREENY);

	fixed_reference_signal = gtk_fixed_new();

	cmd_back = gtk_button_new_with_label("Back");
	g_signal_connect(G_OBJECT(cmd_back), "clicked", G_CALLBACK(cmd_back_clicked), NULL);

	cmd_home = gtk_button_new_with_label("Home");
	g_signal_connect(G_OBJECT(cmd_home), "clicked", G_CALLBACK(cmd_home_clicked), NULL);

	lbl_reference_signal = gtk_label_new("Reference Signal");
	gtk_label_set_markup(GTK_LABEL(lbl_reference_signal), "<span font=\'Liberation Sans 22\'>Reference Signal</span>");

	draw_area_reference_signal = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (draw_area_reference_signal), "expose_event",
                    G_CALLBACK (draw_area_reference_signal_expose_event), NULL);

	
	adj_speed = gtk_adjustment_new(5, HSCROLL_SPEED_MIN_VALUE, HSCROLL_SPEED_MAX_VALUE, 1, 1, 0);

	hscroll_speed = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj_speed));
	g_signal_connect(G_OBJECT(hscroll_speed), "value-changed", G_CALLBACK(hscroll_speed_value_changed), NULL);

	lbl_speed = gtk_label_new("Speed");

	adj_duty = gtk_adjustment_new(5, HSCROLL_DUTY_MIN_VALUE, HSCROLL_DUTY_MAX_VALUE, 1, 1, 0);

	hscroll_duty = gtk_hscrollbar_new(GTK_ADJUSTMENT(adj_duty));
	g_signal_connect(G_OBJECT(hscroll_duty), "value-changed", G_CALLBACK(hscroll_duty_value_changed), NULL);

	lbl_duty = gtk_label_new("On-Off-Rate");
	
	gtk_widget_set_size_request(draw_area_reference_signal, DRAW_AREA_REFERENCE_SIGNAL_WIDTH, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT);
	gtk_widget_set_size_request(hscroll_speed, HSCROLL_SPEED_WIDTH, HSCROLL_SPEED_HEIGHT);
	gtk_widget_set_size_request(hscroll_duty, HSCROLL_DUTY_WIDTH, HSCROLL_DUTY_HEIGHT);
	gtk_widget_set_size_request(cmd_back, CMD_BACK_WIDTH, CMD_BACK_HEIGHT);
	gtk_widget_set_size_request(cmd_home, CMD_HOME_WIDTH, CMD_HOME_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), lbl_reference_signal, LBL_REFERENCE_SIGNAL_POSX, LBL_REFERENCE_SIGNAL_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), draw_area_reference_signal, DRAW_AREA_REFERENCE_SIGNAL_POSX, DRAW_AREA_REFERENCE_SIGNAL_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), hscroll_speed, HSCROLL_SPEED_POSX, HSCROLL_SPEED_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), hscroll_duty, HSCROLL_DUTY_POSX, HSCROLL_DUTY_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), cmd_back, CMD_BACK_POSX, CMD_BACK_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), cmd_home, CMD_HOME_POSX, CMD_HOME_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), lbl_speed, LBL_SPEED_POSX, LBL_SPEED_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_reference_signal), lbl_duty, LBL_DUTY_POSX, LBL_DUTY_POSY);

	gtk_container_add(GTK_CONTAINER(window_reference_signal), fixed_reference_signal);

	gtk_widget_get_allocation(draw_area_reference_signal, &draw_area_size);
	//g_print("width: %d \n height: %d \n", draw_area_size.width, draw_area_size.height);

	
}

static void init_gc(void)	{
	pixmap_reference_signal = gdk_pixmap_new(draw_area_reference_signal->window, DRAW_AREA_REFERENCE_SIGNAL_WIDTH*2, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT, -1);
	/* also here pixmap is two times as wide as draw_area because then signal drawing with for loop is easier! */

	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_reference_signal), &red, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_reference_signal), &black, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_reference_signal), &green, FALSE, TRUE);

	gc = gdk_gc_new(pixmap_reference_signal);
	*gc = *(draw_area_reference_signal->style->fg_gc[gtk_widget_get_state(draw_area_reference_signal)]);
	gdk_gc_set_foreground(gc, &red);
	gdk_gc_set_line_attributes(gc, 3, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcgr = gdk_gc_new(pixmap_reference_signal);
	*gcgr = *(draw_area_reference_signal->style->fg_gc[gtk_widget_get_state(draw_area_reference_signal)]);
	gdk_gc_set_foreground(gcgr, &green);
	gdk_gc_set_line_attributes(gcgr, 3, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcbl = gdk_gc_new(pixmap_reference_signal); // black graphic context
	*gcbl = *(draw_area_reference_signal->style->fg_gc[gtk_widget_get_state(draw_area_reference_signal)]);
	gdk_gc_set_foreground(gcbl, &black);
	gdk_gc_set_line_attributes(gcbl, 3, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
	gdk_draw_rectangle(pixmap_reference_signal, draw_area_reference_signal->style->bg_gc[0], TRUE, 0, 0, DRAW_AREA_REFERENCE_SIGNAL_WIDTH, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT);

}

void window_reference_signal_show()	{
	gtk_widget_show_all(window_reference_signal);
	/* initialize graphic contexts etc. (must not be done before showing the window! */
	init_gc();
	draw_signal();
}

void window_reference_signal_hide()	{
	gtk_widget_hide(window_reference_signal);
}

static void draw_area_reference_signal_expose_event(GtkWidget* widget, GdkEventExpose* event, gpointer data)	{
	gdk_draw_drawable(widget->window, gc, pixmap_reference_signal, 0, 0, 0, 0, DRAW_AREA_REFERENCE_SIGNAL_WIDTH, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT);
}

static void cmd_back_clicked(GtkWidget* widget, gpointer data)	{
	window_reference_signal_hide();
	window_settings_show();
}

static void cmd_home_clicked(GtkWidget* widget, gpointer data)	{
	window_reference_signal_hide();
	window_settings_hide();
	window_main_show();
}

static void hscroll_speed_value_changed(GtkRange* range, gpointer data)	{
	int val = gtk_range_get_value(range);
	g_print("speed-value: %d\n", val);
	window_signal_set_speed(val);
	draw_signal();
}

static void hscroll_duty_value_changed(GtkRange* range, gpointer data)	{
	int val = gtk_range_get_value(range);
	g_print("duty-value: %d\n", val);
	window_signal_set_duty(val);
	draw_signal();
}

static void draw_signal(void)	{
	gdk_draw_rectangle(pixmap_reference_signal, draw_area_reference_signal->style->bg_gc[0], TRUE, 0, 0, DRAW_AREA_REFERENCE_SIGNAL_WIDTH, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT);


	int standard_signal_max_width = window_signal_get_signal_max_width();
	int standard_signal_width = window_signal_get_signal_width();	
	int here_signal_width = (float)standard_signal_width/standard_signal_max_width*(DRAW_AREA_REFERENCE_SIGNAL_WIDTH/*+3*/); /* +3 so that the last rising line is not shown (is not so beautiful!) */
	int standard_signal_border = window_signal_get_border();
	int here_border = (float)standard_signal_border/standard_signal_max_width*(DRAW_AREA_REFERENCE_SIGNAL_WIDTH/*+3*/);
	int divider = DRAW_AREA_REFERENCE_SIGNAL_WIDTH/here_signal_width;

//	g_print("\nnew\n");
//	g_print("here_width: %d\n", here_signal_width);
//	g_print("divider: %d\n", divider);
//	g_print("here_boder: %d\n", here_border);

	int i;
	for(i = 0; i<=divider; i++)	{
		gdk_draw_line(pixmap_reference_signal, gc, i*here_signal_width, 5, i*here_signal_width+here_border, 5);
		gdk_draw_line(pixmap_reference_signal, gc, i*here_signal_width+here_border, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT-5, (i+1)*here_signal_width, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT-5);
		gdk_draw_line(pixmap_reference_signal, gc, i*here_signal_width+here_border, 5, i*here_signal_width+here_border, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT-5);
		gdk_draw_line(pixmap_reference_signal, gc, (i+1)*here_signal_width, 5, (i+1)*here_signal_width, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT-5);
	}

	gdk_draw_drawable(draw_area_reference_signal->window, gc, pixmap_reference_signal, 0, 0, 0, 0, DRAW_AREA_REFERENCE_SIGNAL_WIDTH, DRAW_AREA_REFERENCE_SIGNAL_HEIGHT);

}
