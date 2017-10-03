#include "project.h"
#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define TOP_SPACE 		10
#define BOTTOM_SPACE 	10
#define LEFT_SPACE		10
#define RIGHT_SPACE 	10

#define WIDGET_SPACE 	3
#define BUTTON_SPACE 	WIDGET_SPACE

#define NUMBER_BUTTONS	CMD_MUSIC_VERT
#define BUTTON_HEIGHT	((SCREENY-TOP_SPACE-BOTTOM_SPACE-LBL_MUSIC_HEIGHT-2*WIDGET_SPACE-CMD_BACK_HEIGHT-LBL_PATH_HEIGHT)/NUMBER_BUTTONS-BUTTON_SPACE) /* -10 for optical reasons so that cmd_back and hscroll_duty is not so nearby!) */
#define BUTTON_WIDTH	(SCREENX-LEFT_SPACE-RIGHT_SPACE)

#define LBL_MUSIC_HEIGHT 34 // find it out by trying out!
#define LBL_MUSIC_WIDTH  no_value // don't really know the exact value

#define LBL_PATH_HEIGHT	15
#define LBL_PATH_WIDTH	SCREENX-LEFT_SPACE-RIGHT_SPACE-CMD_UP_WIDTH-WIDGET_SPACE-CMD_INT_EXT_WIDTH-WIDGET_SPACE

#define CMD_INT_EXT_WIDTH	50
#define CMD_INT_EXT_HEIGHT	CMD_UP_HEIGHT

#define CMD_UP_HEIGHT	(LBL_PATH_HEIGHT+LBL_MUSIC_HEIGHT-CMD_UP_LBL_MUSIC_OFFSET)
#define CMD_UP_WIDTH	35
#define CMD_UP_LBL_MUSIC_OFFSET	10

#define CMD_MUSIC_WIDTH		(BUTTON_WIDTH-WIDGET_SPACE)/CMD_MUSIC_HOR
#define CMD_MUSIC_HEIGHT	BUTTON_HEIGHT 

#define CMD_LEFT_WIDTH		35
#define CMD_LEFT_HEIGHT		CMD_HOME_HEIGHT

#define CMD_RIGHT_WIDTH		CMD_LEFT_WIDTH
#define CMD_RIGHT_HEIGHT	CMD_LEFT_HEIGHT

#define LBL_MUSIC_POSX	LEFT_SPACE
#define LBL_MUSIC_POSY	TOP_SPACE

#define LBL_PATH_POSX	LEFT_SPACE
#define LBL_PATH_POSY	LBL_MUSIC_POSY+LBL_MUSIC_HEIGHT

#define CMD_INT_EXT_POSX	LBL_PATH_POSX+LBL_PATH_WIDTH+WIDGET_SPACE
#define CMD_INT_EXT_POSY	CMD_UP_POSY

#define CMD_UP_POSX		CMD_INT_EXT_POSX+CMD_INT_EXT_WIDTH+WIDGET_SPACE
#define CMD_UP_POSY		LBL_MUSIC_POSY+CMD_UP_LBL_MUSIC_OFFSET

#define CMD_MUSIC_INIT_POSX	LBL_PATH_POSX
#define CMD_MUSIC_INIT_POSY	LBL_PATH_POSY+LBL_PATH_HEIGHT+WIDGET_SPACE

#define CMD_MUSIC_HOR	2 /* horizontal number of music choice buttons */
#define CMD_MUSIC_VERT	3 /* vertical number of music choice buttons */
#define CMD_MUSIC_NUMBER	CMD_MUSIC_HOR*CMD_MUSIC_VERT

#define CMD_LEFT_POSX	SCREENX-RIGHT_SPACE-CMD_LEFT_WIDTH-CMD_RIGHT_WIDTH-WIDGET_SPACE
#define CMD_LEFT_POSY	CMD_HOME_POSY

#define CMD_RIGHT_POSX	CMD_LEFT_POSX+CMD_LEFT_WIDTH+WIDGET_SPACE
#define CMD_RIGHT_POSY	CMD_LEFT_POSY

#define STICK_FOUND		0
#define NO_STICK_FOUND	1

#define DIR_COLOR		"blue"
#define MP3_COLOR		"black"
#define DIR_UP_COLOR	"orange"
#define DIR_UP_STRING	"<Directory up>"
#define DIR_UP_STRING_PANGO "&lt;Directory up&gt;"

#define STICK_PATH		"/home/pi/stick"
#define INTERN_PATH		"/home/pi/music"

#define MAX_DIR_LEVEL	20

#define INT	0
#define EXT	1

#define TIMER_TEST_STICK	1	 /* in seconds */



/* function declarations */

static void cmd_back_clicked(GtkWidget*, gpointer);
static void cmd_home_clicked(GtkWidget*, gpointer);
static void cmd_left_clicked(GtkWidget*, gpointer);
static void cmd_right_clicked(GtkWidget*, gpointer);
static void cmd_music_clicked(GtkWidget*, gpointer);
static void cmd_up_clicked(GtkWidget*, gpointer);
static void cmd_int_ext_clicked(GtkWidget*, gpointer);
static void go_back(void);
static void to_intern(void);
static void to_extern(void);
void* thread_test_stick_execute(void*);
void umount_stick(void);
int read_dir(char*, int);
int mount_stick(void);


/* variable declarations */

GtkWidget* window_music;
GtkWidget* fixed_music;
GtkWidget* lbl_music;
GtkWidget* lbl_path;
GtkWidget* cmd_int_ext;
GtkWidget* cmd_up;
GtkWidget* image_up;
GtkWidget* cmd_right;
GtkWidget* cmd_left;
GtkWidget* cmd_home;
GtkWidget* cmd_back;
GtkWidget* cmd_music[CMD_MUSIC_HOR*CMD_MUSIC_VERT];
GtkWidget* lbl_cmd_music[CMD_MUSIC_HOR*CMD_MUSIC_VERT];

pthread_t thread_test_stick;

char actual_path[MAX_PATH_LENGTH];
char stick_path[MAX_PATH_LENGTH];
int actual_page[MAX_DIR_LEVEL];
int actual_page_count=0;

char slash[2]="/";
int int_ext=INT;

volatile int is_shown=FALSE;
volatile int should_test_stick=FALSE;

void window_music_init()	{

	char convert[1000];
	snprintf(convert, 1000, "convert -verbose /home/pi/gtk_drawing/dir_up_orig.png -scale %dx%d! /home/pi/gtk_drawing/dir_up.png", CMD_UP_WIDTH/2, CMD_UP_HEIGHT/2);
	g_print("system: %s", convert);
	system(convert);


	window_music = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_default_size(GTK_WINDOW(window_music), SCREENX, SCREENY);

	fixed_music = gtk_fixed_new();

	cmd_back = gtk_button_new_with_label("Back");
	g_signal_connect(G_OBJECT(cmd_back), "clicked", G_CALLBACK(cmd_back_clicked), NULL);

	cmd_home = gtk_button_new_with_label("Home");
	g_signal_connect(G_OBJECT(cmd_home), "clicked", G_CALLBACK(cmd_home_clicked), NULL);

	cmd_left = gtk_button_new_with_label("<-");
	g_signal_connect(G_OBJECT(cmd_left), "clicked", G_CALLBACK(cmd_left_clicked), NULL);

	cmd_right = gtk_button_new_with_label("->");
	g_signal_connect(G_OBJECT(cmd_right), "clicked", G_CALLBACK(cmd_right_clicked), NULL);

	lbl_music = gtk_label_new("Music");
	gtk_label_set_markup(GTK_LABEL(lbl_music), "<span font=\'Liberation Sans 22\'>Music</span>");
	/* change this font also below in function window_music_show */
	/* change also in cmd_int_ext_clicked function */

	lbl_path = gtk_label_new("");
	gtk_misc_set_alignment(GTK_MISC(lbl_path), 0, 0);
	
	image_up = gtk_image_new_from_file("/home/pi/gtk_drawing/dir_up.png");

	cmd_up = gtk_button_new();
	gtk_button_set_image(GTK_BUTTON(cmd_up), image_up);
	g_signal_connect(G_OBJECT(cmd_up), "clicked", G_CALLBACK(cmd_up_clicked), NULL);

	cmd_int_ext = gtk_button_new_with_label("");
	g_signal_connect(G_OBJECT(cmd_int_ext), "clicked", G_CALLBACK(cmd_int_ext_clicked), NULL);

	int v, h;
	for(v=0; v<CMD_MUSIC_VERT; v++)	{
		for(h=0; h<CMD_MUSIC_HOR; h++)	{
			cmd_music[CMD_MUSIC_HOR*v+h]=gtk_button_new();
			lbl_cmd_music[CMD_MUSIC_HOR*v+h]=gtk_label_new("");
			gtk_widget_set_size_request(cmd_music[CMD_MUSIC_HOR*v+h], CMD_MUSIC_WIDTH, CMD_MUSIC_HEIGHT);
			gtk_fixed_put(GTK_FIXED(fixed_music), cmd_music[CMD_MUSIC_HOR*v+h], CMD_MUSIC_INIT_POSX+h*(CMD_MUSIC_WIDTH+WIDGET_SPACE), CMD_MUSIC_INIT_POSY+v*(CMD_MUSIC_HEIGHT+WIDGET_SPACE));
			gtk_container_add(GTK_CONTAINER(cmd_music[CMD_MUSIC_HOR*v+h]), lbl_cmd_music[CMD_MUSIC_HOR*v+h]);
			g_signal_connect(G_OBJECT(cmd_music[CMD_MUSIC_HOR*v+h]), "clicked", G_CALLBACK(cmd_music_clicked), NULL);
		}
	}

	gtk_widget_set_size_request(cmd_back, CMD_BACK_WIDTH, CMD_BACK_HEIGHT);
	gtk_widget_set_size_request(cmd_home, CMD_HOME_WIDTH, CMD_HOME_HEIGHT);
	gtk_widget_set_size_request(cmd_left, CMD_LEFT_WIDTH, CMD_LEFT_HEIGHT);
	gtk_widget_set_size_request(cmd_right, CMD_RIGHT_WIDTH, CMD_RIGHT_HEIGHT);
	gtk_widget_set_size_request(lbl_path, LBL_PATH_WIDTH, LBL_PATH_HEIGHT);
	gtk_widget_set_size_request(cmd_up, CMD_UP_WIDTH, CMD_UP_HEIGHT);
	gtk_widget_set_size_request(cmd_int_ext, CMD_INT_EXT_WIDTH, CMD_INT_EXT_HEIGHT);

	gtk_fixed_put(GTK_FIXED(fixed_music), lbl_music, LBL_MUSIC_POSX, LBL_MUSIC_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_back, CMD_BACK_POSX, CMD_BACK_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_home, CMD_HOME_POSX, CMD_HOME_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_left, CMD_LEFT_POSX, CMD_LEFT_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_right, CMD_RIGHT_POSX, CMD_RIGHT_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_up, CMD_UP_POSX, CMD_UP_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), lbl_path, LBL_PATH_POSX, LBL_PATH_POSY);
	gtk_fixed_put(GTK_FIXED(fixed_music), cmd_int_ext, CMD_INT_EXT_POSX, CMD_INT_EXT_POSY);

	gtk_container_add(GTK_CONTAINER(window_music), fixed_music);
}

void window_music_show(void)	{
	pthread_create(&thread_test_stick, NULL, thread_test_stick_execute, NULL);
	is_shown=TRUE;
	umount_stick();
	actual_page_count=0;
	actual_page[actual_page_count]=0;
	if(mount_stick()==STICK_FOUND)	{
		strcpy(stick_path, STICK_PATH);
		gtk_label_set_markup(GTK_LABEL(lbl_music), "<span font=\'Liberation Sans 22\'>Music</span><span font=\'Liberation Sans 15\'> (USB drive)</span>");
		gtk_button_set_label(GTK_BUTTON(cmd_int_ext), "Intern");
		gtk_widget_set_sensitive(cmd_int_ext, TRUE);
		int_ext=EXT;
	}
	else	{
		strcpy(stick_path, INTERN_PATH);
		gtk_label_set_markup(GTK_LABEL(lbl_music), "<span font=\'Liberation Sans 22\'>Music</span><span font=\'Liberation Sans 15\'> (Internal drive)</span>");
		gtk_button_set_label(GTK_BUTTON(cmd_int_ext), "USB");
		int_ext=INT;
		gtk_widget_set_sensitive(cmd_int_ext, FALSE);
	}
	strcpy(actual_path, stick_path);

	if(read_dir(stick_path, actual_page[actual_page_count])==FALSE)	{
		gtk_widget_set_sensitive(cmd_right, FALSE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_right, TRUE);
	}
	gtk_widget_set_sensitive(cmd_left, FALSE);

	gtk_widget_show_all(window_music);
	umount_stick();
}

void window_music_hide(void)	{
	pthread_cancel(thread_test_stick);
	is_shown=FALSE;
	umount_stick();
	gtk_widget_hide_all(window_music);
}

void window_music_mount_stick(void)	{
	mount_stick();
}

void window_music_umount_stick(void)	{
	umount_stick();
}

int mount_stick(void)	{
	char stick[2][80]={
		"sudo mount /dev/sda /home/pi/stick -o ro", 
		"sudo mount /dev/sda1 /home/pi/stick -o ro"};
//		"sudo mount /dev/sda2 /home/pi/stick -o ro",
//		"sudo mount /dev/sdb /home/pi/stick -o ro",
//		"sudo mount /dev/sdb1 /home/pi/stick -o ro",
//		"sudo mount /dev/sdb2 /home/pi/stick -o ro"};
	int i;
	for(i=0; i<2; i++)	{
		if(system(stick[i])==0)	{
			return STICK_FOUND;
		}
	}
	return NO_STICK_FOUND;
}

int read_dir(char* path, int page)	{
	/* path must not have a slash at the end! */
	/* return value: TRUE if there can be another page, FALSE if this is the last page */
	if(strstr(path, STICK_PATH)==path)	{
		umount_stick();
		if(mount_stick()==STICK_FOUND)	{
	
		}
		else	{
			to_intern();
			strcpy(path, actual_path);
		}
	 }

	char file_path[MAX_PATH_LENGTH];
	
	int markup_is_set=FALSE;
	int further_page=FALSE;

	int i;
	for(i=0; i<CMD_MUSIC_NUMBER; i++)	{
		gtk_label_set_markup(GTK_LABEL(lbl_cmd_music[i]), "");
	}
	DIR* dir;
	struct dirent* dir_entry;
	dir = opendir(path);

	if(strcmp(actual_path, stick_path)==0)	{
		gtk_label_set_text(lbl_path, "/");
	}
	else	{
		gtk_label_set_text(lbl_path, strlen(stick_path)+actual_path);
	}

	i=-page*CMD_MUSIC_NUMBER;

//	if(i==0 && page==0 && strcmp(actual_path, stick_path)!=0)	{
//				char markup[200+MAX_FILE_NAME_LENGTH];
//				strcpy(markup, "<span color=\"");
//				strcat(markup, DIR_UP_COLOR);
//				strcat(markup, "\">");
//				strcat(markup, DIR_UP_STRING_PANGO);
//				strcat(markup, "</span>");
//				if(i>=0 && i<CMD_MUSIC_NUMBER)	{
//					gtk_label_set_markup(GTK_LABEL(lbl_cmd_music[i]), markup);
//				}
//				markup_is_set=TRUE;
//				i++;
//	}
	if(strcmp(actual_path, stick_path)>0)	{
		gtk_widget_set_sensitive(cmd_up, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_up, FALSE);
	}

	dir_entry=readdir(dir);
	while(dir_entry!=0)	{
		markup_is_set=FALSE;
		if(strcmp(dir_entry->d_name, ".")!=0 && strcmp(dir_entry->d_name, "..")!=0)	{
			strcpy(file_path, path);
			strcat(file_path, slash);
			strcat(file_path, dir_entry->d_name);
			struct stat file_stat;
			stat(file_path, &file_stat);
			if((file_stat.st_mode&S_IFDIR)!=0)	{
				/* it is a directory */
				/* file_path is used now to save the pango-markup-language for the labels! */
				strcpy(file_path, "<span color=\"");
				strcat(file_path, DIR_COLOR);
				strcat(file_path, "\">");
				strcat(file_path, g_markup_escape_text(dir_entry->d_name, -1));
				strcat(file_path, "</span>");
				if(i>=0 && i<CMD_MUSIC_NUMBER)	{
					gtk_label_set_markup(GTK_LABEL(lbl_cmd_music[i]), file_path);
				}
				markup_is_set=TRUE;
				i++;
			}
			else	{
				/* it is a normal file */
				int len = strlen(dir_entry->d_name);	
				if(strcmp(dir_entry->d_name+len-4, ".mp3")==0 || strcmp(dir_entry->d_name+len-4, ".MP3")==0)	{
					/* it is mp3-file */
					strcpy(file_path, "<span color=\"");
					strcat(file_path, MP3_COLOR);
					strcat(file_path, "\">");
					strcat(file_path, g_markup_escape_text(dir_entry->d_name,-1));
					strcat(file_path, "</span>");
					if(i>=0 && i<CMD_MUSIC_NUMBER)	{
						gtk_label_set_markup(GTK_LABEL(lbl_cmd_music[i]), file_path);
					}
					markup_is_set=TRUE;
					i++;
				}
			}
		}
		if(i>(CMD_MUSIC_NUMBER+0) && markup_is_set==TRUE)	{
			/* to check if there would be another element */
			further_page=TRUE;
			break;
		}
		dir_entry=readdir(dir);
	}
	int c;
	for(c=0; c<i; c++)	{
		gtk_widget_set_sensitive(cmd_music[c], TRUE);
	}
	for(c=i; c<CMD_MUSIC_NUMBER; c++)	{
		gtk_widget_set_sensitive(cmd_music[c], FALSE);
	}

	closedir(dir);

	return further_page;
}


void umount_stick(void)	{
	system("sudo umount -f /home/pi/stick");
}

static void go_back()	{
	window_music_hide();
	window_settings_show();
}

static void cmd_back_clicked(GtkWidget* widget, gpointer data)	{
	go_back();
}

static void cmd_home_clicked(GtkWidget* widget, gpointer data)	{
	window_music_hide();
	window_settings_hide();
	window_main_show();
}

static void cmd_right_clicked(GtkWidget* widget, gpointer data)	{
	actual_page[actual_page_count]++;
	if(read_dir(actual_path, actual_page[actual_page_count])==FALSE)	{
		gtk_widget_set_sensitive(cmd_right, FALSE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_right, TRUE);
	}
	if(actual_page[actual_page_count]>0)	{
		gtk_widget_set_sensitive(cmd_left, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_left, FALSE);
	}
}

static void cmd_left_clicked(GtkWidget* widget, gpointer data)	{
	if(actual_page[actual_page_count]>0)	{
		actual_page[actual_page_count]--;
	}
	if(read_dir(actual_path, actual_page[actual_page_count])==TRUE)	{
		gtk_widget_set_sensitive(cmd_right, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_right, FALSE);
	}
	if(actual_page[actual_page_count]>0)	{
		gtk_widget_set_sensitive(cmd_left, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_left, FALSE);
	}
}

static void cmd_music_clicked(GtkWidget* widget, gpointer data)	{
	int i;
	for(i=0; i<CMD_MUSIC_NUMBER; i++)	{
		if(widget==cmd_music[i])	{
			break;
		}
	}
	g_print("%s\n", gtk_label_get_text(GTK_LABEL(lbl_cmd_music[i])));	
	char* lbl_text = gtk_label_get_text(GTK_LABEL(lbl_cmd_music[i]));

	char tmp_path[MAX_PATH_LENGTH];
	//if(strcmp(lbl_text, DIR_UP_STRING)==0)	{
	//	if(strcmp(actual_path, stick_path)>0)	{
	//		/* change one dir up */
	//		char* last_slash = strrchr(actual_path, '/');
	//		if(last_slash!=0)	{
	//			*last_slash=0; /* end string one directory higher -> overwrite the last slash with zero */
	//			actual_page[actual_page_count]=0;
	//			if(actual_page_count>0)	{
	//				actual_page_count--;
	//			}
	//		}
	//	}
	//}
	//else	{
	strcpy(tmp_path, actual_path);
	strcat(tmp_path, slash);
	strcat(tmp_path, gtk_label_get_text(GTK_LABEL(lbl_cmd_music[i])));
	
	//umount_stick();
	if((mount_stick()==STICK_FOUND&&int_ext==EXT) || int_ext==INT)	{
		struct stat file_stat;
		stat(tmp_path, &file_stat);
		g_print("%x\n", file_stat.st_mode);
		g_print("%x\n", S_IFDIR);
		
		if((file_stat.st_mode&S_IFDIR)!=0)	{
			/* is directory -> change to this directory */
			strcpy(actual_path, tmp_path);
			if(actual_page_count<(MAX_DIR_LEVEL-1))	{
				actual_page_count++;
			}
			actual_page[actual_page_count]=0;
		}
		else	{
			/* it is a music file */
			strcpy(tmp_path, actual_path);
			strcat(tmp_path, slash);
			strcat(tmp_path, lbl_text);
			window_signal_set_music(tmp_path);
			go_back();
			return;
		}
		//}
		if(read_dir(actual_path, actual_page[actual_page_count])==FALSE)	{
			gtk_widget_set_sensitive(cmd_right, FALSE);
		}
		else	{
			gtk_widget_set_sensitive(cmd_right, TRUE);
		}
		if(actual_page[actual_page_count]>0)	{
			gtk_widget_set_sensitive(cmd_left, TRUE);
		}
		else	{
			gtk_widget_set_sensitive(cmd_left, FALSE);
		}
	}	
	umount_stick();
}    	
     	
static void cmd_up_clicked(GtkWidget* widget, gpointer data)	{
	if(strcmp(actual_path, stick_path)>0)	{
	    /* change one dir up */
	    char* last_slash = strrchr(actual_path, '/');
	    if(last_slash!=0)	{
	    	*last_slash=0; /* end string one directory higher -> overwrite the last slash with zero */
	    	actual_page[actual_page_count]=0;
	    	if(actual_page_count>0)	{
	    		actual_page_count--;
	    	}
	    }
	}
	if(read_dir(actual_path, actual_page[actual_page_count])==FALSE)	{
		gtk_widget_set_sensitive(cmd_right, FALSE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_right, TRUE);
	}
	if(actual_page[actual_page_count]>0)	{
		gtk_widget_set_sensitive(cmd_left, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_left, FALSE);
	}
}

static void cmd_int_ext_clicked(GtkWidget* widget, gpointer data)	{
	if(int_ext==INT)	{
		to_extern();
	}
	else	{
		to_intern();
	}
	umount_stick();
}

static void to_intern()	{
	strcpy(actual_path, INTERN_PATH);
	strcpy(stick_path, actual_path);
	actual_page_count=0;
	actual_page[actual_page_count]=0;
	umount_stick();
	if(read_dir(actual_path, actual_page[actual_page_count])==FALSE)	{
			gtk_widget_set_sensitive(cmd_right, FALSE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_right, TRUE);
	}
	if(actual_page[actual_page_count]>0)	{
		gtk_widget_set_sensitive(cmd_left, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_left, FALSE);
	}
	
	gtk_button_set_label(GTK_BUTTON(cmd_int_ext), "USB");
	gtk_label_set_markup(GTK_LABEL(lbl_music), "<span font=\'Liberation Sans 22\'>Music</span><span font=\'Liberation Sans 15\'> (Internal drive)</span>");
	int_ext=INT;
	if(mount_stick()==STICK_FOUND)	{
		gtk_widget_set_sensitive(cmd_int_ext, TRUE);
	}
	else	{
		gtk_widget_set_sensitive(cmd_int_ext, FALSE);
	}
	umount_stick();
}

static void to_extern()	{
	umount_stick();
	if(mount_stick()==STICK_FOUND)	{
		strcpy(actual_path, STICK_PATH);
		strcpy(stick_path, actual_path);
		actual_page_count=0;
		actual_page[actual_page_count]=0;
		gtk_button_set_label(GTK_BUTTON(cmd_int_ext), "Intern");
		gtk_label_set_markup(GTK_LABEL(lbl_music), "<span font=\'Liberation Sans 22\'>Music</span><span font=\'Liberation Sans 15\'> (USB drive)</span>");
		int_ext=EXT;
		gtk_widget_set_sensitive(cmd_int_ext, TRUE);
		if(read_dir(actual_path, actual_page[actual_page_count])==FALSE)	{
			gtk_widget_set_sensitive(cmd_right, FALSE);
		}
		else	{
			gtk_widget_set_sensitive(cmd_right, TRUE);
		}
		if(actual_page[actual_page_count]>0)	{
			gtk_widget_set_sensitive(cmd_left, TRUE);
		}
		else	{
			gtk_widget_set_sensitive(cmd_left, FALSE);
		}
	
	}
}

void window_music_main(void)	{
	if(should_test_stick==TRUE)	{
		if(int_ext==INT)	{
			umount_stick();
			if(mount_stick()==STICK_FOUND)	{
				gtk_widget_set_sensitive(cmd_int_ext, TRUE);
			}
			else	{
				gtk_widget_set_sensitive(cmd_int_ext, FALSE);
			}
			umount_stick();
		}
		else	{
			umount_stick();
			if(mount_stick()!=STICK_FOUND)	{
				to_intern();
			}
			umount_stick();
		}
		should_test_stick=FALSE;
	}
}

void* thread_test_stick_execute(void* parameter)	{
	while(is_shown = TRUE)	{
		should_test_stick=TRUE;
		sleep(2);
	}
}

int window_music_is_stop(void)	{
	if(is_shown==TRUE)	{
		return FALSE;
	}
	else	{
		return TRUE;
	}
}
