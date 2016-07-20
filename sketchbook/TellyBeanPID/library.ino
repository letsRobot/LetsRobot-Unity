/*
 * Arduino sketch for Telly
 * (c) Copyright 2016 Let's Robot.
 */

#include <errno.h>
#include <stdarg.h>
#include <limits.h>

#include "library.h"
#include "config.h"

/*
 * Are we trying to be compatible with the old, single-character based
 * protocol?  Start off being compatible, and add a new command, '!', to
 * break out of compatibility mode.
 */
bool compat = false;//true;

/*
 * Controls if keystrokes should be echoed back.  For humans, echo gives you a
 * command prompt.  For a programmatic interface, it's better to disable echo.
 */
bool echo = true;

/*
 * Such a kludge, for human readability
 */
bool redraw_prompt;

/***/

char format_buf[128];

char *format(const char *fmt, ... ) {
    va_list args;
    va_start (args, fmt);
    vsnprintf(format_buf, sizeof(format_buf), fmt, args);
    va_end (args);
    return format_buf;
}

void ERR(const char *fmt, ... ) {
    va_list args;
    va_start (args, fmt);
    vsnprintf(format_buf, sizeof(format_buf), fmt, args);
    va_end (args);
    Serial.print("ERR ");
    Serial.println(format_buf);
}

void OK(const char *fmt, ... ) {
    if (compat) {
        Serial.print("ok\n");
    }

    else if (!fmt) {
        Serial.println("OK");
    }

    else {
        va_list args;
        va_start (args, fmt);
        vsnprintf(format_buf, sizeof(format_buf), fmt, args);
        va_end (args);
        Serial.print("OK ");
        Serial.println(format_buf);
    }
}

void OK(void) {
    OK(NULL);
}

#define debug verbose
void verbose(const char *fmt, ... ) {
    if (compat)
        return;

    va_list args;
    va_start (args, fmt);
    vsnprintf(format_buf, sizeof(format_buf), fmt, args);
    va_end (args);
    Serial.print("DEBUG ");
    Serial.println(format_buf);
}


/***/

char getline_buf[128];

struct {
    const char key;
    const char *string;
} getline_hotkeys[] = {
    { 'B',   "back"         },
    { 'F',   "forward"      },
    { 'L',   "left"         },
    { 'R',   "right"        },
    { 'S',   "stop"         },
    { '!',   "echo on"      },
    { '\0',  NULL           }
};

char *getline() {
    size_t buf_len = 0;
    char c;
    int i;

    Serial.print("%> ");

    while (1) {
        heartbeat();

        if (redraw_prompt) {
            redraw_prompt = false;
            c = 12;
        }
        else {
            if (! Serial.available())
                continue;
            c = Serial.read();
        }

        // First check for special hotkeys, but only if this keypress is the first on the line
        if (buf_len == 0) {
            for (i = 0; getline_hotkeys[i].key; i++) {
                if (getline_hotkeys[i].key == c) {
                    strcpy(getline_buf, getline_hotkeys[i].string);
                    Serial.print(getline_buf);
                    return getline_buf;
                }
            }
        }

        // Control-U
        if (c == 21) {
            Serial.print("\r                                                                                       \r%> ");
            buf_len = 0;
            continue;
        }

        // Backspace or Delete
        if (c == 8 || c == 127) {
            if (buf_len)
                buf_len--;
            Serial.print("\r                                                                                       \r%> ");
            getline_buf[buf_len] = '\0';
            Serial.print(getline_buf);
            continue;
        }

        // Control-L
        if (c == 12) {
            Serial.print("\r                                                                                       \r%> ");
            getline_buf[buf_len] = '\0';
            Serial.print(getline_buf);
            continue;
        }

        if (c == '\r' || c == '\n') {
            break;
        }

        // Random unhandled control sequences
        if (c < 32) {
            continue;
        }

        if (buf_len < sizeof(getline_buf)-1) {
            getline_buf[buf_len++] = c;
            Serial.print(c);
        }
        else {
            Serial.print(" overflow");
        }
    }

    getline_buf[buf_len] = '\0';
    return getline_buf;
}

char *getline_noecho() {
    size_t buf_len = 0;
    char c;

    while (1) {
        heartbeat();

        if (! Serial.available())
            continue;

        c = Serial.read();

        if (buf_len == 0 && c == '!')
            return getline_fixed("echo on");

        if (c == '\r')
            break;

        if (c == '\n')
            break;

        if (buf_len < sizeof(getline_buf)-1) {
            getline_buf[buf_len++] = c;
        }
    }

    getline_buf[buf_len] = '\0';
    return getline_buf;
}

char *getline_fixed(const char *line) {
    return strcpy(getline_buf, line);
}

/***/

int nextarg_int(int min, int max, int default_val, int on_error) {
    char *arg, *endptr;
    int val;

    arg = nextarg();
    errno = 0;

    if (! arg)
        return default_val;

    val = strtol(arg, &endptr, 0);

    if (errno || endptr == arg) {
        //Serial.println(format("Conversion error: %s", arg));
        return on_error;
    }

    if (! (min <= val && val <= max)) {
        //Serial.println(format("Out of range: %d", val));
        return on_error;
    }

    return val;
}

void cmd_noop(void) {
    return;
}

/*
 * Returns a pong, plus the Teeny's uptime in seconds.  printf() on this
 * platform doesn't support floating point by default, so fake it with
 * integer arithmetic.
 */
void cmd_ping(void) {
    unsigned long now = millis();
    unsigned long seconds      = now / 1000.0;
    unsigned long milliseconds = now - seconds*1000;
    OK("pong %lu.%03lu", seconds, milliseconds);
}

void cmd_echo(void) {
    const char *arg = nextarg();

    redraw_prompt = false;

    if (!arg || strcmp(arg, "on") == 0)
        echo = true;

    else if (arg && strcmp(arg, "off") == 0)
        echo = false;
    
    else
        ERR("Usage: echo <on|off>");
}

/*
 * Teensy reset code from https://www.pjrc.com/teensy/jump_to_bootloader.html
 */
void cmd_reset(void) {
//    volatile static int barrier;
//
//    #ifndef TEENSY
//    Serial.println("Reset not supported");
//    return;
//    #endif
//
//    Serial.println("Resetting");
//    delay(100);
//    barrier++;
//
//    cli();
//    UDCON = 1;
//    USBCON = (1<<FRZCLK);
//    UCSR1B = 0;
//    delay(100);
//    EIMSK = 0; PCICR = 0; SPCR = 0; ACSR = 0; EECR = 0; ADCSRA = 0;
//    TIMSK0 = 0; TIMSK1 = 0; TIMSK3 = 0; TIMSK4 = 0; UCSR1B = 0; TWCR = 0;
//    DDRB = 0; DDRC = 0; DDRD = 0; DDRE = 0; DDRF = 0; TWCR = 0;
//    PORTB = 0; PORTC = 0; PORTD = 0; PORTE = 0; PORTF = 0;
//    asm volatile("jmp 0x7E00");
//
//    ERR("Did the reset fail?");
}

void cmd_compat(void) {
    Serial.println();
    Serial.println("Switching to old, compatible protocol");
    compat = true;
    echo = true;
}

struct {
    const char *name;
    command_t f;
} commands[NUM_COMMAND_SLOTS] = {
    { "!",          cmd_noop,       },
    { "@",          cmd_noop,       },
    { "ping",       cmd_ping,       },
    { "uptime",     cmd_ping,       },
    { "compat",     cmd_compat,     },
    { "echo",       cmd_echo,       },
    { "reset",      cmd_reset,      },
    { NULL,         NULL            },
};

void add_command(const char *name, command_t f) {
    int i;

    for (i = 0; i < NUM_COMMAND_SLOTS - 1; i++) {
        if (commands[i].name)
            continue;

        commands[i].name = name;
        commands[i].f = f;

        commands[i+1].name = NULL;
        commands[i+1].f = NULL;
        break;
    }
}

void dispatch(char *line) {
    char *cmd;
    int i;

    cmd = strtok(line, DELIM);
    if (!cmd)
        return;

    if (strcmp(cmd, "help") == 0) {
        for (i = 0; commands[i].name; i++) {
            Serial.println(format("    %s", commands[i].name));
        }
        Serial.println();
        return;
    }

    for (i = 0; commands[i].name; i++) {
        if (strcmp(cmd, commands[i].name) == 0) {
            commands[i].f();
            return;
        }
    }

    ERR("Unknown command");
}

void do_command(const char *line) {
    dispatch(getline_fixed(line));
}

void compat_loop() {
    char c;

    while (1) {
        heartbeat();

        if (! Serial.available())
            continue;

        c = Serial.read();

        switch (c) {
            case 'f': do_command("forward"); break;
            case 'b': do_command("back");    break; 
            case 'l': do_command("left");    break; 
            case 'r': do_command("right");   break; 
            case 'i': do_command("blink");   break; 

            case '!':
                compat = false;
                Serial.print('\r');
                return;

            case '#':
                cmd_reset();
                return;
        }
    }
}

void library_loop() {
    char *line;

    if (compat) {
        compat_loop();
        return;
    }

    if (echo) {
        line = getline();
        Serial.println();
    }
    else {
        line = getline_noecho();
    }

    dispatch(line);
}


