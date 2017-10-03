#ifndef PROJECT_H
#define PROJECT_H

#include "window_main.h"
#include "window_settings.h"
#include "window_reference_signal.h"
#include "window_difficulty.h"
#include "window_signal.h"
#include "window_calibration.h"
#include "window_music.h"
#include "window_shutdown.h"

#define SCREENX 320
#define SCREENY 240


/* has to be changed if SCREENX and SCREENY is changed!! */
#define CMD_BACK_WIDTH	80
#define CMD_BACK_HEIGHT 	45

#define CMD_HOME_WIDTH	80
#define CMD_HOME_HEIGHT	45

#define CMD_BACK_POSX	10
#define CMD_BACK_POSY	183

#define CMD_HOME_POSX	123
#define CMD_HOME_POSY	183


#define DUTY_MAX	10 /* maximum setting of duty (of reference signal) */
#define SPEED_MAX	10

#define DIFFICULTY_MAX	10
#define SENSITIVITY_MAX	10

#define CALIBRATION_MIN_STANDARD 10
#define CALIBRATION_MAX_STANDARD 130

#define MAX_PATH_LENGTH	500
#define MAX_FILE_NAME_LENGTH	256

#endif
