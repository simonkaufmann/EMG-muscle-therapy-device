#include <gtk/gtk.h>
#include "project.h"
#include <stdlib.h>
#include <unistd.h>

#define TOP_SPACE 3
#define LEFT_SPACE 3
#define RIGHT_SPACE 7
#define BOTTOM_SPACE 3
#define WIDGET_SPACE 3


#define CMD_PAUSE_WIDTH 60
#define CMD_STOP_WIDTH 60
#define CMD_MUTE_WIDTH 40 /*  32 */
#define DRAW_AREA_WIDTH (VSCROLL_VOLUME_POSX-WIDGET_SPACE-LEFT_SPACE)

#define LBL_MUSIC_WIDTH		(CMD_MUTE_POSX-WIDGET_SPACE-WIDGET_SPACE*2-CMD_STOP_POSX)
#define LBL_MUSIC_HEIGHT	BUTTON_HEIGHT

#define LBL_MUSIC_POSX	(CMD_STOP_POSX+CMD_STOP_WIDTH+WIDGET_SPACE*2)
#define LBL_MUSIC_POSY	(TOP_SPACE)

#define VSCROLL_VOLUME_HEIGHT (SCREENY-VSCROLL_VOLUME_POSY-BOTTOM_SPACE-5)
#define BUTTON_HEIGHT CMD_HOME_HEIGHT
#define DRAW_AREA_HEIGHT (SCREENY-TOP_SPACE-BOTTOM_SPACE-BUTTON_HEIGHT-WIDGET_SPACE)


#define VSCROLL_VOLUME_POSY 35
#define DRAW_AREA_POSY (BUTTON_HEIGHT+TOP_SPACE+WIDGET_SPACE)

#define CMD_STOP_POSX (LEFT_SPACE+CMD_PAUSE_WIDTH+WIDGET_SPACE)
//#define VSCROLL_VOLUME_POSX (SCREENX-BUTTON_HEIGHT-WIDGET_SPACE)
#define VSCROLL_VOLUME_POSX (SCREENX-RIGHT_SPACE)
#define CMD_MUTE_POSX (SCREENX-RIGHT_SPACE-CMD_MUTE_WIDTH)
#define DRAW_AREA_POSX (LEFT_SPACE)

#define BUFFER_X_DIVIDER 3
#define BUFFER_X (DRAW_AREA_WIDTH/BUFFER_X_DIVIDER) // -> 94 Werte werden am Bildschirm angezeigt!
#define BUFFER_Y_DIVIDER 2
#define BUFFER_Y 80 //von 0 bis 80 geht die Skala nach oben (rund 2 Pixel pro Wert!)
#define BUFFER_PIXEL_OFFSET_Y 10 //von unten 10 pixel ist offset auf der Drawing Area!

#define VOLUME_MAX 100 //now done in percentage!
#define VSCROLL_VOLUME_MAX 10
#define VSCROLL_VOLUME_INIT 8 

#define VARIATION_NUMBER 3
#define VARIATION_OFFSET 20 //up to this variation no variation will be outputed to PSoC (-> no noise up to this variation!)

#define STANDARD_SIGNAL_MAX	((int)(BUFFER_X*1.5))
#define STANDARD_SIGNAL_MIN	BUFFER_X/3

//#define UART_RECEIVED_MULTIPLIER_MIN 80   -> see CALIBRATION_MAX_OVERSIZED  and MAX_UNDERSIZED instead!
//#define UART_RECEIVED_MULTIPLIER_MAX 200

#define UART_SEND_MULTIPLIER_MIN 50
#define UART_SEND_MULTIPLIER_MAX 250


#define SIGNAL_BASE_LINE	10
#define CALIBRATION_MAX_OVERSIZED 1.5		/* the factor that the reference signal is higher at most difficulty than the calibration max_value! */
#define CALIBRATION_MAX_UNDERSIZED 0.3

#define STANDARD_SIGNAL_HIGH	70
#define STANDARD_SIGNAL_LOW		10


/* function definitions */
static void init_gc(void);
gboolean expose_event_callback(GtkWidget*, GdkEventExpose*, gpointer);
static void cmd_mute_clicked(GtkWidget*, gpointer);
static void vscroll_volume_value_changed(GtkWidget*, gpointer);
static void cmd_pause_clicked(GtkWidget*, gpointer);
static void cmd_stop_clicked(GtkWidget*, gpointer);
void* uart_receive(void*);
void* uart_send(void*);
void* play_music(void*);
void change_volume(int);
static void calculate_standard_signal(void);
static void calculate_difficulty(void);
static void calculate_sensitivity(void);

/* widget declarations */ 
GtkWidget* window_signal;
GtkWidget* draw_area;
GdkPixmap* pixmap;

GtkWidget* fixed_top;
GtkWidget* image_volume_off;
GtkWidget* image_volume_on;


GtkWidget* cmd_pause;
GtkWidget* cmd_stop;
GtkWidget* cmd_mute;
GtkWidget* vscroll_volume;

GtkWidget* lbl_music_file;

GtkObject* adj_volume;

GtkAllocation draw_area_size;


GdkGC* gc; // graphic context
GdkGC* gcgr; // green
GdkGC* gcbl; // gc black line 

GdkColor red, green, black;

FILE *uart;
FILE *uart2;
int received;

char music_path[MAX_PATH_LENGTH]="/home/pi/music/highway_lights.mp3"; 

volatile pthread_t thread_send;
volatile pthread_t thread_uart_receive;
volatile pthread_t thread_music;

volatile int standard_signal[STANDARD_SIGNAL_MAX+1]; //one more is needed for output routine down in the main loop (0 and BUFFER_X have the same value than!)
volatile int measure_signal[BUFFER_X+1];
volatile int variation[VARIATION_NUMBER]; //abweichung
volatile int variation_average;
volatile int variation_count=0;
volatile int count, count_new, count_standard_signal;
volatile int initial_count=0;

volatile int mute = FALSE; 
volatile int volume = (float)VSCROLL_VOLUME_INIT/VSCROLL_VOLUME_MAX*VOLUME_MAX;

volatile int is_stop=TRUE, is_pause=FALSE;

volatile int duty=DUTY_MAX/2; /* between 1 an 10 */
volatile int speed=SPEED_MAX/2;
volatile int sensitivity=SENSITIVITY_MAX/2;
volatile int difficulty=DIFFICULTY_MAX/2;
volatile int standard_signal_max;
volatile int border=0;
volatile float uart_received_multiplier;
volatile int uart_send_multiplier;

void (* volatile value_function)(int);
volatile int value_function_is_set=FALSE;

volatile int calibration_max=CALIBRATION_MAX_STANDARD;
volatile int calibration_min=CALIBRATION_MIN_STANDARD;

volatile int uart_is_start=FALSE;
volatile int uart2_is_start=FALSE;

void window_signal_init(void)	{

//	FILE *standard_song = fopen("/home/pi/gtk_drawing/standard_song", "r");
//	if(standard_song!=NULL)	{
//		fgets(music_path, MAX_PATH_LENGTH, standard_song);
//		fclose(standard_song);
//	}

	system("rm /tmp/cmd");
	system("mkfifo /tmp/cmd");
	
	/* uart initialisation and sound and so on must be included!
	+ this file must be configurable (speed and so on) */
	

	// set raspberry pi's uart
	system("sudo stty -onlcr -onlret -olcuc -ocrnl -echo 9600 -F /dev/ttyAMA0");
	//fprintf(uart, "Draw Area:\nWidth: %d, Height: %d", DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

	// open the uart
	//uart = fopen("/dev/ttyUSB0", "r+"); // read and write!
	//if(uart==NULL)	{
//		g_print("Error could not open UART /dev/ttyAMA0\n");
//	}

	//define some colors
	// colors have to get allocated with gtk_collormap_alloc()
	red.red = 65535;
	red.blue = 0;
	red.green = 0;

	gdk_color_parse("black", &black);

	green.green = 40000;
	green.blue = 0;
	green.red = 0;

	// initalization of controls 

	image_volume_on = gtk_image_new_from_file("/home/pi/gtk_drawing/lautsprecher_mit.png");
	image_volume_off = gtk_image_new_from_file("/home/pi/gtk_drawing/lautsprecher_ohne.png");

	fixed_top = gtk_fixed_new();

	adj_volume = gtk_adjustment_new(VSCROLL_VOLUME_MAX-VSCROLL_VOLUME_INIT, 0, VSCROLL_VOLUME_MAX, 1, 1, 1);

	vscroll_volume = gtk_vscrollbar_new(GTK_ADJUSTMENT(adj_volume));
	g_signal_connect(G_OBJECT(vscroll_volume), "value-changed", G_CALLBACK(vscroll_volume_value_changed), NULL);

	cmd_pause = gtk_button_new_with_label("Pause");
	g_signal_connect(G_OBJECT(cmd_pause), "clicked", G_CALLBACK(cmd_pause_clicked), NULL);

	cmd_stop = gtk_button_new_with_label("Stop");
	g_signal_connect(G_OBJECT(cmd_stop), "clicked", G_CALLBACK(cmd_stop_clicked), NULL);

	cmd_mute = gtk_button_new(); // new label because of image!
	mute = FALSE;
	gtk_button_set_image(GTK_BUTTON(cmd_mute), image_volume_on);
	g_signal_connect(G_OBJECT(cmd_mute), "clicked", G_CALLBACK(cmd_mute_clicked), NULL);

	lbl_music_file = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(lbl_music_file), 0, 0.5);

	window_signal = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(window_signal), 3);
	gtk_window_set_default_size(GTK_WINDOW(window_signal), SCREENX, SCREENY);
	//g_signal_connect(G_OBJECT(window_signal), "destroy", G_CALLBACK(destroy), NULL);
	//gdk_window_set_cursor(GDK_WINDOW(window), GDK_BLANK_CURSOR);
	// gdk_window is needed, but we have gtk_window -> how to get gtk_window's gdk_window?
	
	draw_area = gtk_drawing_area_new();
	g_signal_connect (G_OBJECT (draw_area), "expose_event",
                    G_CALLBACK (expose_event_callback), NULL);
	
	// include controls 
	
	gtk_widget_set_size_request(cmd_pause, CMD_PAUSE_WIDTH, BUTTON_HEIGHT);
	gtk_widget_set_size_request(cmd_stop, CMD_STOP_WIDTH, BUTTON_HEIGHT);
	gtk_widget_set_size_request(vscroll_volume, BUTTON_HEIGHT, VSCROLL_VOLUME_HEIGHT);
	gtk_widget_set_size_request(cmd_mute, CMD_MUTE_WIDTH, BUTTON_HEIGHT);
	gtk_widget_set_size_request(draw_area, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
	gtk_widget_set_size_request(lbl_music_file, LBL_MUSIC_WIDTH, LBL_MUSIC_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_top), lbl_music_file, LBL_MUSIC_POSX, LBL_MUSIC_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_top), cmd_pause, LEFT_SPACE, TOP_SPACE);
	gtk_fixed_put(GTK_FIXED(fixed_top), cmd_stop, CMD_STOP_POSX, TOP_SPACE);
	//gtk_fixed_put(GTK_FIXED(fixed_top), vscroll_volume, VSCROLL_VOLUME_POSX, VSCROLL_VOLUME_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_top), cmd_mute, CMD_MUTE_POSX, TOP_SPACE);
	gtk_fixed_put(GTK_FIXED(fixed_top), draw_area, DRAW_AREA_POSX, DRAW_AREA_POSY);
		
	gtk_container_add(GTK_CONTAINER(window_signal), fixed_top);
	

	
	
	//
	//g_print("thread uart_received started\n");
	//pthread_t thread_uart;
	//pthread_create(&thread_uart, NULL, &uart_receive, NULL);

	//change_volume(volume);

	/* is this needed:?*/
	
	/*while(gtk_events_pending())	{
		gtk_main_iteration();
	}*/


}

void window_signal_uart_start(void)	{
	if(uart_is_start==FALSE)	{
		uart = fopen("/dev/ttyAMA0", "r");
		pthread_create(&thread_uart_receive, NULL, &uart_receive, NULL);
		uart_is_start=TRUE;
	}
}

void window_signal_uart_stop(void)	{
	if(uart_is_start==TRUE)	{
		g_print("vor cancel\n");
		pthread_cancel(thread_uart_receive);
		g_print("vor uart\n");
		fclose(uart);
		g_print("vor uart2\n");
		uart_is_start=FALSE;
	}
	if(uart2_is_start==TRUE)	{
		fclose(uart2);
		uart2_is_start=FALSE;
	}
}


void window_signal_main(void)	{

	//g_print("signal_main\n");
	gtk_widget_show_all(window_signal); /* is necessary so that drawable (draw_area) can be drawn!!! */
	//while(1)	{
		//while(gtk_events_pending())	{
		//	gtk_main_iteration();
		//}
		//gtk_main_iteration_do(FALSE);
		if(count!=count_new)	{

			count_standard_signal++;
			if(count_standard_signal>=standard_signal_max)	{
				count_standard_signal=0;
			}
			//g_print("newcount\n");
			int count_new_save = count_new;
			int tmp_count;
			//gdk_draw_line(pixmap, gcbl, 10, 0, 10, count);
			//delete screen:
			//gdk_draw_rectangle(pixmap, draw_area->style->bg_gc[0], TRUE, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT); // make screen in default color 
			if(count_new_save<count)	{ // screen at left again!
				tmp_count = count+1;
			}
			else	{
				tmp_count = count_new_save;
			}
			//gdk_draw_rectangle(pixmap, draw_area->style->bg_gc[0], TRUE, BUFFER_X_DIVIDER*count+DRAW_AREA_WIDTH, 0, BUFFER_X_DIVIDER*(count+1)+DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);

			// a line will be drawn between the old count and the count_new value (if because of time problems some count_new values weren't executed, the
			// will be jumped over but there won't be a space, the line ist just longer)

			// plot reference signal
			standard_signal[standard_signal_max]=standard_signal[0]; //needed for the output routine!!
			gdk_draw_line(pixmap, gc, BUFFER_X_DIVIDER*count+DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT-(standard_signal[count_standard_signal]*BUFFER_Y_DIVIDER+BUFFER_PIXEL_OFFSET_Y), BUFFER_X_DIVIDER*(tmp_count)+DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT-(standard_signal[count_standard_signal+1]*BUFFER_Y_DIVIDER+BUFFER_PIXEL_OFFSET_Y));
			/* it might be a problem that we don't have a count_new here but only a count_standard_signal+1 (if because of delays some counts are skipped!!*/
			
			// plot measured signal
			measure_signal[BUFFER_X]=measure_signal[0]; //needed for the output routine!!
			gdk_draw_line(pixmap, gcgr, BUFFER_X_DIVIDER*count+DRAW_AREA_WIDTH/2, DRAW_AREA_HEIGHT-(measure_signal[count]*BUFFER_Y_DIVIDER+BUFFER_PIXEL_OFFSET_Y), BUFFER_X_DIVIDER*(tmp_count)+DRAW_AREA_WIDTH/2, DRAW_AREA_HEIGHT-(measure_signal[count_new_save]*BUFFER_Y_DIVIDER+BUFFER_PIXEL_OFFSET_Y));

			//calculate variation of measured signal
			//calculation is done in receive thread!
			int tmp_count2;
			if((count_standard_signal-BUFFER_X/2)<0)	{
				tmp_count2 = standard_signal_max + (count_standard_signal-BUFFER_X/2);
				while(tmp_count2<0)	{
					tmp_count2 = standard_signal_max+tmp_count2;
				}
			}
			else	{
				tmp_count2 = count_standard_signal-BUFFER_X/2;
			}
			/* tmp_count2 is now index of standard_signal that is at the same position as the new measure_signal value*/
			/* this is needed because standard signal is shown also for the future so that person knows what to do next with the muscle */
			if(initial_count>BUFFER_X/2)	{
				/* so that at the beginning nothing is sent to psoc, otherwise standard_signal values that are  not shown would be sent */
				if(tmp_count2==0)	{
					fputc(0x88, uart2);
					fflush(uart2);
				}
				else if(tmp_count2==border)	{
					fputc(0x11, uart2);
					fflush(uart2);
				}
			}
			else	{
				initial_count++;
			}

			variation[variation_count] = abs(measure_signal[count_new_save]-standard_signal[tmp_count2])*uart_send_multiplier/100.0;
			//g_print("variation: %d\t\t standard: %d \t\t measure: %d \t\t tmp_count2: %d \t\t count_new_save: %d\t\t max: %d\n", variation[variation_count], standard_signal[tmp_count2], measure_signal[count_new_save], tmp_count2, count_new_save, standard_signal_max);
			variation_count++;
			if(variation_count>=VARIATION_NUMBER)	{
				variation_count = 0;
			}

			// write out changes of pixmap to the draw_area
			gdk_draw_drawable(draw_area->window, gc, pixmap,count*BUFFER_X_DIVIDER, 0, 0, 0, draw_area_size.width, draw_area_size.height);

			// delete the second half of the pixmap after the gdk_draw_lines!! otherwise screen will shortly be cleared on the raspberry!!!
			if(count_new_save < count)	{
				gdk_draw_drawable(pixmap, gc, pixmap, DRAW_AREA_WIDTH-1, 0, 0, 0, DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT);
				gdk_draw_rectangle(pixmap, draw_area->style->bg_gc[0], TRUE, DRAW_AREA_WIDTH, 0, 2*DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT); // delete the second part of the screen so that it can be redrawn again!
			}
			count=count_new;
		}
		//if (is_stop==TRUE)	{
		//	break;
		//}
	//}

}

void window_signal_set_duty(int set_duty)	{
	duty = set_duty;
	g_print("duty_set: %d\n", duty);
}

void window_signal_set_speed(int set_speed)	{
	speed = set_speed;
}

void window_signal_set_difficulty(int set_difficulty)	{
	difficulty = set_difficulty;
}

void window_signal_set_sensitivity(int set_sensitivity)	{
	sensitivity = set_sensitivity;
}

static void init_gc(void)	{
	
	count_new = 0;
	calculate_standard_signal();
	calculate_difficulty();
	calculate_sensitivity();

	count = 0;
	count_new = 0;
	count_standard_signal = border; /* border so that signal starts with low */
	initial_count=0;


	// drawing

	is_stop=FALSE;
	is_pause=FALSE;
	mute=FALSE;
	image_volume_on = GTK_WIDGET(gtk_image_new_from_file("/home/pi/gtk_drawing/lautsprecher_mit.png"));
	gtk_button_set_image(GTK_BUTTON(cmd_mute), image_volume_on);
	gtk_button_set_label(GTK_BUTTON(cmd_pause), "Pause");


	count = 0;
	count_new = 0;
	count_standard_signal = border; /* border so that signal starts with low */


	gtk_widget_get_allocation(draw_area, &draw_area_size);
	g_print("width: %d \n height: %d \n", draw_area_size.width, draw_area_size.height);
	
	pixmap = gdk_pixmap_new(draw_area->window, 2*draw_area_size.width, draw_area_size.height, -1);

	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area), &red, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area), &black, FALSE, TRUE);
	gdk_colormap_alloc_color(gtk_widget_get_colormap(draw_area), &green, FALSE, TRUE);
	
	gc = gdk_gc_new(pixmap);
	*gc = *(draw_area->style->fg_gc[gtk_widget_get_state(draw_area)]);
	gdk_gc_set_foreground(gc, &red);
	gdk_gc_set_line_attributes(gc, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcgr = gdk_gc_new(pixmap);
	*gcgr = *(draw_area->style->fg_gc[gtk_widget_get_state(draw_area)]);
	gdk_gc_set_foreground(gcgr, &green);
	gdk_gc_set_line_attributes(gcgr, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

	gcbl = gdk_gc_new(pixmap); // black graphic context
	*gcbl = *(draw_area->style->fg_gc[gtk_widget_get_state(draw_area)]);
	gdk_gc_set_foreground(gcbl, &black);
	gdk_gc_set_line_attributes(gcbl, 2, GDK_LINE_SOLID, GDK_CAP_NOT_LAST, GDK_JOIN_MITER);

}

int window_signal_get_signal_width()	{
	calculate_standard_signal();
	return standard_signal_max;
}

int window_signal_get_border()	{
	calculate_standard_signal();
	return border;
}

int window_signal_get_signal_max_width()	{
	return STANDARD_SIGNAL_MAX;
}

void window_signal_show(void)	{
	gtk_widget_show_all(window_signal);
	if(is_stop==TRUE)	 {
		g_print("init_gc\n");
		init_gc();
		pthread_create(&thread_music, NULL, &play_music, NULL);
	
		//draw rectangle with standard background of draw_area widget
		//Der Index 0 ist einfach ausprobiert, die stehen fuer unterschiedlichen Status des Widgets!
		gdk_draw_rectangle(pixmap, draw_area->style->bg_gc[0], TRUE, 0, 0, 2*DRAW_AREA_WIDTH, DRAW_AREA_HEIGHT); // make screen in default color 
		/*gdk_draw_line(pixmap, gcbl, 1, 0, 1, DRAW_AREA_HEIGHT-3);
		gdk_draw_line(pixmap, gcbl, 0, DRAW_AREA_HEIGHT-3, DRAW_AREA_WIDTH-3, DRAW_AREA_HEIGHT-3); 
		gdk_draw_line(pixmap, gcbl, 0, 1, DRAW_AREA_WIDTH-3, 1);
		gdk_draw_line(pixmap, gcbl, DRAW_AREA_WIDTH-3, 0, DRAW_AREA_WIDTH-3, DRAW_AREA_HEIGHT-3);
		gdk_draw_line(pixmap, gcbl, 10, 10, 20, 20);*/
	
		//uart = fopen("/dev/ttyAMA0", "r");
		uart2_is_start = TRUE;
		uart2 = fopen("/dev/ttyAMA0", "w");
		//pthread_create(&thread_send, NULL, &uart_send, NULL);
		
		fputc(0x55, uart2);
		fputc(0x44, uart2);
		fputc(0x33, uart2);
		fputc((unsigned char)calibration_min, uart2);
		fputc((unsigned char)calibration_max, uart2);
		fputc((unsigned char)(uart_received_multiplier*100), uart2);
		fputc(100, uart2); /* divider of uart_received_multiplier */
		fputc(uart_send_multiplier, uart2);
		fputc(50, uart2);
		fputc((unsigned char)VARIATION_NUMBER, uart2);
		fputc((unsigned char)VARIATION_OFFSET, uart2);
		fputc((unsigned char)STANDARD_SIGNAL_LOW, uart2);
		fputc((unsigned char)STANDARD_SIGNAL_HIGH, uart2);
		fputc(0x33, uart2);
		fputc(0x44, uart2);
		fputc(0x55, uart2);
		fflush(uart2);
		fputc(0x33, uart2);
		fflush(uart2);
		fputc(0x11, uart2);
		fflush(uart2);
	
	
		char* music_file = strrchr(music_path, '/');
		g_print("%s\n", music_file);
		gtk_label_set_text(GTK_LABEL(lbl_music_file), music_file+1);

		system("echo . > /tmp/cmd"); /* start music */
		g_print("tmp/cmd written\n");
	}
}

/* start happens at init_gc! */
//void window_signal_start(void)	{
//	is_stop=FALSE;
//}

int window_signal_is_stop(void)	{
	return is_stop;
}	

void window_signal_hide(void)	{
	gtk_widget_hide_all(window_signal);
}

void*  uart_receive(void* parameter)	{
	//while(is_stop==FALSE)	{
	while(1)	{
		fscanf(uart, "%d", &received);
		//g_print("received: %d\n", received);
		//do	{
			//received = fgetc(uart);
		//} while(received==EOF);//has to be tested for binary values (is the code correct?)
		//g_print("%d\n", received);
		if(is_stop==FALSE && is_pause==FALSE)	{
			count_new++;
			if(count_new>=BUFFER_X)	{
				count_new=0;
			}
			//measure_signal[count_new/*+1*/]=(int)((float)(received-calibration_min+SIGNAL_BASE_LINE)/255.0/**100*/*uart_received_multiplier); // +1 is needed so that the plotting routine plots only the line from the old value to the actual value and 
			//measure_signal[count_new/*+1*/]=(int)((float)(received)/*/255.0/*100*/*uart_received_multiplier); // +1 is needed so that the plotting routine plots only the line from the old value to the actual value and 
			measure_signal[count_new]=(int)(((float)(received)-calibration_min)*(STANDARD_SIGNAL_HIGH-STANDARD_SIGNAL_LOW)/((float)(calibration_max-calibration_min)*uart_received_multiplier)+STANDARD_SIGNAL_LOW);


			//measure_signal[count_new+1]=received; // +1 is needed so that the plotting routine plots only the line from the old value to the actual value and 
			/* +1 causes a problem that signal goes down each time because coun=0 is never written!! */
			if(measure_signal[count_new]>100)	{
				measure_signal[count_new]=100;
			}
		}
		if(value_function_is_set==TRUE)	{
			//g_print("received2: %d\n", received);
			(*value_function)(received);
		}
		

		//g_print("%d\n", measure_signal[count_new+1]);
		// not from the actual value to a value it doesn't now!
//		static int c_send=0;
//		if(c_send>10)	{
//			//pthread_t thread_send;
			//pthread_create(&thread_send, NULL, &uart_send, NULL);
//			c_send=0;
//		}
//		c_send++;
	 	int	variation_av=0;
		char i;
		for(i = 0; i<VARIATION_NUMBER; i++)	{
			variation_av+=variation[i];
		}
		variation_av=variation_av/VARIATION_NUMBER;//-VARIATION_OFFSET;
		if(variation_av<0)	{
			variation_av = 0;
			//write(uart, &variation_average, 1);
		}
		variation_average=variation_av;
		//fputc((char)variation_average, uart);
		g_print("average: %d\t\t variation: %d\n", variation_average, variation[variation_count]);
		fflush(stdout);
	
		//pthread_cancel(thread_send);
		//return;

	}
}

void window_signal_set_calibration_max(int cal_max)	{
	calibration_max = cal_max;
}

void window_signal_set_calibration_min(int cal_min)	{
	calibration_min = cal_min;
}

void window_signal_set_value_function(void (*func)(int))	{
	value_function_is_set = TRUE;
	value_function =func;
}

void* uart_send(void* par)	{
//	int variation_average=0;
//	char i;
//	for(i = 0; i<VARIATION_NUMBER; i++)	{
//		variation_average+=variation[i];
//	}
//	variation_average=variation_average/VARIATION_NUMBER;//-VARIATION_OFFSET;
//	if(variation_average<0)	{
//		variation_average = 0;
//		//write(uart, &variation_average, 1);
//	}
////	fputc((char)variation_average, uart);
//	g_print("average: %d\n", variation_average);
//
//	pthread_cancel(thread_send);
//	return;
	//while(is_stop==FALSE)	{
		//fputc((char) variation_average, uart);
		//usleep(200);
	//}
}

void* play_music(void* par)	{
	while(1)	{
		// because I don't know how to end a thread from outside the thread
		// -> the variable is_stop is used to determine whether a new song should be started!
		if(is_stop==FALSE)	{
			g_print("music starts");
			//system("aplay /home/pi/Musik/wie_long.wav");
			//system("echo . > /tmp/cmd");
			window_music_umount_stick();
			window_music_mount_stick();
			system("killall omxplayer.bin");
			char omxplayer_command[MAX_PATH_LENGTH+60]="omxplayer \"";
			strcat(omxplayer_command, music_path);
			strcat(omxplayer_command, "\" < /tmp/cmd > /dev/null 2>&1");
			system(omxplayer_command);
			g_print("omx started\n");
			window_music_umount_stick();
			g_print("music over");
		}
		else	{
			break;
		}
	}
}

void window_signal_set_music(const char* set_music_path)	{
	strcpy(music_path, set_music_path);
}

void change_volume(int vol)	{
	char string[50];
	snprintf(string, 50, "amixer sset PCM %d%%", vol);
	system(string);
	return;
}



gboolean expose_event_callback(GtkWidget *widget, GdkEventExpose *event, gpointer data)	{
	/* gdk_draw_line(widget->window, gc, 10, 0, 20, 50);
	//gdk_draw_line(widget->window, widget->style->fg_gc[gtk_widget_get_state(widget)], 10, 0, 20, 50);
	gdk_draw_line(widget->window, widget->style->fg_gc[gtk_widget_get_state(widget)], 0, 0, 10, 10);
	gdk_draw_arc (widget->window,
		widget->style->fg_gc[gtk_widget_get_state (widget)],
		TRUE,
		0, 0, widget->allocation.width, widget->allocation.height,
		0, 64 * 360); */
	gdk_draw_drawable(widget->window, gc, pixmap, 0, 0, 0, 0, draw_area_size.width, draw_area_size.height);
}

static void calculate_sensitivity(void)	{
	uart_send_multiplier = (float)(sensitivity)/SENSITIVITY_MAX*(UART_SEND_MULTIPLIER_MAX-UART_SEND_MULTIPLIER_MIN)+UART_SEND_MULTIPLIER_MIN;
	g_print("uart_send: %d\n", uart_send_multiplier);
}

static void calculate_difficulty(void)	{
	//uart_received_multiplier = (float)(DIFFICULTY_MAX-difficulty+1)/DIFFICULTY_MAX*((float)STANDARD_SIGNAL_HIGH/calibration_max*0.8*CALIBRATION_MAX_OVERSIZED)+calibration_max*0.2;
	//uart_received_multiplier = (float)(DIFFICULTY_MAX-difficulty+1)/DIFFICULTY_MAX*(UART_RECEIVED_MULTIPLIER_MAX-UART_RECEIVED_MULTIPLIER_MIN)+UART_RECEIVED_MULTIPLIER_MIN;
	//uart_received_multiplier = (float)(
	uart_received_multiplier = difficulty/(float)(DIFFICULTY_MAX)*(CALIBRATION_MAX_OVERSIZED-CALIBRATION_MAX_UNDERSIZED)+CALIBRATION_MAX_UNDERSIZED;
	g_print("difficulty: %f\nDIFF_MAX: %d\nCALIB_MAX: %d\nCAL_MAX_UNDER: %d\n", difficulty, DIFFICULTY_MAX, CALIBRATION_MAX_OVERSIZED, CALIBRATION_MAX_UNDERSIZED);
	g_print("calibr_max: %d\ncalibrat_min: %d\n", calibration_max, calibration_min);

	g_print("received_multi: %f\n", uart_received_multiplier);
}

static void calculate_standard_signal(void)	{
	int i;
	standard_signal_max = (STANDARD_SIGNAL_MAX-STANDARD_SIGNAL_MIN)*(SPEED_MAX-(speed-1))/SPEED_MAX+STANDARD_SIGNAL_MIN;
	if(standard_signal_max>=STANDARD_SIGNAL_MAX)	{
		standard_signal_max = STANDARD_SIGNAL_MAX;
		g_print("warning standard_signal_max\n");
	}
	else if(standard_signal_max<=STANDARD_SIGNAL_MIN)	{
		standard_signal_max = STANDARD_SIGNAL_MIN;
		g_print("warning standard_signal_min\n");
	}
	border = (float)(duty)/(DUTY_MAX+1)*standard_signal_max; /*DUTY_MAX+2 and duty+1 so that it cannot always be high!)*/
	g_print("border: %d\n", border);
	g_print("standard: %d\n", standard_signal_max);
	for (i=0; i<border; i++)	{
		standard_signal[i] = STANDARD_SIGNAL_HIGH;
	}
	for (i=border; i<standard_signal_max; i++)	{
		standard_signal[i] = STANDARD_SIGNAL_LOW;
	}
	for (i=0; i<BUFFER_X; i++)	{
		measure_signal[i]=0;
	}

}


static void cmd_mute_clicked(GtkWidget* widget, gpointer data)	{
	if(mute==TRUE)	{
		mute=FALSE;
		image_volume_on = GTK_WIDGET(gtk_image_new_from_file("/home/pi/gtk_drawing/lautsprecher_mit.png"));
		gtk_button_set_image(GTK_BUTTON(cmd_mute), image_volume_on);
		//change_volume(volume);
		if(is_pause==FALSE)	{
			system("echo -n p > /tmp/cmd");
			fputc(0x33, uart2); 
			fflush(uart2);
		}
	}
	else	{
		mute=TRUE;
		image_volume_off = GTK_WIDGET(gtk_image_new_from_file("/home/pi/gtk_drawing/lautsprecher_ohne.png"));
		gtk_button_set_image(GTK_BUTTON(cmd_mute), image_volume_off);
		if(is_pause==FALSE)	{
			system("echo -n p > /tmp/cmd");
			fputc(0x44, uart2); /* 0x44 switches of the sound but lets values send! *//* 0x22 switches off the values and the sound */
			fflush(uart2);
		}
		//change_volume(0);
	}
	return;
}

static void vscroll_volume_value_changed(GtkWidget* widget, gpointer data)	{
	double value = VSCROLL_VOLUME_MAX - gtk_range_get_value(GTK_RANGE(vscroll_volume));
	volume = VOLUME_MAX/VSCROLL_VOLUME_MAX*value;
	//pthread_t thread_change_volume;
	//pthread_create(&thread_change_volume, NULL, &change_volume, NULL);
	//with thread there are problems because of too many threads (when scrolling the scrollbar fast)
	//-> next timer program that the thread will wait or something like that so that the main loop doesn't 
	//has to make the complicated system call with amixer!!
	//then the call of change_volume has to be changed for threads (void*) function(void*)
	if(mute==FALSE)	{
		change_volume(volume);
	}
}

static void cmd_stop_clicked(GtkWidget* widget, gpointer data)	{
	if(is_stop==FALSE)	{
		is_stop=TRUE;
		//gtk_button_set_label(GTK_BUTTON(cmd_stop), "Start");
		//system("killall aplay");
		g_print("kill omxplayer!!\n");
		system("echo -n q > /tmp/cmd");
		fputc(0x22, uart2);
		fflush(uart2);
	//	system("killall omxplayer.bin");
		g_print("vor hide\n");
		window_main_show();
		g_print("hide2\n");
		window_signal_hide();
		g_print("hide3\n");
		window_signal_uart_stop();
		g_print("während hide\n");
		//window_main_hide();
		g_print("nach hide\n");
		//window_main_show();
		pthread_cancel(play_music);
	}	
	else	{
		is_stop=FALSE;
		gtk_button_set_label(GTK_BUTTON(cmd_stop), "Stop");
	}
}

	static void cmd_pause_clicked(GtkWidget* widget, gpointer data)	{
	if(is_pause==FALSE)	{
		is_pause=TRUE;
		if(mute==FALSE)	{
			system("echo -n p > /tmp/cmd");
			fputc(0x22, uart2);
			fflush(uart2);
		}
		gtk_button_set_label(GTK_BUTTON(cmd_pause), "Play");
	}
	else	{
		is_pause=FALSE;
		if(mute==FALSE)	{
			system("echo -n p > /tmp/cmd");
			fputc(0x33, uart2);
			fflush(uart2);
		}
		else	{
			fputc(0x44, uart2);
			fflush(uart2);
		}
		gtk_button_set_label(GTK_BUTTON(cmd_pause), "Pause");
	}
}


