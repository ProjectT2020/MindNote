#ifndef TTY_H
#define TTY_H

#include <termios.h>

struct tty_info {
    int width;
    int height;
};


// UTF-8
int utf8_prev_char_start(const char *s, int len_bytes) ;
int utf8_last_char_width(const char *s) ;
void utf8_pop_last_char(char *s) ;

// cursor and screen control
void get_tty_size(int *width_out, int *height_out);
void ui_adapter_get_terminal_size(int *width, int *height);
struct termios tty_mode_save(void);
void tty_mode_restore(struct termios original_tty_attr);
void hide_cursor(void);
void show_cursor(void);
void cursor_blink(void);
void set_cursor_position(int x, int y);

// TTY mode 
void tty_edit_mode( struct termios *tty_attr_origin) ;

void ui_adapter_enable_raw_mode(void);
void ui_adapter_disable_raw_mode(void);

// line editing
void tty_erase_chars(int cols) ;

void tty_set_timeout(int seconds) ;
void tty_no_timeout(void) ;

#endif // TTY_H
