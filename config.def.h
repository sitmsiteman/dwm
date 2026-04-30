/* See LICENSE file for copyright and license details. */

#include <X11/XF86keysym.h>

/* appearance */
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 32;       /* snap pixel */
static const unsigned int systraypinning = 0;   /* 0: sloppy systray follows selected monitor, >0: pin systray to monitor X */
static const unsigned int systrayonleft = 0;    /* 0: systray in the right corner, >0: systray on left of status text */
static const unsigned int systrayspacing = 2;   /* systray spacing */
static const int systraypinningfailfirst = 1;   /* 1: if pinning fails, display systray on the first monitor, False: display systray on the last monitor*/
static const int showsystray        = 1;        /* 0 means no systray */
static const int showbar            = 1;        /* 0 means no bar */
static const int topbar             = 1;        /* 0 means bottom bar */
static const char *fonts[]          = { "Noto Sans CJK KR:size=10" };
static const char dmenufont[]       = "Noto Sans CJK KR:size=10";
static const char col_gray1[]       = "#222222";
static const char col_gray2[]       = "#444444";
static const char col_gray3[]       = "#bbbbbb";
static const char col_gray4[]       = "#eeeeee";
static const char col_cyan[]        = "#005577";
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray2 },
	[SchemeSel]  = { col_gray4, col_cyan,  col_cyan  },
};

static const char *const autostart[] = {
	"st", "-c", "st-main", "-e", "tmux", "new-session", "-A", "-s", "main", NULL,
	"st", "-c", "st-remote", "-e", "tmux", "new-session", "-A", "-s", "remote", NULL,
	"st", "-c", "st-mail", "-e", "aerc", NULL,
	"slstatus", NULL,
        NULL /* terminate */
};

/* tagging */

/*
  1: Terminal                   6: Mail, messengers ...
  2: SSH/Drawterm               7: Multimedia/Gaming
  3: Output Qemu/Monitoring     8: Editors like acme, emacs, gimp ..
  4: File Managers              9: Web
  5: Documents                  -: Temp
*/

static const char *tags[] = { "Local", "Remote", "Out", "Files",
			      "Docs", "Mail", "Media", "Ed", "Web", "-" };

static const Rule rules[]	 = {
	/* xprop(1):
	 *	WM_CLASS(STRING) = instance, class
	 *	WM_NAME(STRING)	 = title
	 */
	/* class      instance    title       tags mask     isfloating   monitor */
	{"Gimp"				,NULL			,NULL	,1 << 7	,1	,-1},
	{"firefox"			,NULL			,NULL	,1 << 8 ,0	,-1},
	{"Emacs"			,NULL			,NULL	,1 << 7	,0	,-1},
	{"acme"				,NULL			,NULL	,1 << 7	,0	,-1},
	{"mpv"				,NULL			,NULL	,1 << 6	,0	,-1},
	{"Zathura"			,NULL			,NULL	,1 << 4	,0	,-1},
	{"steam"			,"steamwebhelper"	,NULL	,1 << 6	,0	,-1},
	{"steam_app"			,NULL			,NULL	,1 << 6	,0	,-1},
	{"gamescope"			,NULL			,NULL	,1 << 6	,0	,-1},
	{"st-256color"			,NULL			,NULL	,0	,0	,-1},
	{"st-remote"			,NULL			,NULL	,1 << 1	,0	,-1},
	{"st-main"			,NULL			,NULL	,1	,0	,-1},
	{"st-mail"                      ,NULL                   ,NULL   ,1 << 5 ,0      ,-1},
	{"Drawterm"			,NULL			,NULL	,1 << 1	,0	,-1},
	{"qemu-system"			,NULL			,NULL	,1 << 2 ,1	,-1},
	{"TelegramDesktop"		,NULL			,NULL	,1 << 5	,1	,-1},
	{"xdg-desktop-portal-lxqt"	,NULL			,NULL	,0	,1	,-1},
	{"Pcmanfm"			,NULL			,NULL	,1 << 3	,0	,-1},
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 1;    /* 1 means respect size hints in tiled resizals */
static const int lockfullscreen = 1; /* 1 will force focus on the fullscreen window */
static const int refreshrate = 120;  /* refresh rate (per second) for client move/resize */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[]=",      tile },    /* first entry is default */
	{ "><>",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
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

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { "dmenu_run", "-m", dmenumon, "-fn", dmenufont, "-nb", col_gray1, "-nf", col_gray3, "-sb", col_cyan, "-sf", col_gray4, NULL };
static const char *termcmd[]  = { "st", "-e", "tmux", "new-session", "-A", "-s", "sub", NULL };
static const char *screenshotcmd[]  = { "xshot.sh", NULL };
static const char *toggledpmscmd[]  = { "toggle-dpms", NULL };
static const char *dpmsoffcmd[] = { "xset", "dpms", "force", "off", NULL };
static const char *upbrightness[]   = { "brightnessctl", "set", "+5%", NULL };
static const char *downbrightness[] = { "brightnessctl", "set", "5%-", NULL };
static const char *mutecmd[]   = { "pactl", "set-sink-mute", "@DEFAULT_SINK@", "toggle", NULL };
static const char *volupcmd[]   = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "+5%", NULL };
static const char *voldowncmd[] = { "pactl", "set-sink-volume", "@DEFAULT_SINK@", "-5%", NULL };
static const char scratchpadname[] = "scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "120x34", NULL };

static const Key keys[] = {
	/* modifier                     key        function        argument */
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY|ShiftMask,             XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_grave,  togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY,                       XK_j,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = -1 } },
	{ MODKEY,                       XK_i,      incnmaster,     {.i = +1 } },
	{ MODKEY,                       XK_d,      incnmaster,     {.i = -1 } },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY,                       XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY|ShiftMask,             XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY,              XK_v,  setlayout,      {0} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_0,      view,           {.ui = ~0 } },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
	TAGKEYS(                        XK_minus,                  9)
	{ MODKEY|ShiftMask,             XK_q,      quit,           {0} },
	{ 0,                     XF86XK_AudioMute,        spawn,          {.v = mutecmd } },
	{ 0,                     XF86XK_AudioRaiseVolume, spawn,          {.v = volupcmd } },
	{ 0,                     XF86XK_AudioLowerVolume, spawn,          {.v = voldowncmd } },
	{ 0,            XF86XK_MonBrightnessUp,    spawn,          {.v = upbrightness } },
	{ 0,            XF86XK_MonBrightnessDown,  spawn,          {.v = downbrightness } },
	{ MODKEY|Mod1Mask,            XK_l,  spawn,          {.v = dpmsoffcmd } },
	{ MODKEY|ShiftMask,             XK_h,      setcfact,       {.f = +0.25} },
	{ MODKEY|ShiftMask,             XK_l,      setcfact,       {.f = -0.25} },
	{ MODKEY|ShiftMask,             XK_o,      setcfact,       {.f =  0.00} },
	{ 0,            	XK_Print, 		spawn,          {.v = screenshotcmd } },
	{ MODKEY,            	XK_q, 		spawn,          {.v = toggledpmscmd } },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static const Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
	{ ClkTagBar,            0,              Button1,        view,           {0} },
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};

