/* See LICENSE file for copyright and license details. */
/* Default settings; can be overriden by command line. */

static int instant = 0;                     /* -n  option; if 1, select single entry automatically */
static int topbar = 0;                      /* -b  option; if 0, dmenu appears at bottom     */
static const unsigned int alpha = 0xe9;     /* Amount of opacity. 0xff is opaque             */

static unsigned int barpadh	= 750;
static unsigned int barpadv	= 10;
static unsigned int barheight	= 2;
static unsigned int barborder	= 0;

/* -fn option overrides fonts[0]; default X11 font or font set */
static const char *fonts[] = {
	"iosevka:size=12"
};
static const char *prompt      = NULL;      /* -p  option; prompt to the left of input field */
static const char *colors[SchemeLast][2] = {
	            /*     fg         bg       */
	  [SchemeNorm] = { "#bbbbbb", "#2B303B" },
	   [SchemeSel] = { "#eeeeee", "#005577" },
	   [SchemeHp]  = { "#bbbbbb", "#333333" },
	   [SchemeOut] = { "#000000", "#00ffff" },
	[SchemeBorder] = { "#005577", "#005577" },
	[SchemePrompt] = { "#444444", "#222222" },
};

static const unsigned int alphas[SchemeLast][2] = {
	[SchemeNorm] = { OPAQUE, alpha },
	[SchemeSel] = { OPAQUE, alpha },
	[SchemeHp] = { OPAQUE, alpha },
	[SchemeOut] = { OPAQUE, alpha },
	[SchemeBorder] = { OPAQUE, OPAQUE },
	[SchemePrompt] = { OPAQUE, alpha },
};
/* -l option; if nonzero, dmenu uses vertical list with given number of lines */
static unsigned int lines      = 0;

/*
 * Characters not considered part of a word while deleting words
 * for example: " /?\"&[]"
 */
static const char worddelimiters[] = " ";
