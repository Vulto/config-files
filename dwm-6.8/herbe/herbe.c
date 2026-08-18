#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xft/Xft.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>
#include <semaphore.h>

#include "config.h"
#include "../theme.h"

#define EXIT_ACTION 0
#define EXIT_FAIL 1
#define EXIT_DISMISS 2

Display *display;
Window window;
int exit_code = EXIT_DISMISS;
static int screen;
static Visual *visual;
static Colormap colormap;
static const char *background_color;
static const char *border_color;
static const char *font_color;
static XftColor xft_bg, xft_border, xft_font;
static int colors_loaded;
static Atom dwmcolormodeatom;
static XftDraw *draw;
static XftFont *font;
static char **lines;
static int num_of_lines;
static unsigned int text_height;

static Theme herbetheme;

static void
setcolormode(void)
{
	char name[THEME_VAL];

	theme_current_name(name, sizeof name);
	if (theme_load(&herbetheme, name) < 0)
		theme_load(&herbetheme, "nord");
	background_color = herbetheme.bg;
	border_color = herbetheme.border;
	font_color = herbetheme.fg;

	if (!display)
		return;
	if (colors_loaded) {
		XftColorFree(display, visual, colormap, &xft_bg);
		XftColorFree(display, visual, colormap, &xft_border);
		XftColorFree(display, visual, colormap, &xft_font);
	}
	XftColorAllocName(display, visual, colormap, background_color, &xft_bg);
	XftColorAllocName(display, visual, colormap, border_color, &xft_border);
	XftColorAllocName(display, visual, colormap, font_color, &xft_font);
	colors_loaded = 1;

	if (window) {
		int i;

		XSetWindowBackground(display, window, xft_bg.pixel);
		XSetWindowBorder(display, window, xft_border.pixel);
		XClearWindow(display, window);
		if (draw && font && lines) {
			for (i = 0; i < num_of_lines; i++)
				XftDrawStringUtf8(draw, &xft_font, font, padding,
					line_spacing * i + text_height * (i + 1) + padding,
					(FcChar8 *)lines[i], strlen(lines[i]));
		}
		XFlush(display);
	}
}

static void die(const char *format, ...)
{
	va_list ap;
	va_start(ap, format);
	vfprintf(stderr, format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
	exit(EXIT_FAIL);
}

int get_max_len(char *string, XftFont *font, int max_text_width)
{
	int eol = strlen(string);
	XGlyphInfo info;
	XftTextExtentsUtf8(display, font, (FcChar8 *)string, eol, &info);

	if (info.width > max_text_width)
	{
		eol = max_text_width / font->max_advance_width;
		info.width = 0;

		while (info.width < max_text_width)
		{
			eol++;
			XftTextExtentsUtf8(display, font, (FcChar8 *)string, eol, &info);
		}

		eol--;
	}

	for (int i = 0; i < eol; i++)
		if (string[i] == '\n')
		{
			string[i] = ' ';
			return ++i;
		}

	if (info.width <= max_text_width)
		return eol;

	int temp = eol;

	while (string[eol] != ' ' && eol)
		--eol;

	if (eol == 0)
		return temp;
	else
		return ++eol;
}

void expire(int sig)
{
	XEvent event;
	event.type = ButtonPress;
	event.xbutton.button = (sig == SIGUSR2) ? (ACTION_BUTTON) : (DISMISS_BUTTON);
	XSendEvent(display, window, 0, 0, &event);
	XFlush(display);
}

int main(int argc, char *argv[])
{
	if (argc == 1)
	{
		sem_unlink("/herbe");
		die("Usage: %s [-d duration] body...", argv[0]);
	}

	unsigned int timeout = duration;
	int argi = 1;

	for (; argi < argc; argi++)
	{
		if (!strcmp(argv[argi], "--"))
		{
			argi++;
			break;
		}
		if (!strcmp(argv[argi], "-d"))
		{
			char *end;

			if (++argi >= argc)
				die("Usage: %s [-d duration] body...", argv[0]);

			timeout = strtoul(argv[argi], &end, 10);
			if (*end != '\0' || end == argv[argi])
				die("Invalid duration: %s", argv[argi]);
			continue;
		}
		if (argv[argi][0] == '-')
			die("Unknown option: %s", argv[argi]);
		break;
	}

	if (argi >= argc)
		die("Usage: %s [-d duration] body...", argv[0]);

	struct sigaction act_expire, act_ignore;

	act_expire.sa_handler = expire;
	act_expire.sa_flags = SA_RESTART;
	sigemptyset(&act_expire.sa_mask);

	act_ignore.sa_handler = SIG_IGN;
	act_ignore.sa_flags = 0;
	sigemptyset(&act_ignore.sa_mask);

	sigaction(SIGALRM, &act_expire, 0);
	sigaction(SIGTERM, &act_expire, 0);
	sigaction(SIGINT, &act_expire, 0);

	sigaction(SIGUSR1, &act_ignore, 0);
	sigaction(SIGUSR2, &act_ignore, 0);

	if (!(display = XOpenDisplay(0)))
		die("Cannot open display");

	screen = DefaultScreen(display);
	visual = DefaultVisual(display, screen);
	colormap = DefaultColormap(display, screen);
	dwmcolormodeatom = XInternAtom(display, "_DWM_COLORMODE", False);
	setcolormode();

	int screen_width = DisplayWidth(display, screen);
	int screen_height = DisplayHeight(display, screen);

	XSetWindowAttributes attributes;
	attributes.override_redirect = True;
	attributes.background_pixel = xft_bg.pixel;
	attributes.border_pixel = xft_border.pixel;

	num_of_lines = 0;
	int max_text_width = width - 2 * padding;
	int lines_size = 5;
	lines = malloc(lines_size * sizeof(char *));
	if (!lines)
		die("malloc failed");

	font = XftFontOpenName(display, screen, font_pattern);

	for (int i = argi; i < argc; i++)
	{
		for (unsigned int eol = get_max_len(argv[i], font, max_text_width); eol; argv[i] += eol, num_of_lines++, eol = get_max_len(argv[i], font, max_text_width))
		{
			if (lines_size <= num_of_lines)
			{
				lines = realloc(lines, (lines_size += 5) * sizeof(char *));
				if (!lines)
					die("realloc failed");
			}

			lines[num_of_lines] = malloc((eol + 1) * sizeof(char));
			if (!lines[num_of_lines])
				die("malloc failed");

			strncpy(lines[num_of_lines], argv[i], eol);
			lines[num_of_lines][eol] = '\0';
		}
	}

	unsigned int x = pos_x;
	unsigned int y = pos_y;
	text_height = font->ascent - font->descent;
	unsigned int height = (num_of_lines - 1) * line_spacing + num_of_lines * text_height + 2 * padding;

	if (corner == TOP_RIGHT || corner == BOTTOM_RIGHT)
		x = screen_width - width - border_size * 2 - pos_x;

	if (corner == BOTTOM_LEFT || corner == BOTTOM_RIGHT)
		y = screen_height - height - border_size * 2 - pos_y;

	window = XCreateWindow(display, RootWindow(display, screen), x, y, width, height, border_size, DefaultDepth(display, screen),
						   CopyFromParent, visual, CWOverrideRedirect | CWBackPixel | CWBorderPixel, &attributes);

	{
		XClassHint ch = { "herbe", "herbe" };

		XSetClassHint(display, window, &ch);
	}

	draw = XftDrawCreate(display, window, visual, colormap);

	XSelectInput(display, window, ExposureMask | ButtonPress);
	XSelectInput(display, RootWindow(display, screen), PropertyChangeMask);
	XMapWindow(display, window);

	sem_t *mutex = sem_open("/herbe", O_CREAT, 0644, 1);
	sem_wait(mutex);

	sigaction(SIGUSR1, &act_expire, 0);
	sigaction(SIGUSR2, &act_expire, 0);

	if (timeout != 0)
		alarm(timeout);

	for (;;)
	{
		XEvent event;
		XNextEvent(display, &event);

		if (event.type == Expose)
		{
			XClearWindow(display, window);
			for (int i = 0; i < num_of_lines; i++)
				XftDrawStringUtf8(draw, &xft_font, font, padding, line_spacing * i + text_height * (i + 1) + padding,
								  (FcChar8 *)lines[i], strlen(lines[i]));
		}
		else if (event.type == PropertyNotify)
		{
			if (event.xproperty.window == RootWindow(display, screen)
			 && event.xproperty.atom == dwmcolormodeatom)
				setcolormode();
		}
		else if (event.type == ButtonPress)
		{
			if (event.xbutton.button == DISMISS_BUTTON)
				break;
			else if (event.xbutton.button == ACTION_BUTTON)
			{
				exit_code = EXIT_ACTION;
				break;
			}
		}
	}

	sem_post(mutex);
	sem_close(mutex);

	for (int i = 0; i < num_of_lines; i++)
		free(lines[i]);

	free(lines);
	XftDrawDestroy(draw);
	if (colors_loaded) {
		XftColorFree(display, visual, colormap, &xft_bg);
		XftColorFree(display, visual, colormap, &xft_border);
		XftColorFree(display, visual, colormap, &xft_font);
	}
	XftFontClose(display, font);
	XCloseDisplay(display);

	return exit_code;
}