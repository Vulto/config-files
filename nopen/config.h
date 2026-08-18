#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <magic.h>
#include <unistd.h>
#include <getopt.h>
#include <stdbool.h>
#include <ctype.h>

#define editor  "nvim"
#define imgv  "nsxiv"
#define player  "mpv"
#define pdf  "zathura"
#define browser  "surf"
#define filemgr  "cfl"
#define audio  "mpg123"
#define wine  "wine"
#define shell  "bash"
#define emu	"mednafen"

	typedef enum {
		TUI,
		GUI,
	} type;

    const struct {
        const char *file_type;
        const char *application;
        type interface;
    } fileTypes[] = {
        {"text/plain",                      editor, TUI},
        {"text/x-c",                        editor, TUI},
        {"text/csv",                        editor, TUI},
        {"text/x-makefile",                 editor, TUI},
        {"text/x-shellscript",              editor, TUI},
        {"text/x-tex",                      editor, TUI},
        {"text/html",                       browser, GUI},
        {"x-www-browser",                   browser, GUI},
        {"application/json",                editor, TUI},
        {"inode/x-empty",                   editor, TUI},
        {"application/x-nes-rom",			emu, GUI},
        {"application/octet-stream",        emu, GUI},
        {"application/javascript",          editor, TUI},
        {"inode/directory",                 filemgr, TUI},
        {"video/mp4",                       player, GUI},
        {"video/webm",                      player, GUI},
        {"video/quicktime",                 player, GUI},
        {"video/x-matroska",                player, GUI},
        {"image/png",                       imgv, GUI},
        {"image/jpg",                       imgv, GUI},
        {"image/jpeg",                      imgv, GUI},
        {"image/gif",                       imgv, GUI},
        {"image/webpg",                     imgv, GUI},
        {"audio/x-wav",                     audio, TUI},
        {"audio/mpeg",                      audio, TUI},
        {"application/pdf",									pdf, GUI},
        {"application/vnd.microsoft.portable-executable",	wine, GUI},
    };
const char *findProg(const char *mime_type) {
    char mime_lower[256];
    strncpy( mime_lower, mime_type, sizeof( mime_lower ) - 1);
    mime_lower[sizeof( mime_lower ) - 1] = '\0';
    for ( char *p = mime_lower; *p; p++ )
        *p = ( char )tolower(( unsigned char ) *p );

   

    for (size_t i = 0; i < sizeof(fileTypes) / sizeof(fileTypes[0]); ++i) {
        if (strcmp(fileTypes[i].file_type, mime_lower) == 0) {
            return fileTypes[i].application;
        }
    }

    return editor;  // fallback
}
