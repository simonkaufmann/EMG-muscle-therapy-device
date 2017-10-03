#include "project.h"
#include <gtk/gtk.h>

#define TOP_SPACE 		10
#define BOTTOM_SPACE 	10
#define LEFT_SPACE		10
#define RIGHT_SPACE 	10

#define WIDGET_SPACE 	6
#define BUTTON_SPACE 	WIDGET_SPACE

#define NUMBER_BUTTONS	4
#define BUTTON_HEIGHT	((SCREENY-TOP_SPACE-BOTTOM_SPACE-LBL_CALIBRATION_HEIGHT-WIDGET_SPACE-CMD_BACK_HEIGHT)/NUMBER_BUTTONS-BUTTON_SPACE) /* -10 for optical reasons so that cmd_back and hscroll_duty is not so nearby!) */
#define BUTTON_WIDTH	(SCREENX-LEFT_SPACE-RIGHT_SPACE)

#define LBL_CALIBRATION_HEIGHT 28 // find it out by trying out!
#define LBL_CALIBRATION_WIDTH  no_value // don't really know the exact value

#define DRAW_AREA_CALIBRATION_HEIGHT 	2*BUTTON_HEIGHT+BUTTON_SPACE
#define DRAW_AREA_CALIBRATION_WIDTH		BUTTON_WIDTH

#define CMD_MIN_VALUE_HEIGHT			(BUTTON_HEIGHT*2)
#define CMD_MIN_VALUE_WIDTH				BUTTON_WIDTH/2-BUTTON_SPACE

#define CMD_MAX_VALUE_HEIGHT			(BUTTON_HEIGHT*2)
#define CMD_MAX_VALUE_WIDTH				(BUTTON_WIDTH/2-BUTTON_SPACE)

#define LBL_CALIBRATION_POSX			LEFT_SPACE
#define LBL_CALIBRATION_POSY			TOP_SPACE

#define DRAW_AREA_CALIBRATION_POSX		LEFT_SPACE
#define DRAW_AREA_CALIBRATION_POSY		LBL_CALIBRATION_POSY+LBL_CALIBRATION_HEIGHT+WIDGET_SPACE

#define CMD_MIN_VALUE_POSX				LEFT_SPACE
#define CMD_MIN_VALUE_POSY				DRAW_AREA_CALIBRATION_POSY+DRAW_AREA_CALIBRATION_HEIGHT+BUTTON_SPACE

#define CMD_MAX_VALUE_POSX				SCREENX-RIGHT_SPACE-CMD_MAX_VALUE_WIDTH /*CMD_MIN_VALUE_POSX+CMD_MIN_VALUE_WIDTH+BUTTON_SPACE*/
#define CMD_MAX_VALUE_POSY				CMD_MIN_VALUE_POSY

#define VALUE_BUFFER_SIZE				5


#define CMD_RESET_WIDTH					CMD_BACK_WIDTH
#define CMD_RESET_HEIGHT				CMD_BACK_HEIGHT

#define CMD_RESET_POSX					SCREENX-RIGHT_SPACE-CMD_RESET_WIDTH
#define CMD_RESET_POSY					CMD_BACK_POSY

#define BUFFER_X_DIVIDER	3
#define BUFFER_X			(DRAW_AREA_CALIBRATION_WIDTH/BUFFER_X_DIVIDER)

#define VALUE_MAX			255.0


// function declarations
static void init_gc(void);
static void draw_area_calibration_expose_event(GtkWidget*, GdkEventExpose*, gpointer);
static void cmd_back_clicked(GtkWidget*, gpointer);
static void cmd_home_clicked(GtkWidget*, gpointer);
void get_value(int);
static int  calculate_average();
static void cmd_min_value_clicked(GtkWidget*, gpointer);
static void cmd_max_value_clicked(GtkWidget*, gpointer);
static void cmd_reset_clicked(GtkWidget*, gpointer);

// variable declarations

GtkWidget* window_calibration;
GtkWidget* fixed_calibration;
GtkWidget* lbl_calibration;
GtkWidget* draw_area_calibration;
GdkPixmap* pixmap_calibration;
GtkWidget* cmd_min_value;
GtkWidget* lbl_cmd_min_value_child;
GtkWidget* cmd_max_value;
GtkWidget* lbl_cmd_max_value_child;
GtkWidget* cmd_back;
GtkWidget* cmd_home;
GtkWidget* cmd_reset;

GdkGC* gc;
GdkGC* gcgr; // green
GdkGC* gcbl; // gc black line 

GtkAllocation draw_area_size;

GdkColor orange, blue, green;

//int measured_signal[BUFFER_X+1];
int measured_signal[BUFFER_X+1];
int measured_signal_count_new;
int measured_signal_count;
int value_buffer[VALUE_BUFFER_SIZE];
int value_buffer_count=0;
static int is_stop=TRUE;
//static int value_new=FALSE;

int min_value;
int max_value;
int min_value_is_set=FALSE;
int max_value_is_set=FALSE;

int old_value=0;
int old_min_value=0;
int old_max_value=0;

//speed, duty cycle

	
void window_calibration_init()	{

	int i;
	for(i=0; i<BUFFER_X; i++)	{
		measured_signal[i]=0;
	}

	window_signal_set_value_function(get_value);

	//define some colors
	// colors have to get allocated with gtk_collormap_alloc()
	gdk_color_parse("orange", &orange);

	gdk_color_parse("blue", &blue);

	gdk_color_parse("green", &green);



	window_calibration = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_calibration), SCREENX, SCREENY);

	fixed_calibration = gtk_fixed_new();

	cmd_back = gtk_button_new_with_label("Back");
	g_signal_connect(G_OBJECT(cmd_back), "clicked", G_CALLBACK(cmd_back_clicked), NULL);

	cmd_home = gtk_button_new_with_label("Home");
	g_signal_connect(G_OBJECT(cmd_home), "clicked", G_CALLBACK(cmd_home_clicked), NULL);

	cmd_reset = gtk_button_new_with_label("    Reset\nCalibration");
	g_signal_connect(G_OBJECT(cmd_reset), "clicked", G_CALLBACK(cmd_reset_clicked), NULL);

	lbl_calibration = gtk_label_new("Calibration");
	gtk_label_set_markup(GTK_LABEL(lbl_calibration), "<span font=\'Liberation Sans 22\'>Calibration</span>");

	draw_area_calibration = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (draw_area_calibration), "expose_event",
                    G_CALLBACK (draw_area_calibration_expose_event), NULL);

	
	cmd_min_value = gtk_button_new();
	lbl_cmd_min_value_child = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(lbl_cmd_min_value_child), "<span foreground=\'orange\'>Relaxed Muscle</span>");
	gtk_container_add(GTK_CONTAINER(cmd_min_value), lbl_cmd_min_value_child);
	g_signal_connect(G_OBJECT(cmd_min_value), "clicked", G_CALLBACK(cmd_min_value_clicked), NULL);

	cmd_max_value = gtk_button_new();
	lbl_cmd_max_value_child = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(lbl_cmd_max_value_child), "<span foreground=\'blue\'>Contracted Muscle</span>");
	gtk_container_add(GTK_CONTAINER(cmd_max_value), lbl_cmd_max_value_child);
	g_signal_connect(G_OBJECT(cmd_max_value), "clicked", G_CALLBACK(cmd_max_value_clicked), NULL);
	
	gtk_widget_set_size_request(draw_area_calibration, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);
	gtk_widget_set_size_request(cmd_back, CMD_BACK_WIDTH, CMD_BACK_HEIGHT);
	gtk_widget_set_size_request(cmd_home, CMD_HOME_WIDTH, CMD_HOME_HEIGHT);
	gtk_widget_set_size_request(cmd_min_value, CMD_MIN_VALUE_WIDTH, CMD_MIN_VALUE_HEIGHT);
	gtk_widget_set_size_request(cmd_max_value, CMD_MAX_VALUE_WIDTH, CMD_MAX_VALUE_HEIGHT);
	gtk_widget_set_size_request(cmd_reset, CMD_RESET_WIDTH, CMD_RESET_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_calibration), lbl_calibration, LBL_CALIBRATION_POSX, LBL_CALIBRATION_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), draw_area_calibration, DRAW_AREA_CALIBRATION_POSX, DRAW_AREA_CALIBRATION_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), cmd_back, CMD_BACK_POSX, CMD_BACK_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), cmd_home, CMD_HOME_POSX, CMD_HOME_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), cmd_min_value, CMD_MIN_VALUE_POSX, CMD_MIN_VALUE_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), cmd_max_value, CMD_MAX_VALUE_POSX, CMD_MAX_VALUE_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_calibration), cmd_reset, CMD_RESET_POSX, CMD_RESET_POSY);

	gtk_container_add(GTK_CONTAINER(window_calibration), fixed_calibration);

	gtk_widget_get_allocation(draw_area_calibration, &draw_area_size);
	//g_print("width: %d \n height: %d \n", draw_area_size.width, draw_area_size.height);

	
}

static void init_gc(void)	{

	old_value=0;
	old_min_value=0;
	old_max_value=0; /* that these values are plotted again when draw area is shown newly */

	pixmap_calibration = gdk_pixmap_new(draw_area_calibration->window, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT, -1);
	/* also here pixmap is two times as wide as draw_area because then measured_signal drawing with for loop is easier! */

	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_calibration), &orange, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_calibration), &blue, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area_calibration), &green, FALSE, TRUE);

	gc = gdk_gc_new(pixmap_calibration);
	*gc = *(draw_area_calibration->style->fg_gc[gtk_widget_get_state(draw_area_calibration)]);
	gdk_gc_set_foreground(gc, &orange);
	gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcgr = gdk_gc_new(pixmap_calibration);
	*gcgr = *(draw_area_calibration->style->fg_gc[gtk_widget_get_state(draw_area_calibration)]);
	gdk_gc_set_foreground(gcgr, &green);
	gdk_gc_set_line_attributes(gcgr, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcbl = gdk_gc_new(pixmap_calibration); // black graphic context
	*gcbl = *(draw_area_calibration->style->fg_gc[gtk_widget_get_state(draw_area_calibration)]);
	gdk_gc_set_foreground(gcbl, &blue);
	gdk_gc_set_line_attributes(gcbl, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);
	gdk_draw_rectangle(pixmap_calibration, draw_area_calibration->style->bg_gc[0], TRUE, 0, 0, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT);

	gdk_draw_drawable(draw_area_calibration->window, gc, pixmap_calibration, 0, 0, 0, 0, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);

	is_stop = FALSE;

}

void window_calibration_show()	{
	gtk_widget_show_all(window_calibration);
	if(is_stop==TRUE)	{
		/* initialize graphic contexts etc. (must not be done before showing the window! */
		init_gc();
		FILE *u;
		u = fopen("/dev/ttyAMA0", "w");
		fputc(0x44, u);
		fclose(u);
	}
}

void window_calibration_hide()	{
	if(is_stop==FALSE)	{
		is_stop = TRUE;
		window_signal_uart_stop();
		gtk_widget_hide(window_calibration);
		FILE *u;
		u = fopen("/dev/ttyAMA0", "w");
		fputc(0x22, u);
		fclose(u);
	}
}

static void draw_area_calibration_expose_event(GtkWidget* widget, GdkEventExpose* event, gpointer data)	{
	//gdk_draw_drawable(widget->window, gc, pixmap_calibration, 0, 0, 0, 0, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);


	return TRUE;
}

static void cmd_back_clicked(GtkWidget* widget, gpointer data)	{
	window_calibration_hide();
	window_settings_show();
}

static void cmd_home_clicked(GtkWidget* widget, gpointer data)	{
	window_calibration_hide();
	window_settings_hide();
	window_main_show();
}

static void cmd_min_value_clicked(GtkWidget* widget, gpointer data)	{
	int average = calculate_average();
	min_value = average/VALUE_MAX*(DRAW_AREA_CALIBRATION_HEIGHT-2);
	min_value_is_set = TRUE;
	window_signal_set_calibration_min(average);
	g_print("calibra_min: %d\n", average);
	//value_new=TRUE;
}

static void cmd_max_value_clicked(GtkWidget* widget, gpointer data)	{
	int average = calculate_average();
	max_value = average/VALUE_MAX*(DRAW_AREA_CALIBRATION_HEIGHT-2);
	max_value_is_set = TRUE;
	window_signal_set_calibration_max(average);
	g_print("calibrt_max: %d\n", average);
	//value_new=TRUE;
}

static void cmd_reset_clicked(GtkWidget* widget, gpointer data)	{
	max_value_is_set=FALSE;
	min_value_is_set=FALSE;
	max_value=0;
	min_value=0;
	window_signal_set_calibration_min(CALIBRATION_MIN_STANDARD);
	window_signal_set_calibration_max(CALIBRATION_MAX_STANDARD);
}

void get_value(int value)	{
	if(is_stop==FALSE)	{
		// fortsetzen bei is_stop -> nur wenn Fenster gezeigt wird!!! bei hide wieder FALSE
		value_buffer[value_buffer_count]=value;
		value_buffer_count++;
		if(value_buffer_count>=VALUE_BUFFER_SIZE)	{
			value_buffer_count=0;
		}
		measured_signal_count_new++;
		if(measured_signal_count_new>=BUFFER_X)	{
			measured_signal_count_new=0;
		}
		measured_signal[measured_signal_count_new]=value;
		//g_print("value_orig: %d\n", value);
		//value_new=TRUE;
		//draw_measured_signal();
		/* no draw_measured_signal here!! -> it is not thread save so the drawing has to be done out of main_loop!! */
	}
}

int window_calibration_is_stop(void)	{
	return is_stop;
}

static int calculate_average()	{
	int i=0;
	int average=0;
	for(i=0; i<VALUE_BUFFER_SIZE; i++)	{
		average+=value_buffer[i];
	}
	//g_print("orig-average: %d\n", average);
	average=average/VALUE_BUFFER_SIZE;
	return average;
}


void window_calibration_main(void)	{

		int average = calculate_average();
		int value = (int)((float)(average)/VALUE_MAX*(DRAW_AREA_CALIBRATION_HEIGHT-2));
		//g_print("value: %d\n", value);
		if(measured_signal_count_new!=measured_signal_count||min_value!=old_min_value||max_value!=old_max_value)	{
			//g_print("sig: %d\n", measured_signal[measured_signal_count]);
			//g_print("sig2: %d\n\n", measured_signal[measured_signal_count_new]);
			gtk_widget_show_all(window_calibration);

			int measured_signal_count_new_save=measured_signal_count_new;
			int tmp_count;

			if(measured_signal_count_new_save<measured_signal_count)	{
				tmp_count = measured_signal_count+1;
			}
			else	{
				tmp_count = measured_signal_count_new_save;
			}

			// plot measured measured_signal
			measured_signal[BUFFER_X]=measured_signal[0]; //needed for the output routine!!
			gdk_draw_line(pixmap_calibration, gcgr, BUFFER_X_DIVIDER*measured_signal_count+DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT-(measured_signal[measured_signal_count]/VALUE_MAX*(DRAW_AREA_CALIBRATION_HEIGHT-2)), BUFFER_X_DIVIDER*(tmp_count)+DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT-(measured_signal[measured_signal_count_new_save]/VALUE_MAX*(DRAW_AREA_CALIBRATION_HEIGHT-2)));


	
			//gdk_draw_rectangle(pixmap_calibration, draw_area_calibration->style->bg_gc[0], TRUE, 0, 0, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);
				
		//	gdk_draw_line(pixmap_calibration, draw_area_calibration->style->bg_gc[0], 0, DRAW_AREA_CALIBRATION_HEIGHT-old_value, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT-old_value);
			//gdk_draw_line(pixmap_calibration, draw_area_calibration->style->bg_gc[0], 0, DRAW_AREA_CALIBRATION_HEIGHT-old_min_value, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT-old_min_value);
			//gdk_draw_line(pixmap_calibration, draw_area_calibration->style->bg_gc[0], 0, DRAW_AREA_CALIBRATION_HEIGHT-old_max_value, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT-old_max_value);

			old_value=value;
			old_min_value = min_value;
			old_max_value = max_value;

	
			//gdk_draw_line(pixmap_calibration, gcgr, 0, DRAW_AREA_CALIBRATION_HEIGHT-value, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT-value);
	
		
	
			gdk_draw_drawable(draw_area_calibration->window, gc, pixmap_calibration, measured_signal_count*BUFFER_X_DIVIDER, 0, 0, 0, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);


			/* is directly drawn into drawing area, so that deleting is not such a problem! */
			if(min_value_is_set == TRUE)	{
				gdk_draw_line(draw_area_calibration->window, gc, 0, DRAW_AREA_CALIBRATION_HEIGHT-min_value, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT-min_value);
			}
			if(max_value_is_set == TRUE)	{
				gdk_draw_line(draw_area_calibration->window, gcbl, 0, DRAW_AREA_CALIBRATION_HEIGHT-max_value, DRAW_AREA_CALIBRATION_WIDTH*2, DRAW_AREA_CALIBRATION_HEIGHT-max_value);
			}


			// delete the second half of the pixmap after the gdk_draw_lines!! otherwise screen will shortly be cleared on the raspberry!!!
			if(measured_signal_count_new_save < measured_signal_count)	{
				gdk_draw_drawable(pixmap_calibration, gc, pixmap_calibration, DRAW_AREA_CALIBRATION_WIDTH-1, 0, 0, 0, DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT);
				gdk_draw_rectangle(pixmap_calibration, draw_area_calibration->style->bg_gc[0], TRUE, DRAW_AREA_CALIBRATION_WIDTH, 0, 2*DRAW_AREA_CALIBRATION_WIDTH, DRAW_AREA_CALIBRATION_HEIGHT); // delete the second part of the screen so that it can be redrawn again!
			}
			measured_signal_count=measured_signal_count_new;


		}
}
