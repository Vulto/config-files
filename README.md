# config-files

A daily-driver **X11 desktop** built around patched suckless software. Almost
every binary is compiled from this tree and installed to `/usr/local/bin`.
Shared live themes keep dwm, st, herbe, and Neovim in sync.

On Gentoo:

```sh
doas ./install-gentoo.sh --nvim
sx
```

## Layout

| Path | Role |
| --- | --- |
| `dwm-6.8/` | `dwm` + `config.h` + `theme.h` + `theme-apply` |
| `dwm-6.8/dmenu-5.4/` | Application launcher (linked into `dwm`) |
| `dwm-6.8/st-0.9.3/` | Terminal (linked into `dwm`) |
| `dwm-6.8/blocks/` | Bar helpers: `battery`, `net`, `spotify-notify` |
| `dwm-6.8/herbe/` | Notifications |
| `dwm-6.8/mons/` | xrandr helper (autostart `-s`) |
| `dwm-6.8/nopen/` | MIME opener (`libmagic`) |
| `dwm-6.8/nsxiv/` | Image viewer |
| `dwm-6.8/slock/` | Screen locker (setuid, Imlib2 blur) |
| `dwm-6.8/sent/` | Plaintext slide deck |
| `soap/` | Regex `xdg-open` replacement (opt-in, invasive) |
| `sx/` | `startx` replacement |
| `xhidecursor/` | Hide the cursor on keypress |
| `fontconfig/` | Reject Unifont Sample; weak Nerd Font fallback |
| `fonts/Terminus.zip` | Terminess Nerd Font (patched Terminus) |
| `init.vim` | Neovim config (follows the dwm theme) |
| `nvim/` | Extra Neovim files (`after/`, `lsp/`) |
| `install-gentoo.sh` | Emerge dependencies and install everything |

## Icons

Fonts and geometry for dwm, st, dmenu, herbe, and slock live in
`dwm-6.8/config.h` (`FONT`, `FONT_*`, `BARW`, `BAR_ALPHA`, `BORDERPX`).
dwm and dmenu also list Font Awesome as fallbacks so status icons render.
st takes a single `FONT_ST` string — do not put commas in it.

`fontconfig/10-nerd-symbols.conf` rejects **Unifont Sample** (it claims
the whole Private Use Area and draws empty boxes).

To pick an icon later:

1. Open https://www.w3schools.com/icons/icons_reference.asp and use the
   **Font Awesome 4** list. `&#xf025;` **is** the character (`` headphones).
2. If FA4 has no icon (Neovim), use https://www.nerdfonts.com/cheat-sheet
   (`linux-neovim`, `dev-vim`, `md-*`).
3. Do not copy Font Awesome 5/6 or fontawesome.com codepoints — they collide.

The Neovim appicon is `` (`linux-neovim`).

## Themes

Colors live in `dwm-6.8/themes/` as `key=value` files. That directory is
the only place to edit hex values. `make` in `dwm-6.8` embeds those files
into dwm, st, dmenu, and herbe. Super+t still cycles which compiled theme
is active (`~/.config/dwm/current`).

```
edit themes/papercolor  →  make  →  install / restart
```

| Key | Used by |
| --- | --- |
| `name` | Theme id written to `~/.config/dwm/current` |
| `nvim` | Neovim `colorscheme` |
| `mode` | `dark` or `light` — published as the system color-scheme |
| `fg` `bg` `border` `sel_fg` `sel_bg` `float` | dwm bar / borders, dmenu, herbe |
| `term0`–`term15` `termfg` `termbg` | st palette |

`bg` is the bar, dmenu normal background, and herbe fill. `termbg` / `term0`
are the terminal background — change both if you want them to match.

A file in `~/.config/dwm/themes/` that is **not** also in `themes/` adds a
theme without a rebuild. A live file named `nord` or `papercolor` is ignored;
the compiled copy always wins. If `mode` is omitted, it is inferred from
the luminance of `bg`.

Super+t calls `theme_cycle()`, recolors the bar, sends `ST_COLORMODE` to
every `st` window, writes `~/.config/dwm/active` (`name` / `nvim` / `mode`),
and runs `theme-apply` so GTK / portal apps follow. Neovim watches
`~/.config/dwm/` and reloads the colorscheme from `active`.

Shipped themes:

- **nord** — `mode=dark`, Neovim `nord`
- **papercolor** — `mode=light`, Neovim `PaperColor`

### System appearance

`theme-apply` (installed to `/usr/local/bin`, also run at autostart) reads
`mode` from the current theme and writes the usual Linux “prefer dark/light”
hooks:

- `~/.config/dwm/appearance` — one line, `dark` or `light`
- `gsettings` `org.gnome.desktop.interface color-scheme` and `gtk-theme`
  (`prefer-dark` / `Adwaita-dark` or `prefer-light` / `Adwaita`)
- `~/.config/gtk-3.0/settings.ini` and `gtk-4.0/settings.ini`
  (`gtk-application-prefer-dark-theme`)
- `~/.config/xsettingsd/xsettingsd.conf` (`Net/ThemeName`), then `SIGHUP`
  xsettingsd if it is running
- `~/.config/xdg-desktop-portal/portals.conf` (`default=gtk`) if missing

That is what Grok (`theme = "auto"`), Brave, Firefox, and other Electron/GTK
apps query via `org.freedesktop.appearance` / `color-scheme`. Portal-backed
clients pick up Super+t within a few seconds. GTK apps that only read
`settings.ini` at startup need a restart unless xsettingsd is running.

Grok must be set to follow the OS (this repo does not edit `~/.grok/config.toml`):

```toml
[ui]
theme = "auto"
```

In Brave / Firefox, leave appearance on **Device** / **Auto**.

## Keybindings

Modifier is **Super** (`Mod4`).

| Key | Action |
| --- | --- |
| Super+Return | `st` |
| Super+d | `dmenu_run` (colors from the current theme) |
| Super+t | Cycle theme (and publish dark/light to the system) |
| Super+m | `st -e wiremix` |
| Super+n | `wall` (wallpaper script, not in this repo) |
| Super+a | Toggle inactive-window opacity |
| Super+b | Toggle bar |
| Super+c | Kill client |
| Super+f | Fullscreen |
| Super+h / l | Shrink / grow master |
| Super+j / k | Focus stack |
| Super+u / o | Centered floating master / monocle |
| Super+z | Zoom (swap master) |
| Super+space | Toggle floating |
| Super+Tab | Preview all windows |
| Super+, / . | Focus monitor |
| Super+Shift+, / . | Send to monitor |
| Super+1..4 | View tag |
| Super+Shift+1..4 | Tag client |
| Super+0 | View all tags |
| Super+Home | `slock` |
| Super+End | `shutdown` |
| Super+Shift+q | Quit dwm |
| Print | Screenshot → `$HOME/images/screenshots/` then `nsxiv -t` |
| Super+Print | Selection screenshot, same folder |
| XF86 audio | `pactl` volume / mute, `playerctl` prev / play / next |
| XF86 WLAN | `rfkill unblock wifi` |
| XF86 brightness | `light -U 5` / `light -A 5` |

Layouts (cycle from the bar symbol): centered floating master, centered master,
floating, monocle.

Window rules: Brave uses fake-fullscreen; `st`, `imv`, `mpv`, `iv` float.

## Autostart

From the cool-autostart list in `dwm-6.8/config.h`:

1. `xset r rate 180 25 m 0 0 -dpms s off`
3. `setxkbmap -option caps:swapescape`
4. `xhidecursor`
5. `mons -s` (external monitor only, if present)
6. `wall --dry` (no-op if `wall` is not installed)
7. `dbus-update-activation-environment --systemd DISPLAY XAUTHORITY`
8. `picom --backend xrender -fc`

## Programs

Every binary below is installed to **`/usr/local/bin`**.

### dwm 6.8

Patched window manager. Applied diffs live in `dwm-6.8/patches-ok/`
(leftover unused diffs stay in `patches/`).

alpha, alttagsdecoration, alwayscenter, appicons, attachtop, autodarkmode,
bar-height, barpadding, bottomdockgap, centeredmaster, centeredwindowname,
clientindicators, cool-autostart, fadeinactive, float-border-color, fullscreen,
functionalgaps+pertag, hide vacant tags, ispermanent, noborder flicker fix,
preserveonrestart, preview-all-windows, resizecorners, selectivefakefullscreen,
statuscmd.

Build libs: `libX11`, `libXinerama`, `libXft`, `libXrender`, `fontconfig`,
`freetype`. Font: Terminess Nerd Font Mono 12 (see [Icons](#icons)).

### dmenu 5.4

Applied: alpha, floatingbar, instant, rejectnomatch.

Padded to sit on the same floating bar as dwm (`barpadh` 750, `barpadv` 10).
dwm passes the current theme colors on each spawn.

### st 0.9.3

Applied: alpha-osc11, anysize, autodarkmode, bold-is-not-bright, boxdraw,
clickurl, clipboard, colorschemes, delkey.

Palette comes from the dwm theme. Font: Terminess Nerd Font Mono
(see [Icons](#icons)).

### Bar blocks

Table in `dwm-6.8/config.h` (`blocks[]`). Helpers in `dwm-6.8/blocks/`.

| Command | Interval | Signal | Notes |
| --- | --- | --- | --- |
| `spotify-now` | 3s | 1 | MPRIS / D-Bus, Spotify only |
| `net` | 10s | 2 | sysfs wifi / ethernet icons |
| `battery` | 5s | 3 | sysfs `BAT0`; `herbe` on critical |
| `date '+%H:%M '` | 60s | 4 | |

Helpers are compiled as `battery`, `net`, and `spotify-now` into
`/usr/local/bin`. Clicking the bar sends `statuscmd` signals.

### herbe

Daemon-less notifications. Colors from the current theme. Font Terminess
Nerd Font Mono 12,
top-right, 10s.

### mons

POSIX xrandr wrapper. Autostart uses `-s` (second monitor only). Library
files go to `/usr/local/lib/libshlist`; the binary is still
`/usr/local/bin/mons`.

### nopen

MIME opener via `libmagic`. Mappings in `dwm-6.8/nopen/config.h`:

| Kind | Program |
| --- | --- |
| text / json / scripts | `nvim` |
| images | `nsxiv` |
| video | `mpv` |
| PDF | `zathura` |
| HTML | `surf` |
| audio | `mpg123` |
| NES / octet-stream | `mednafen` |
| PE | `wine` |
| directories | `cfl` (not in this repo) |

### nsxiv

Image viewer (Imlib2 + optional libexif / Xft). Print-screen pipelines open
`$HOME/images/screenshots/` in thumbnail mode.

### slock

Setuid locker. Drops to user `nobody`, group `wheel`. Imlib2 blur (radius 45).

### sent

Plaintext presentations. Image slides need [farbfeld](https://tools.suckless.org/farbfeld/)
(`2ff`).

### soap

Regex-based `xdg-open` replacement. **Off by default** — `make install` in
`soap/` overwrites the system opener. The Gentoo script installs
`/usr/local/bin/soap` always, and only installs it as `xdg-open` with
`--soap`.

### sx

Minimal Xorg starter. Reads `$XDG_CONFIG_HOME/sx/sxrc` (must be executable).
`install-gentoo.sh` writes one that `exec dwm` if you do not already have one.

### xhidecursor

Hides the pointer on keypress, shows it on mouse motion. Needs
`libX11`, `libXi`, `libXfixes`.

### Neovim

`init.vim` is the config used on this machine, with paths resolved at
runtime (`stdpath('state')/undo`, `~/.config/dwm/...`) so it works for any
user and as a system-wide `sysinit.vim`. Extra files live under `nvim/`.

It follows the dwm theme, draws a custom statusline, and enables clangd +
treesitter for C. Undo files go to `stdpath('state')/undo`
(`~/.local/state/nvim/undo` for a normal user). To load it for every account,
copy `init.vim` to `/etc/xdg/nvim/sysinit.vim`.

## External programs

**Needed to compile and run the session**

Xorg, `libX11`, `libXft`, `libXinerama`, `libXrender`, `libXext`, `libXrandr`,
`libXfixes`, `libXi`, freetype, fontconfig, Imlib2, libexif, libmagic, libdbus,
libcrypt, Iosevka, Font Awesome, DejaVu (sent fallback), picom, xset,
setxkbmap, xrandr, xauth, dbus, ncurses (`tic` for st terminfo),
`xdg-desktop-portal`, `xdg-desktop-portal-gtk`, `gsettings-desktop-schemas`,
`xsettingsd`.

**Needed for bound keys / the bar**

`scrot`, `pactl` (PipeWire + Pulse compat), `playerctl`, `light`, `rfkill`,
`wiremix`.

**Used by nopen / soap**

`nvim`, `mpv`, `zathura`, `surf`, `mpg123`, `wget`, `gifview` (gifsicle),
`mplayer`, `youtube-viewer`, `wine`, `mednafen`, `imv`.

**Not in this repo** (the session still starts without them)

| Command | What it is |
| --- | --- |
| `wall` | Wallpaper helper (`Super+n`, autostart `--dry`) |
| `cfl` | Directory opener in nopen |
| Brave | Class rule only (fake-fullscreen) |

## Gentoo install

```sh
# dependencies + every in-tree binary (not soap-as-xdg-open)
doas ./install-gentoo.sh

# also install Neovim config + Nord / PaperColor
doas ./install-gentoo.sh --nvim

# also emerge nopen/soap handlers (surf, zathura, wine, …)
doas ./install-gentoo.sh --optional --nvim

# replace xdg-open with soap (backs up, installs under /usr/local/bin)
doas ./install-gentoo.sh --soap
```

| Flag | Effect |
| --- | --- |
| `--deps-only` | emerge only |
| `--build-only` | compile / install in-tree tools only |
| `--optional` | extra Portage atoms for nopen / soap |
| `--soap` | install soap as `/usr/local/bin/xdg-open` |
| `--nvim` | install `init.vim`, ftplugins, clangd lsp, color schemes |
| `--dry-run` | print actions |
| `--force` | skip the Gentoo check |
| `-jN` | `make` jobs (default: `nproc`) |

The script detects the real user behind `doas` (`DOAS_USER`) or `sudo`
(`SUDO_USER`) for `~/.config/sx/sxrc`, `~/.config/nvim/`, and
`$HOME/images/screenshots`. It does **not** emerge distro `dwm` / `st` /
`dmenu` / `slock` / `nsxiv`. You can also run it as yourself; it elevates
individual commands with `doas`.

### Session

On a tty, as your user:

```sh
sx
```

`sxrc` is just:

```sh
#!/bin/sh
exec dwm
```

To start X on login, add `sx` to `~/.bash_profile` when the tty is a VT.

## Building by hand

```sh
make -C xhidecursor && doas make -C xhidecursor install
make -C sx PREFIX=/usr/local install
make -C dwm-6.8/dmenu-5.4 && doas make -C dwm-6.8/dmenu-5.4 install
make -C dwm-6.8/st-0.9.3 && doas make -C dwm-6.8/st-0.9.3 install
make -C dwm-6.8 && doas make -C dwm-6.8 install
make -C dwm-6.8/herbe && doas make -C dwm-6.8/herbe install
make -C dwm-6.8/mons PREFIX=/usr/local install
make -C dwm-6.8/nsxiv && doas make -C dwm-6.8/nsxiv install
make -C dwm-6.8/slock && doas make -C dwm-6.8/slock install
make -C dwm-6.8/sent && doas make -C dwm-6.8/sent install
clang -std=c23 -Wall -o nopen dwm-6.8/nopen/main.c -lmagic
doas install -m755 nopen /usr/local/bin/nopen
```

Edit `dwm-6.8/config.h` for dwm, st, dmenu, herbe, slock, and the bar.
`make` links one `dwm`. Super+Return forks `st_main`, Super+d forks
`dmenu_main`, and the bar updates inside dwm. Child `config.h` files
are two-line wrappers. After a good `make`, `config.h` is copied to
`config.def.h`.

## Caveats

- `slock` is setuid root and must be able to drop to `nobody:wheel`.
- `soap --soap` puts an `xdg-open` on your `PATH` ahead of the desktop one.
- Bar helpers install as `battery`, `net`, and `spotify-now`.
- `patches/` directories hold unused diffs. What is actually applied is in
  `patches-ok/` (and `st-0.9.3/ok/` for delkey).
