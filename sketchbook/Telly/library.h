// vim:set sw=4 ts=4 ai et:

// Consumers of the library must implement heartbeat()
void heartbeat();

#define NUM_COMMAND_SLOTS   100
typedef void (*command_t)(void);
void add_command(const char *name, command_t f);
void do_command(const char *line);

#define DELIM   " \r\n\t"
#define nextarg() strtok(NULL, DELIM)
extern bool compat, echo, redraw_prompt;
int nextarg_int(int min, int max, int default_val, int on_error);

char *format(const char *fmt, ... );
void ERR(const char *fmt, ... );
void OK(const char *fmt, ... );
void OK(void);
void verbose(const char *fmt, ... );

void compat_loop();
void library_loop();
