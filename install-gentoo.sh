#!/usr/bin/env bash
# Install this rice on Gentoo: emerge deps, build every in-tree program
# into /usr/local/bin, optionally drop Neovim / sx session files.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PREFIX=/usr/local
BINDIR="${PREFIX}/bin"
USE_FILE=/etc/portage/package.use/config-files
KEYWORDS_FILE=/etc/portage/package.accept_keywords/config-files
USE_MARKER="# managed by config-files/install-gentoo.sh"

DEPS_ONLY=0
BUILD_ONLY=0
OPTIONAL=0
SOAP=0
NVIM=0
DRY_RUN=0
FORCE=0
JOBS="$(nproc 2>/dev/null || echo 1)"

EMERGED=()
BUILT=()
SKIPPED=()
WARNED=()

usage() {
	cat <<'EOF'
Usage: install-gentoo.sh [options]

  --deps-only    emerge packages only
  --build-only   compile and install in-tree programs only
  --optional     also emerge nopen/soap handlers (surf, zathura, wine, …)
  --soap         install soap as /usr/local/bin/xdg-open
  --nvim         install init.vim, ftplugins, clangd lsp, color schemes
  --dry-run      print actions without running them
  --force        skip the Gentoo check
  -jN            make -jN (default: nproc)
  -h, --help     this help
EOF
}

log()  { printf '==> %s\n' "$*"; }
warn() { printf '!!  %s\n' "$*" >&2; WARNED+=("$*"); }
die()  { printf 'error: %s\n' "$*" >&2; exit 1; }

run() {
	if (( DRY_RUN )); then
		printf 'DRY  '
		printf '%q ' "$@"
		printf '\n'
		return 0
	fi
	"$@"
}

as_root() {
	if (( DRY_RUN )); then
		run "$@"
		return 0
	fi
	if (( EUID == 0 )); then
		"$@"
	elif command -v doas >/dev/null 2>&1; then
		doas "$@"
	elif command -v sudo >/dev/null 2>&1; then
		sudo --preserve-env=HOME,SUDO_USER,SUDO_UID,SUDO_GID -- "$@"
	else
		die "need root (doas or sudo) for: $*"
	fi
}

as_user() {
	if (( DRY_RUN )); then
		run "$@"
		return 0
	fi
	if (( EUID != 0 )) || [[ -z "${TARGET_USER:-}" || "${TARGET_USER}" == root ]]; then
		"$@"
		return
	fi
	if command -v runuser >/dev/null 2>&1; then
		runuser -u "${TARGET_USER}" -- "$@"
	else
		su -s /bin/sh "${TARGET_USER}" -c "$(printf '%q ' "$@")"
	fi
}

while [[ $# -gt 0 ]]; do
	case "$1" in
		--deps-only)  DEPS_ONLY=1 ;;
		--build-only) BUILD_ONLY=1 ;;
		--optional)   OPTIONAL=1 ;;
		--soap)       SOAP=1 ;;
		--nvim)       NVIM=1 ;;
		--dry-run)    DRY_RUN=1 ;;
		--force)      FORCE=1 ;;
		-j*)          JOBS="${1#-j}" ;;
		-h|--help)    usage; exit 0 ;;
		*)            die "unknown option: $1" ;;
	esac
	shift
done

if (( DEPS_ONLY && BUILD_ONLY )); then
	die "--deps-only and --build-only cannot be combined"
fi

if (( ! FORCE )); then
	if [[ ! -r /etc/os-release ]]; then
		die "cannot read /etc/os-release (use --force to override)"
	fi
	# shellcheck disable=SC1091
	. /etc/os-release
	if [[ "${ID:-}" != gentoo ]]; then
		die "this script is for Gentoo (ID=${ID:-unknown}); use --force to override"
	fi
fi

if [[ -n "${DOAS_USER:-}" && "${DOAS_USER}" != root ]]; then
	TARGET_USER="${DOAS_USER}"
elif [[ -n "${SUDO_USER:-}" && "${SUDO_USER}" != root ]]; then
	TARGET_USER="${SUDO_USER}"
elif (( EUID == 0 )); then
	TARGET_USER="$(logname 2>/dev/null || true)"
	if [[ -z "${TARGET_USER}" || "${TARGET_USER}" == root ]]; then
		TARGET_USER="${TARGET_USER:-root}"
		warn "running as root with no DOAS_USER/SUDO_USER; user files go to /root"
	fi
else
	TARGET_USER="$(id -un)"
fi

TARGET_HOME="$(getent passwd "${TARGET_USER}" | cut -d: -f6)"
[[ -n "${TARGET_HOME}" ]] || die "cannot resolve home for ${TARGET_USER}"

log "user=${TARGET_USER} home=${TARGET_HOME} prefix=${PREFIX} jobs=${JOBS}"

atom_visible() {
	local atom="$1"
	if command -v portageq >/dev/null 2>&1; then
		portageq best_visible / "${atom}" >/dev/null 2>&1
	else
		return 0
	fi
}

REQUIRED_PKGS=(
	sys-devel/gcc
	sys-devel/clang
	sys-devel/make
	dev-util/pkgconf
	sys-libs/ncurses
	x11-libs/libX11
	x11-libs/libXft
	x11-libs/libXinerama
	x11-libs/libXrender
	x11-libs/libXext
	x11-libs/libXrandr
	x11-libs/libXfixes
	x11-libs/libXi
	x11-base/xorg-proto
	media-libs/freetype
	media-libs/fontconfig
	media-libs/imlib2
	media-libs/libexif
	sys-apps/file
	sys-apps/dbus
	sys-libs/libxcrypt
	x11-base/xorg-server
	x11-apps/xauth
	x11-apps/xset
	x11-apps/setxkbmap
	x11-apps/xrandr
	x11-apps/xprop
	x11-misc/picom
	x11-misc/xsettingsd
	sys-apps/xdg-desktop-portal
	sys-apps/xdg-desktop-portal-gtk
	gnome-base/gsettings-desktop-schemas
	media-fonts/iosevka
	media-fonts/fontawesome
	media-fonts/symbols-nerd-font
	media-fonts/noto-emoji
	media-fonts/dejavu
	media-gfx/scrot
	sys-power/light
	media-sound/playerctl
	media-video/pipewire
	media-video/wireplumber
	media-sound/pulseaudio
	media-sound/wiremix
	app-editors/neovim
	media-video/mpv
	media-gfx/farbfeld
	sys-apps/util-linux
	net-misc/wget
	dev-vcs/git
	app-arch/unzip
)

OPTIONAL_PKGS=(
	www-client/surf
	app-text/zathura
	app-text/zathura-pdf-mupdf
	media-sound/mpg123
	media-gfx/gifsicle
	media-video/mplayer
	media-gfx/imv
	games-emulation/mednafen
	app-emulation/wine
	net-misc/youtube-viewer
)

write_portage_dropin() {
	local dest="$1" dir body
	dir="$(dirname "${dest}")"
	body=$(cat)
	if [[ -e "${dest}" ]] && ! grep -qF "${USE_MARKER}" "${dest}" 2>/dev/null; then
		warn "${dest} exists and is not owned by this script; leaving it alone"
		return 0
	fi
	if (( DRY_RUN )); then
		printf 'DRY  write %s\n' "${dest}"
		return 0
	fi
	if [[ -d "${dir}" ]]; then
		as_root tee "${dest}" >/dev/null <<<"${body}"
	elif [[ -f "${dir}" ]]; then
		if grep -qF "${USE_MARKER}" "${dir}" 2>/dev/null; then
			return 0
		fi
		as_root tee -a "${dir}" >/dev/null <<<"${body}"
	else
		as_root mkdir -p "${dir}"
		as_root tee "${dest}" >/dev/null <<<"${body}"
	fi
}

write_package_use() {
	write_portage_dropin "${USE_FILE}" <<EOF
${USE_MARKER}
media-libs/imlib2 X
media-video/pipewire sound-server
x11-base/xorg-server udev
media-fonts/iosevka iosevka
EOF
}

write_package_keywords() {
	write_portage_dropin "${KEYWORDS_FILE}" <<EOF
${USE_MARKER}
media-fonts/symbols-nerd-font ~amd64
EOF
}

install_fontconfig() {
	local src="${ROOT}/fontconfig/10-nerd-symbols.conf"
	local dest=/etc/fonts/conf.d/10-nerd-symbols.conf
	[[ -f "${src}" ]] || die "missing ${src}"
	log "install ${dest}"
	if (( DRY_RUN )); then
		return 0
	fi
	as_root install -m644 "${src}" "${dest}"
	as_root fc-cache -f
}

install_terminess_font() {
	local zip="${ROOT}/fonts/Terminus.zip"
	local dest=/usr/local/share/fonts/terminess-nerd-font
	local tmp
	[[ -f "${zip}" ]] || die "missing ${zip}"
	log "install Terminess Nerd Font → ${dest}"
	if (( DRY_RUN )); then
		return 0
	fi
	tmp="$(mktemp -d)"
	unzip -qo "${zip}" -d "${tmp}"
	as_root mkdir -p "${dest}"
	as_root install -m644 "${tmp}"/*.ttf "${dest}/"
	rm -rf "${tmp}"
	as_root fc-cache -f "${dest}"
}

emerge_set() {
	local -a want=() have=()
	local atom
	for atom in "$@"; do
		if atom_visible "${atom}"; then
			want+=("${atom}")
		else
			warn "not in configured repos, skipping: ${atom}"
			SKIPPED+=("${atom}")
		fi
	done
	((${#want[@]})) || return 0
	local -a emerge=(emerge --noreplace --quiet-build=y --jobs="${JOBS}")
	if [[ -t 0 ]]; then
		emerge+=(--ask)
	fi
	log "emerge ${want[*]}"
	as_root "${emerge[@]}" "${want[@]}"
	have=("${want[@]}")
	EMERGED+=("${have[@]}")
}

install_bin() {
	local src="$1" name="${2:-$(basename "$1")}"
	as_root install -d "${BINDIR}"
	as_root install -m755 "${src}" "${BINDIR}/${name}"
}

make_install() {
	local dir="$1"
	shift
	log "make -C ${dir} $*"
	run make -C "${dir}" -j"${JOBS}" "$@"
	as_root make -C "${dir}" install "$@"
	BUILT+=("${dir}")
}

# Keep existing config.h (do not let make recreate it from config.def.h).
touch_config() {
	local f
	for f in "$@"; do
		[[ -f "${f}" ]] && run touch "${f}"
	done
}

compile_blocks() {
	local blocks="${ROOT}/dwm-6.8/blocks"
	local dbus_cflags dbus_libs
	dbus_cflags="$(pkg-config --cflags dbus-1 2>/dev/null || true)"
	dbus_libs="$(pkg-config --libs dbus-1 2>/dev/null || echo -ldbus-1)"

	log "compile bar helpers → ${BINDIR}"
	run cc -std=c23 -Wall -Wextra -O2 -o "${blocks}/battery/battery" "${blocks}/battery/main.c"
	run cc -std=c23 -Wall -Wextra -O2 -o "${blocks}/net/net" "${blocks}/net/main.c"
	run cc -std=c23 -Wall -Wextra -O2 ${dbus_cflags} \
		-o "${blocks}/spotify-notify/spotify-now" \
		"${blocks}/spotify-notify/main.c" ${dbus_libs}

	install_bin "${blocks}/battery/battery" battery
	install_bin "${blocks}/net/net" net
	install_bin "${blocks}/spotify-notify/spotify-now" spotify-now
	BUILT+=("bar helpers")
}

compile_nopen() {
	local dir="${ROOT}/dwm-6.8/nopen"
	log "compile nopen"
	run clang -std=c23 -Wall -Wextra -pedantic -O2 -o "${dir}/nopen" "${dir}/main.c" -lmagic
	install_bin "${dir}/nopen" nopen
	BUILT+=("nopen")
}

compile_soap() {
	local dir="${ROOT}/soap"
	log "compile soap"
	run make -C "${dir}" -j"${JOBS}"
	install_bin "${dir}/soap" soap
	BUILT+=("soap")
	if (( SOAP )); then
		if [[ -e "${BINDIR}/xdg-open" && ! -e "${BINDIR}/xdg-open_" ]]; then
			as_root cp -a "${BINDIR}/xdg-open" "${BINDIR}/xdg-open_"
		fi
		as_root install -m755 "${dir}/soap" "${BINDIR}/xdg-open"
		log "installed soap as ${BINDIR}/xdg-open"
	fi
}

write_sxrc() {
	local cfg="${TARGET_HOME}/.config/sx"
	local sxrc="${cfg}/sxrc"
	if [[ -e "${sxrc}" ]]; then
		log "keeping existing ${sxrc}"
		return 0
	fi
	log "write ${sxrc}"
	if (( DRY_RUN )); then
		return 0
	fi
	as_root mkdir -p "${cfg}"
	as_root tee "${sxrc}" >/dev/null <<'EOF'
#!/bin/sh
exec dwm
EOF
	as_root chmod 755 "${sxrc}"
	as_root chown -R "${TARGET_USER}:${TARGET_USER}" "${cfg}"
}

ensure_screenshot_dir() {
	local dir="${TARGET_HOME}/images/screenshots"
	log "mkdir ${dir}"
	if (( DRY_RUN )); then
		return 0
	fi
	as_root mkdir -p "${dir}"
	as_root chown -R "${TARGET_USER}:${TARGET_USER}" "${TARGET_HOME}/images"
}

install_nvim() {
	local dest="${TARGET_HOME}/.config/nvim"
	local pack="${dest}/pack/themes/start"
	log "install Neovim config → ${dest}"
	if (( DRY_RUN )); then
		return 0
	fi
	as_root mkdir -p "${dest}/after/ftplugin" "${dest}/lsp" "${pack}" \
		"${TARGET_HOME}/.local/state/nvim/undo"
	as_root chown -R "${TARGET_USER}:${TARGET_USER}" "${dest}" \
		"${TARGET_HOME}/.local/state/nvim"

	if [[ -f "${dest}/init.vim" ]] && ! cmp -s "${ROOT}/init.vim" "${dest}/init.vim"; then
		as_root cp -a "${dest}/init.vim" "${dest}/init.vim.bak.$(date +%Y%m%d%H%M%S)"
	fi
	as_root cp "${ROOT}/init.vim" "${dest}/init.vim"
	as_root cp "${ROOT}/nvim/after/ftplugin/c.vim" "${dest}/after/ftplugin/c.vim"
	as_root cp "${ROOT}/nvim/after/ftplugin/cpp.vim" "${dest}/after/ftplugin/cpp.vim"
	as_root cp "${ROOT}/nvim/lsp/clangd.lua" "${dest}/lsp/clangd.lua"
	as_root chown -R "${TARGET_USER}:${TARGET_USER}" "${dest}"

	if ! command -v git >/dev/null 2>&1; then
		warn "git not found; skip Nord / PaperColor clones (emerge dev-vcs/git)"
	else
		clone_theme() {
			local url="$1" name="$2"
			if [[ -d "${pack}/${name}/.git" ]]; then
				as_user git -C "${pack}/${name}" pull --ff-only || warn "git pull failed: ${name}"
			else
				as_root rm -rf "${pack}/${name}"
				as_user git clone --depth=1 "${url}" "${pack}/${name}" \
					|| warn "git clone failed: ${url}"
			fi
		}
		clone_theme https://github.com/nordtheme/vim.git nord-vim
		clone_theme https://github.com/NLKNguyen/papercolor-theme.git papercolor-theme
	fi

	as_root chown -R "${TARGET_USER}:${TARGET_USER}" "${dest}" \
		"${TARGET_HOME}/.local/state/nvim"
	BUILT+=("nvim config")
}

if (( ! BUILD_ONLY )); then
	write_package_use
	write_package_keywords
	emerge_set "${REQUIRED_PKGS[@]}"
	if (( OPTIONAL )); then
		emerge_set "${OPTIONAL_PKGS[@]}"
	fi
fi

install_fontconfig
install_terminess_font

if (( ! DEPS_ONLY )); then
	touch_config \
		"${ROOT}/dwm-6.8/config.h" \
		"${ROOT}/dwm-6.8/config.def.h" \
		"${ROOT}/dwm-6.8/dmenu-5.4/config.h" \
		"${ROOT}/dwm-6.8/st-0.9.3/config.h" \
		"${ROOT}/dwm-6.8/nsxiv/config.h" \
		"${ROOT}/dwm-6.8/slock/config.h" \
		"${ROOT}/dwm-6.8/sent/config.h" \
		"${ROOT}/dwm-6.8/herbe/config.h"

	make_install "${ROOT}/xhidecursor"
	as_root make -C "${ROOT}/sx" PREFIX="${PREFIX}" install
	BUILT+=("sx")
	make_install "${ROOT}/dwm-6.8/dmenu-5.4"
	make_install "${ROOT}/dwm-6.8/st-0.9.3"
	make_install "${ROOT}/dwm-6.8"
	compile_blocks
	make_install "${ROOT}/dwm-6.8/herbe"
	# Binary still lands in ${PREFIX}/bin; support files under ${PREFIX}/{lib,share}.
	as_root make -C "${ROOT}/dwm-6.8/mons" PREFIX="${PREFIX}" install
	BUILT+=("mons")
	compile_nopen
	make_install "${ROOT}/dwm-6.8/nsxiv"
	make_install "${ROOT}/dwm-6.8/slock"
	make_install "${ROOT}/dwm-6.8/sent"
	compile_soap
	write_sxrc
	ensure_screenshot_dir
	if (( NVIM )); then
		install_nvim
	fi
fi

cat <<EOF

----------------------------------------------------------------
done.

user     ${TARGET_USER} (${TARGET_HOME})
prefix   ${BINDIR}

emerged  ${EMERGED[*]:-(none this run)}
built    ${BUILT[*]:-(none this run)}
skipped  ${SKIPPED[*]:-(none)}
warnings ${#WARNED[@]}

Start the session on a tty as ${TARGET_USER}:
    sx

Not shipped here (session still starts without them):
    wall    wallpaper helper (Super+n, autostart --dry)
    cfl     nopen directory opener
    Brave   class rule only

slock is setuid root and drops to nobody:wheel.
$( (( SOAP )) || printf 'soap is at %s/soap; pass --soap to also install it as xdg-open.\n' "${BINDIR}" )
----------------------------------------------------------------
EOF
