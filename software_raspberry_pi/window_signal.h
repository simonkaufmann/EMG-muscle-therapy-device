#ifndef WINDOW_SIGNAL_H
#define WINDOW_SIGNAL_H

void window_signal_init(void);
//void window_signal_start(void);
void window_signal_main(void);
void window_signal_show(void);
void window_signal_hide(void);
int window_signal_is_stop(void);

void window_signal_set_duty(int);
void window_signal_set_speed(int);
int window_signal_get_signal_width(void);
int window_signal_get_border(void);
int window_signal_get_signal_max_width(void);
void window_signal_set_difficulty(int);
void window_signal_set_sensitivity(int);
void window_signal_set_calibration_max(int);
void window_signal_set_calibration_min(int);

void window_signal_set_value_function(void (*func)(int)); /* give as parameter a function that is called and given the new value that was received via uart */
//void window_signal_unset_value_function(void (*func)(int));
void window_signal_set_music(const char*);

void window_signal_uart_start(void);
void window_signal_uart_stop(void);
void window_signal_uart2_stop(void);
//void window_signal_uart2_start(void);


#endif
