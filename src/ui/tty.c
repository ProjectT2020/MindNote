#include <termios.h>
#include <string.h>
#include <wchar.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#include "utils/logging.h"
#include "tty.h"

static struct termios original_tty_attr;
/**
 * find the start byte of the last UTF-8 character in the string
 */
int utf8_prev_char_start(const char *s, int len_bytes) {
    if (len_bytes <= 0) return 0;
    int i = len_bytes - 1;
    // move left over continuation bytes (10xxxxxx)
    while (i > 0 && ((unsigned char)s[i] & 0xC0) == 0x80) {
        i--;
    }
    return i;
}

/**
 * delete the last UTF-8 character in the string (in-place)
 */
void utf8_pop_last_char(char *s) {
    int len = (int)strlen(s);
    if (len <= 0) return;
    int start = utf8_prev_char_start(s, len);
    s[start] = '\0';
}

int utf8_last_char_width(const char *s) {
    wchar_t wc;
    size_t len = strlen(s);
    if (len == 0) return 0;

    const char *p = s;
    const char *last = NULL;
    while (*p) {
        last = p;
        p += mblen(p, MB_CUR_MAX);
    }

    mbtowc(&wc, last, MB_CUR_MAX);
    int w = wcwidth(wc);
    return w > 0 ? w : 1;
}


void get_tty_size(int *width_out, int *height_out) {
    struct winsize winsize;
    ioctl(0, TIOCGWINSZ, &winsize);
    *width_out = winsize.ws_col;
    *height_out = winsize.ws_row;
}

void ui_adapter_get_terminal_size(int *width, int *height) {
    struct winsize winsize;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &winsize) == 0) {
        *width = winsize.ws_col;
        *height = winsize.ws_row;
    } else {
        *width = 80;
        *height = 24;
    }
}

void ui_adapter_enable_raw_mode() {
    tcgetattr(STDIN_FILENO, &original_tty_attr);
    struct termios raw = original_tty_attr;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 1; // at least 1 byte
    raw.c_cc[VTIME] = 0; // no timeout
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void ui_adapter_disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_tty_attr);
}

void tty_mode_restore(struct termios original_tty_attr_param) {
    tcsetattr(0, TCSANOW, &original_tty_attr_param);
    printf("\x1b[?2004l");
}

void hide_cursor(void) {
    printf("\033[?25l");
    fflush(stdout);
}

void show_cursor(void) {
    printf("\033[?25h");
    fflush(stdout);
}

void cursor_blink(void) {
    printf("\033[?12h");
    fflush(stdout);
}

void set_cursor_position(int x, int y) {
    log_debug("set_cursor_position: moving cursor to (%d, %d)", x, y);
    printf("\033[%d;%dH", y + 1, x + 1);
    fflush(stdout);
}


void tty_edit_mode( struct termios *tty_attr_origin) {
    tcgetattr(0, tty_attr_origin);

    struct termios tty_attr;
    tcgetattr(0, &tty_attr);
    // tty_attr.c_lflag |= (ICANON | ECHO);  // enable canonical mode and echo
    tty_attr.c_lflag &= ~(ICANON | ECHO | IEXTEN);
    tty_attr.c_cc[VMIN] = 1;              // at least 1 byte
    tty_attr.c_cc[VTIME] = 0;             // no timeout
    tcsetattr(0, TCSANOW, &tty_attr);
    // Bracketed Paste Mode
    printf("\x1b[?2004h");

    system("stty icrnl");  // map CR to NL so Enter sends '\n'
}

// line editing
void tty_erase_chars(int cols) {
    for (int i = 0; i < cols; i++) {
        write(STDOUT_FILENO, "\b \b", 3);
    }
}