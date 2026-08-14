/* See LICENSE file for copyright and license details. */
#include <X11/XF86keysym.h>

//#define ACTUALFULLSCREEN /* Uncomment if the actualfullscreen patch is added */
//#define AWESOMEBAR       /* Uncommnet if the awesomebar patch is used */

#define IMAGES_FOLDER "/home/vulto/images"

/* appearance */
static const unsigned int borderpx  = 0;        /* border pixel of windows */
static const int startwithgaps[]    = { 1 };	/* 1 means gaps are used by default, this can be customized for each tag */
static const unsigned int gappx[]   = { 10 };   /* default gap between windows in pixels, this can be customized for each tag */
static const unsigned int snap      = 20;       /* snap pixel */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const int user_bh            = 30;        /* 0 means that dwm will calculate bar height, >= 1 means dwm will user_bh as bar height */
static const double activeopacity   = 1.0f;     /* Window opacity when it's focused (0 <= opacity <= 1) */
static const double inactiveopacity = 0.675f;   /* Window opacity when it's inactive (0 <= opacity <= 1) */
static       Bool bUseOpacity       = False;     /* Starts with opacity on any unfocused windows */
static const int vertpad            = 10;       /* vertical padding of bar */
static const int sidepad            = 750;      /* horizontal padding of bar */
static const char *fonts[]          = { "iosevka:size=12" };
static const char dmenufont[]       = "iosevka:size=12";
static unsigned int baralpha        = 0xe9;
static unsigned int borderalpha     = OPAQUE;
static const char col_gray1[]       = "#999999";
static const char col_gray2[]       = "#282828";
static const char col_gray3[]       = "#0000aa";
static const char col_gray4[]       = "#2B303B";
static const char col_cyan[]        = "#005577";
/* PaperColor light */
static const char col_paper_fg[]    = "#444444";
static const char col_paper_bg[]    = "#eeeeee";
static const char col_paper_brd[]   = "#d0d0d0";
static const char col_paper_acc[]   = "#005f87";
static const char *colorsdark[][4]      = {
	/*               fg         bg         border     float */
	[SchemeNorm] = { col_gray1, col_gray4, col_gray2, col_gray2 },
	[SchemeSel]  = { col_gray1, col_gray4, col_gray2, col_cyan },
};
static const char *colorslight[][4]     = {
	/*               fg           bg           border        float */
	[SchemeNorm] = { col_paper_fg, col_paper_bg, col_paper_brd, col_paper_brd },
	[SchemeSel]  = { col_paper_fg, col_paper_bg, col_paper_brd, col_paper_acc },
};

static const char *const autostart[] = {
	"dwmblocks", NULL,
	"xset", "r","rate","180","25", "m","0","0", "-dpms", "s", "off", NULL,
	"setxkbmap", "-option", "caps:swapescape", NULL,
	"xhidecursor", NULL,
	"mons", "-s", NULL,
	"wall", "--dry", NULL,
	"dbus-update-activation-environment", "--systemd", "DISPLAY", "XAUTHORITY", NULL,
	"picom", "--backend", "xrender", "-fc", NULL,
	NULL /* terminate */
};

/* tagging */
static const char *tags[] = { "⚬", "⚬", "⚬", "⚬"};
static const char *alttags[] = { "⚫", "⚫", "⚫", "⚫"};

/* appicons */
/* NOTE: set to 0 to set to default (whitespace) */
static char outer_separator_beg      = '[';
static char outer_separator_end      = ']';
static char inner_separator          = ' ';
static unsigned truncate_icons_after = 9; /* will default to 1, that is the min */
static char truncate_symbol[]         = "...";

static const Rule rules[] = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING) = title
	 */
	/* class    instance    title       tags mask    isfloating  ispermanent  isfakefullscreen  monitor  appicon */
	{ "Brave",  NULL,       NULL,       0,           0,          0,           1,                -1,      NULL },
	{ "st",     NULL,       NULL,       0,		 1,          0,           0,                -1,      NULL },
	{ "imv",    NULL,       NULL,       0,		 1,          0,           0,                -1,      NULL },
	{ "mpv",    NULL,       NULL,       0,		 1,          0,           0,                -1,      NULL },
	{ "iv",     NULL,       NULL,       0,		 1,          0,           0,                -1,      NULL },
};

/* layout(s) */
static const float mfact     = 0.50; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 60;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ ">M>",     centeredfloatingmaster },
	{ " ",      centeredmaster },           /* first entry is default */
	{ " ",      NULL },                     /* no layout function means floating behavior */
	{ " ",       monocle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

#define STATUSBAR "dwmblocks"

/* commands */
static char dmenumon[2] = "0"; /* component of dmenu{dark,light}, manipulated in spawndmenu() */
static const char *dmenudark[]  = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray4, "-nf", col_gray1, "-sb", col_cyan, "-sf", col_gray1, NULL };
static const char *dmenulight[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_paper_bg, "-nf", col_paper_fg, "-sb", col_paper_acc, "-sf", col_paper_bg, NULL };
static const char *termcmd[]  = { "st", NULL };

static const Key keys[] = {
	/* modifier         key		  	      function        argument */
	{ MODKEY,           XK_Return,		      spawn,	      SHCMD("st") },
	{ MODKEY,           XK_a,		      toggleopacity,  {0} },
	{ MODKEY,	    XK_b,		      togglebar,      {0} },
	{ MODKEY,	    XK_b,	              spawn,	      SHCMD("pgrep -x dwmblocks >/dev/null && pkill -x dwmblocks || dwmblocks &") },
	{ MODKEY,           XK_c, 		      killclient,     {0} },
	{ MODKEY,	    XK_d,		      spawndmenu,     {0} },
	{ MODKEY,           XK_t,                     togglecolormode,{0} },
	{ MODKEY,	    XK_f,                     togglefullscr,  {0} },
	{ MODKEY|ShiftMask, XK_f,                     fullscreen,     {0} },
	{ MODKEY,	    XK_h,		      setmfact,	      {.f = -0.05} },
	{ MODKEY,	    XK_j,		      focusstack,     {.i = +1 } },
	{ MODKEY,           XK_k,		      focusstack,     {.i = -1 } },
	{ MODKEY,	    XK_l,		      setmfact,	      {.f = +0.05} },
	{ MODKEY,	    XK_m,		      spawn,          SHCMD("st -e wiremix") },
	{ MODKEY,	    XK_n,		      spawn,          SHCMD("wall") },
	{ MODKEY,           XK_u,                     setlayout,      {.v = &layouts[0]} },
	{ MODKEY,           XK_o,                     setlayout,      {.v = &layouts[3]} },
	{ MODKEY,	    XK_equal,		      incnmaster,     {.i = +1 } },
	{ MODKEY,	    XK_minus,		      incnmaster,     {.i = -1 } },
	{ MODKEY,	    XK_z,		      zoom,	      {0} },
	{ MODKEY,	    XK_Tab,		      view,	      {0} },
	{ MODKEY,	    XK_space,		      togglefloating, {0} },
	{ MODKEY,	    XK_0,		      view,	      {.ui = ~0 } },
	{ MODKEY|ShiftMask, XK_0,		      tag,	      {.ui = ~0 } },
	{ MODKEY,	    XK_comma,		      focusmon,       {.i = -1 } },
	{ MODKEY,	    XK_period,		      focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask, XK_comma,		      tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask, XK_period,		      tagmon,         {.i = +1 } },
	{ MODKEY|ShiftMask, XK_Tab,                   view,           {0} },
	{ MODKEY,           XK_Tab,                   previewallwin,  {0} },
	{ MODKEY,	    XK_minus,                 setgaps,        {.i = GAP_RESET } },
	{ MODKEY,           XK_equal,                 setgaps,        {.i = +1 } },
	{ MODKEY|ShiftMask, XK_equal,                 setgaps,        {.i = GAP_TOGGLE} },
	{ 0,		    XK_Print,		      spawn,          SHCMD("scrot ~/images/screenshots/%a-%e-%b_%H-%M-%S.png && nsxiv -t ~/images/screenshots/") },
	{ MODKEY,	    XK_Print,		      spawn,          SHCMD("scrot -s ~/images/screenshots/%a-%e-%b_%H-%M-%S.png && nsxiv -t  ~/images/screenshots/") },
	{ MODKEY,	    XK_End,		      spawn,          SHCMD("shutdown") },
	{ MODKEY,	    XK_Home,		      spawn,          SHCMD("slock") },
	{ 0,		    XF86XK_AudioLowerVolume,  spawn,          SHCMD("pactl set-sink-volume @DEFAULT_SINK@ -1%") },
	{ 0,		    XF86XK_AudioMute,         spawn,          SHCMD("pactl set-sink-mute @DEFAULT_SINK@ toggle") },
	{ 0,		    XF86XK_AudioRaiseVolume,  spawn,          SHCMD("pactl set-sink-volume @DEFAULT_SINK@ +1%") },
	{ 0,		    XF86XK_AudioPrev,	      spawn,          SHCMD("playerctl previous" ) },
	{ 0,		    XF86XK_AudioPlay,         spawn,          SHCMD("playerctl play-pause") },
	{ 0,		    XF86XK_AudioNext,         spawn,          SHCMD("playerctl next") },
	{ 0,		    XF86XK_WLAN,              spawn,          SHCMD("rfkill unblock wifi") },
	{ 0,		    XF86XK_MonBrightnessDown, spawn,          SHCMD("light -U 5") },
	{ 0,		    XF86XK_MonBrightnessUp,   spawn,          SHCMD("light -A 5") },
	TAGKEYS(	    XK_1,                      0)
	TAGKEYS(	    XK_2,                      1)
	TAGKEYS(	    XK_3,                      2)
	TAGKEYS(	    XK_4,                      3)
	TAGKEYS(	    XK_5,                      4)
	TAGKEYS(	    XK_6,                      5)
	TAGKEYS(	    XK_7,                      6)
	TAGKEYS(	    XK_8,                      7)
	TAGKEYS(	    XK_9,                      8)
	{ MODKEY|ShiftMask, XK_q,                     quit,		 {0} },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button1,        sigstatusbar,   {.i = 1} },
	{ ClkStatusText,        0,              Button2,        sigstatusbar,   {.i = 2} },
	{ ClkStatusText,        0,              Button3,        sigstatusbar,   {.i = 3} },
	{ ClkStatusText,        0,              Button4,        sigstatusbar,   {.i = 4} },
	{ ClkStatusText,        0,              Button5,        sigstatusbar,   {.i = 5} },
	{ ClkStatusText,        0,              6,              sigstatusbar,   {.i = 6} },
	{ ClkStatusText,        0,              7,              sigstatusbar,   {.i = 7} },
	{ ClkStatusText,        0,              8,              sigstatusbar,   {.i = 8} },
	{ ClkStatusText,        0,              9,              sigstatusbar,   {.i = 9} },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
