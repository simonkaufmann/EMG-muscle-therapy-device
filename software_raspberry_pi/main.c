#include <gtk/gtk.h>
#include "project.h"

int main(int argc, char* argv[])	{

	gtk_init(&argc, &argv);

	window_main_init();
	window_main_show();

	window_settings_init();

	window_reference_signal_init();

	window_signal_init();

	window_difficulty_init();

	window_calibration_init();

	window_music_init();

	window_shutdown_init();

	while(1)	{
		gtk_main_iteration_do(FALSE);
		if(window_signal_is_stop()==FALSE)	{
			window_signal_main();
		}
		else if(window_calibration_is_stop()==FALSE)	{
			window_calibration_main();
		}
		else if(window_music_is_stop()==FALSE)	{
			window_music_main();
		}
	}
	//gtk_main();

}
