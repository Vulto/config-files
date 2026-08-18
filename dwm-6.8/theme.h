/* Shared desktop themes. Edit color values in themes/; rebuild the tree.
 * Super+t cycles ~/.config/dwm/current. Extra files in
 * ~/.config/dwm/themes/ add a theme without a rebuild. Format: key=value. */

#ifndef DWM_THEME_H
#define DWM_THEME_H

#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include "theme_builtins.h"

#define THEME_MAX 32
#define THEME_KEY 32
#define THEME_VAL 64
#define THEME_NBUILTINS (sizeof theme_builtins / sizeof theme_builtins[0])
#if defined(__GNUC__)
#define THEME_UNUSED __attribute__((unused))
#else
#define THEME_UNUSED
#endif

typedef struct {
	char name[THEME_VAL];
	char nvim[THEME_VAL];
	char mode[THEME_VAL]; /* "dark" or "light" — system appearance */
	char fg[THEME_VAL];
	char bg[THEME_VAL];
	char border[THEME_VAL];
	char sel_fg[THEME_VAL];
	char sel_bg[THEME_VAL];
	char flt[THEME_VAL];
	char term[16][THEME_VAL];
	char termfg[THEME_VAL];
	char termbg[THEME_VAL];
} Theme;

static int
theme_homepath(char *buf, size_t n, const char *rel)
{
	const char *home = getenv("HOME");

	if (!home)
		return -1;
	snprintf(buf, n, "%s/.config/dwm/%s", home, rel);
	return 0;
}

static void
theme_writefile(const char *path, const char *body)
{
	FILE *fp = fopen(path, "w");

	if (!fp)
		return;
	fputs(body, fp);
	fclose(fp);
}

static void
theme_ensure(void)
{
	char dir[256], path[256];
	const char *home = getenv("HOME");

	if (!home)
		return;
	snprintf(dir, sizeof dir, "%s/.config/dwm", home);
	mkdir(dir, 0755);
	if (theme_homepath(dir, sizeof dir, "themes") == 0)
		mkdir(dir, 0755);
	if (theme_homepath(path, sizeof path, "current") == 0 && access(path, F_OK) != 0) {
		char seed[THEME_VAL + 2];

		snprintf(seed, sizeof seed, "%s\n",
			THEME_NBUILTINS ? theme_builtins[0].name : "nord");
		theme_writefile(path, seed);
	}
}

static void
theme_set(const char *key, const char *val, Theme *t)
{
	if (!strcmp(key, "name"))
		snprintf(t->name, sizeof t->name, "%s", val);
	else if (!strcmp(key, "nvim"))
		snprintf(t->nvim, sizeof t->nvim, "%s", val);
	else if (!strcmp(key, "mode"))
		snprintf(t->mode, sizeof t->mode, "%s", val);
	else if (!strcmp(key, "fg"))
		snprintf(t->fg, sizeof t->fg, "%s", val);
	else if (!strcmp(key, "bg"))
		snprintf(t->bg, sizeof t->bg, "%s", val);
	else if (!strcmp(key, "border"))
		snprintf(t->border, sizeof t->border, "%s", val);
	else if (!strcmp(key, "sel_fg"))
		snprintf(t->sel_fg, sizeof t->sel_fg, "%s", val);
	else if (!strcmp(key, "sel_bg"))
		snprintf(t->sel_bg, sizeof t->sel_bg, "%s", val);
	else if (!strcmp(key, "float"))
		snprintf(t->flt, sizeof t->flt, "%s", val);
	else if (!strcmp(key, "termfg"))
		snprintf(t->termfg, sizeof t->termfg, "%s", val);
	else if (!strcmp(key, "termbg"))
		snprintf(t->termbg, sizeof t->termbg, "%s", val);
	else if (!strncmp(key, "term", 4) && key[4] >= '0' && key[4] <= '9') {
		int i = atoi(key + 4);

		if (i >= 0 && i < 16)
			snprintf(t->term[i], sizeof t->term[i], "%s", val);
	}
}

static int
theme_parse_buf(Theme *t, const char *buf)
{
	char line[256], key[THEME_KEY], val[THEME_VAL];
	const char *p, *nl;
	int len;

	memset(t, 0, sizeof *t);
	for (p = buf; *p; p = nl ? nl + 1 : p + strlen(p)) {
		nl = strchr(p, '\n');
		len = nl ? (int)(nl - p) : (int)strlen(p);
		snprintf(line, sizeof line, "%.*s", len, p);
		if (!line[0] || line[0] == '#')
			continue;
		if (sscanf(line, " %31[^=]=%63s", key, val) == 2)
			theme_set(key, val, t);
	}
	if (!t->sel_fg[0])
		snprintf(t->sel_fg, sizeof t->sel_fg, "%s", t->fg);
	if (!t->sel_bg[0])
		snprintf(t->sel_bg, sizeof t->sel_bg, "%s", t->bg);
	if (!t->flt[0])
		snprintf(t->flt, sizeof t->flt, "%s", t->border);
	if (!t->termfg[0])
		snprintf(t->termfg, sizeof t->termfg, "%s", t->fg);
	if (!t->termbg[0])
		snprintf(t->termbg, sizeof t->termbg, "%s", t->bg);
	if (strcmp(t->mode, "dark") && strcmp(t->mode, "light")) {
		unsigned r = 0, g = 0, b = 0;
		const char *p = t->bg[0] == '#' ? t->bg + 1 : t->bg;

		if (sscanf(p, "%2x%2x%2x", &r, &g, &b) == 3
		 && 0.299 * r + 0.587 * g + 0.114 * b >= 128)
			snprintf(t->mode, sizeof t->mode, "light");
		else
			snprintf(t->mode, sizeof t->mode, "dark");
	}
	return t->fg[0] && t->bg[0] ? 0 : -1;
}

static int
theme_is_builtin(const char *name)
{
	size_t i;

	for (i = 0; i < THEME_NBUILTINS; i++)
		if (!strcmp(theme_builtins[i].name, name))
			return 1;
	return 0;
}

static int
theme_load(Theme *t, const char *name)
{
	char path[256], body[4096];
	FILE *fp;
	size_t i, n;

	theme_ensure();
	if (!name || !name[0])
		name = "nord";
	for (i = 0; i < THEME_NBUILTINS; i++) {
		if (strcmp(theme_builtins[i].name, name))
			continue;
		if (theme_parse_buf(t, theme_builtins[i].body) < 0)
			return -1;
		if (!t->name[0])
			snprintf(t->name, sizeof t->name, "%s", name);
		return 0;
	}
	if (theme_homepath(path, sizeof path, "themes/") < 0)
		return -1;
	snprintf(path + strlen(path), sizeof path - strlen(path), "%s", name);
	if (!(fp = fopen(path, "r")))
		return -1;
	n = fread(body, 1, sizeof body - 1, fp);
	fclose(fp);
	body[n] = '\0';
	if (theme_parse_buf(t, body) < 0)
		return -1;
	if (!t->name[0])
		snprintf(t->name, sizeof t->name, "%s", name);
	return 0;
}

static void THEME_UNUSED
theme_write_active(const Theme *t)
{
	char path[256], body[256];

	if (!t || theme_homepath(path, sizeof path, "active") < 0)
		return;
	snprintf(body, sizeof body, "name=%s\nnvim=%s\nmode=%s\n",
		t->name[0] ? t->name : "nord",
		t->nvim[0] ? t->nvim : (t->name[0] ? t->name : "nord"),
		t->mode[0] ? t->mode : "dark");
	theme_writefile(path, body);
}

static int
theme_current_name(char *name, size_t n)
{
	char path[256];
	FILE *fp;

	theme_ensure();
	snprintf(name, n, "nord");
	if (theme_homepath(path, sizeof path, "current") < 0)
		return -1;
	if (!(fp = fopen(path, "r")))
		return 0;
	if (!fgets(name, n, fp))
		snprintf(name, n, "nord");
	fclose(fp);
	name[strcspn(name, "\r\n")] = '\0';
	if (!name[0])
		snprintf(name, n, "nord");
	return 0;
}

static int
theme_set_current(const char *name)
{
	char path[256], body[THEME_VAL + 2];

	if (theme_homepath(path, sizeof path, "current") < 0)
		return -1;
	snprintf(body, sizeof body, "%s\n", name);
	theme_writefile(path, body);
	return 0;
}

static int
theme_namecmp(const void *a, const void *b)
{
	return strcmp(*(char *const *)a, *(char *const *)b);
}

static int THEME_UNUSED
theme_cycle(char *name, size_t n)
{
	char dir[256], cur[THEME_VAL], *list[THEME_MAX];
	int i, count = 0, next = 0;
	size_t b;
	DIR *dp;
	struct dirent *de;

	theme_ensure();
	theme_current_name(cur, sizeof cur);
	for (b = 0; b < THEME_NBUILTINS && count < THEME_MAX; b++) {
		list[count] = strdup(theme_builtins[b].name);
		if (list[count])
			count++;
	}
	if (theme_homepath(dir, sizeof dir, "themes") == 0 && (dp = opendir(dir))) {
		while ((de = readdir(dp)) && count < THEME_MAX) {
			if (de->d_name[0] == '.')
				continue;
			if (theme_is_builtin(de->d_name))
				continue;
			list[count] = strdup(de->d_name);
			if (list[count])
				count++;
		}
		closedir(dp);
	}
	if (!count) {
		snprintf(name, n, "nord");
		return theme_set_current(name);
	}
	qsort(list, count, sizeof list[0], theme_namecmp);
	for (i = 0; i < count; i++)
		if (!strcmp(list[i], cur)) {
			next = (i + 1) % count;
			break;
		}
	snprintf(name, n, "%s", list[next]);
	theme_set_current(name);
	for (i = 0; i < count; i++)
		free(list[i]);
	return 0;
}

#endif
