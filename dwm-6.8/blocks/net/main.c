// Linux-only net indicator (C23). Wi-Fi: radio / IP / signal / internet.
// Ethernet: cable carrier only.

#define _GNU_SOURCE

#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <ifaddrs.h>
#include <limits.h>
#include <linux/genetlink.h>
#include <linux/netlink.h>
#include <linux/nl80211.h>
#include <net/if.h>
#include <netinet/in.h>
#include <poll.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#define CONFIG_BLOCK_NET
#include "../../config.h"

#ifndef NLA_OK
#define NLA_OK(nla, rem) \
	((rem) >= (int)sizeof(struct nlattr) \
	 && (nla)->nla_len >= sizeof(struct nlattr) \
	 && (nla)->nla_len <= (rem))
#define NLA_NEXT(nla, rem) \
	((rem) -= NLA_ALIGN((nla)->nla_len), \
	 (struct nlattr *)((char *)(nla) + NLA_ALIGN((nla)->nla_len)))
#define NLA_DATA(nla) ((void *)((char *)(nla) + NLA_HDRLEN))
#define NLA_PAYLOAD(nla) ((int)((nla)->nla_len - NLA_HDRLEN))
#endif

static int
path_is_dir(const char *path)
{
	struct stat st;

	return stat(path, &st) == 0 && S_ISDIR(st.st_mode);
}

static int
read_file_trim(const char *path, char *buf, size_t bufsz)
{
	FILE *f = fopen(path, "r");
	size_t n;

	if (!f)
		return 0;
	n = fread(buf, 1, bufsz - 1, f);
	fclose(f);
	buf[n] = '\0';
	while (n > 0 && (buf[n - 1] == '\n' || buf[n - 1] == '\r'
	    || buf[n - 1] == ' ' || buf[n - 1] == '\t'))
		buf[--n] = '\0';
	return 1;
}

static int
is_wireless(const char *ifname)
{
	char path[256];

	snprintf(path, sizeof(path), "/sys/class/net/%s/wireless", ifname);
	return path_is_dir(path);
}

static int
name_is_ethernet(const char *ifname)
{
	return strncmp(ifname, "en", 2) == 0 || strncmp(ifname, "eth", 3) == 0;
}

static int
ifname_ok(const char *s)
{
	if (!s || !*s || strlen(s) >= IFNAMSIZ)
		return 0;
	for (const char *p = s; *p; p++) {
		if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')
		    || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-' || *p == '.'))
			return 0;
	}
	return 1;
}

static int
iface_admin_up(const char *ifname)
{
	char path[256], s[32];
	unsigned long flags;

	snprintf(path, sizeof(path), "/sys/class/net/%s/flags", ifname);
	if (!read_file_trim(path, s, sizeof(s)))
		return 0;
	flags = strtoul(s, NULL, 0);
	return (flags & IFF_UP) != 0;
}

/* Cable present. Unreadable carrier (typical admin-down) counts as unplugged. */
static int
has_carrier(const char *ifname)
{
	char path[256], s[32];

	snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", ifname);
	if (!read_file_trim(path, s, sizeof(s)))
		return 0;
	return strcmp(s, "1") == 0;
}

static int
wifi_rfkill(int *present, int *blocked)
{
	DIR *d = opendir("/sys/class/rfkill");
	struct dirent *de;

	*present = 0;
	*blocked = 0;
	if (!d)
		return 0;
	while ((de = readdir(d))) {
		char path[PATH_MAX], type[32], state[8];

		if (de->d_name[0] == '.')
			continue;
		snprintf(path, sizeof(path), "/sys/class/rfkill/%s/type", de->d_name);
		if (!read_file_trim(path, type, sizeof(type)))
			continue;
		if (strcmp(type, "wlan") != 0)
			continue;
		*present = 1;
		snprintf(path, sizeof(path), "/sys/class/rfkill/%s/state", de->d_name);
		if (read_file_trim(path, state, sizeof(state)) && strcmp(state, "1") != 0) {
			*blocked = 1;
			break;
		}
	}
	closedir(d);
	return 1;
}

static int
iface_has_global_ip(const char *ifname)
{
	struct ifaddrs *ifa, *p;
	int ok = 0;

	if (getifaddrs(&ifa) != 0)
		return 0;
	for (p = ifa; p; p = p->ifa_next) {
		if (!p->ifa_addr || strcmp(p->ifa_name, ifname) != 0)
			continue;
		if (p->ifa_addr->sa_family == AF_INET) {
			uint32_t a = ntohl(((struct sockaddr_in *)p->ifa_addr)->sin_addr.s_addr);

			if ((a >> 24) == 127)
				continue;
			if ((a & 0xffff0000u) == 0xa9fe0000u) /* 169.254.0.0/16 */
				continue;
			ok = 1;
			break;
		}
		if (p->ifa_addr->sa_family == AF_INET6) {
			const struct in6_addr *a = &((struct sockaddr_in6 *)p->ifa_addr)->sin6_addr;

			if (IN6_IS_ADDR_LOOPBACK(a) || IN6_IS_ADDR_LINKLOCAL(a))
				continue;
			ok = 1;
			break;
		}
	}
	freeifaddrs(ifa);
	return ok;
}

static int
nla_add(struct nlmsghdr *n, size_t maxlen, uint16_t type, const void *data, uint16_t dlen)
{
	uint16_t tlen = NLA_HDRLEN + dlen;
	struct nlattr *nla;

	if (NLMSG_ALIGN(n->nlmsg_len) + NLA_ALIGN(tlen) > maxlen)
		return -1;
	nla = (struct nlattr *)((char *)n + NLMSG_ALIGN(n->nlmsg_len));
	nla->nla_type = type;
	nla->nla_len = tlen;
	if (dlen)
		memcpy((char *)nla + NLA_HDRLEN, data, dlen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + NLA_ALIGN(tlen);
	return 0;
}

static int
nl_open(void)
{
	int fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_GENERIC);
	struct sockaddr_nl sa = { .nl_family = AF_NETLINK };
	struct timeval tv = { .tv_usec = 200000 };

	if (fd < 0)
		return -1;
	if (bind(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
		close(fd);
		return -1;
	}
	setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
	return fd;
}

static int
nl_family_id(int fd)
{
	struct {
		struct nlmsghdr n;
		struct genlmsghdr g;
		char attr[64];
	} req;
	char buf[4096];
	ssize_t nr;
	struct nlmsghdr *nh;
	struct genlmsghdr *gh;
	struct nlattr *a;
	int rem;
	const char name[] = "nl80211";

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.g));
	req.n.nlmsg_type = GENL_ID_CTRL;
	req.n.nlmsg_flags = NLM_F_REQUEST;
	req.n.nlmsg_seq = 1;
	req.g.cmd = CTRL_CMD_GETFAMILY;
	req.g.version = 1;
	if (nla_add(&req.n, sizeof(req), CTRL_ATTR_FAMILY_NAME, name, sizeof(name)) < 0)
		return -1;
	if (send(fd, &req, req.n.nlmsg_len, 0) < 0)
		return -1;
	nr = recv(fd, buf, sizeof(buf), 0);
	if (nr < 0)
		return -1;
	nh = (struct nlmsghdr *)buf;
	if (!NLMSG_OK(nh, (unsigned)nr) || nh->nlmsg_type == NLMSG_ERROR)
		return -1;
	gh = NLMSG_DATA(nh);
	rem = NLMSG_PAYLOAD(nh, sizeof(*gh));
	for (a = (struct nlattr *)(gh + 1); NLA_OK(a, rem); a = NLA_NEXT(a, rem)) {
		if ((a->nla_type & NLA_TYPE_MASK) == CTRL_ATTR_FAMILY_ID
		    && NLA_PAYLOAD(a) >= 2)
			return *(uint16_t *)NLA_DATA(a);
	}
	return -1;
}

static int
nl_parse_signal(struct nlattr *sta, int *rssi)
{
	int rem = NLA_PAYLOAD(sta);
	int have_avg = 0, have = 0, avg = 0, sig = 0;
	struct nlattr *s;

	for (s = NLA_DATA(sta); NLA_OK(s, rem); s = NLA_NEXT(s, rem)) {
		uint16_t t = s->nla_type & NLA_TYPE_MASK;

		if (t == NL80211_STA_INFO_SIGNAL_AVG && NLA_PAYLOAD(s) >= 1) {
			avg = *(int8_t *)NLA_DATA(s);
			have_avg = 1;
		} else if (t == NL80211_STA_INFO_SIGNAL && NLA_PAYLOAD(s) >= 1) {
			sig = *(int8_t *)NLA_DATA(s);
			have = 1;
		}
	}
	if (have_avg) {
		*rssi = avg;
		return 1;
	}
	if (have) {
		*rssi = sig;
		return 1;
	}
	return 0;
}

static int
nl_station_rssi(int fd, int family, unsigned ifindex, int *rssi)
{
	struct {
		struct nlmsghdr n;
		struct genlmsghdr g;
		char attr[64];
	} req;
	uint32_t idx = ifindex;
	int found = 0;

	memset(&req, 0, sizeof(req));
	req.n.nlmsg_len = NLMSG_LENGTH(sizeof(req.g));
	req.n.nlmsg_type = (uint16_t)family;
	req.n.nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
	req.n.nlmsg_seq = 2;
	req.g.cmd = NL80211_CMD_GET_STATION;
	if (nla_add(&req.n, sizeof(req), NL80211_ATTR_IFINDEX, &idx, sizeof(idx)) < 0)
		return 0;
	if (send(fd, &req, req.n.nlmsg_len, 0) < 0)
		return 0;

	for (;;) {
		char buf[8192];
		ssize_t nr = recv(fd, buf, sizeof(buf), 0);
		int rem;
		struct nlmsghdr *nh;

		if (nr < 0)
			break;
		rem = (int)nr;
		for (nh = (struct nlmsghdr *)buf; NLMSG_OK(nh, (unsigned)rem); nh = NLMSG_NEXT(nh, rem)) {
			struct genlmsghdr *gh;
			struct nlattr *a;
			int arem;

			if (nh->nlmsg_type == NLMSG_DONE)
				return found;
			if (nh->nlmsg_type == NLMSG_ERROR)
				return found;
			if (nh->nlmsg_type != (uint16_t)family)
				continue;
			gh = NLMSG_DATA(nh);
			arem = NLMSG_PAYLOAD(nh, sizeof(*gh));
			for (a = (struct nlattr *)(gh + 1); NLA_OK(a, arem); a = NLA_NEXT(a, arem)) {
				if ((a->nla_type & NLA_TYPE_MASK) == NL80211_ATTR_STA_INFO
				    && nl_parse_signal(a, rssi))
					found = 1;
			}
		}
	}
	return found;
}

static int
rssi_nl80211(const char *ifname, int *rssi)
{
	unsigned ifindex = if_nametoindex(ifname);
	int fd, family, ok;

	if (!ifindex)
		return 0;
	fd = nl_open();
	if (fd < 0)
		return 0;
	family = nl_family_id(fd);
	if (family < 0) {
		close(fd);
		return 0;
	}
	ok = nl_station_rssi(fd, family, ifindex, rssi);
	close(fd);
	return ok;
}

static int
parse_dbm(const char *line, int *out)
{
	const char *p = line;
	char *end;
	long v;

	while (*p && *p != '-' && (*p < '0' || *p > '9'))
		p++;
	if (!*p)
		return 0;
	v = strtol(p, &end, 10);
	if (end == p)
		return 0;
	*out = (int)v;
	return 1;
}

static int
rssi_iwctl(const char *ifname, int *rssi)
{
	char cmd[128], line[256];
	FILE *f;
	int avg = 0, inst = 0, have_avg = 0, have_inst = 0;

	if (!ifname_ok(ifname))
		return 0;
	snprintf(cmd, sizeof(cmd), "iwctl station %s show 2>/dev/null", ifname);
	f = popen(cmd, "r");
	if (!f)
		return 0;
	while (fgets(line, sizeof(line), f)) {
		if (strstr(line, "AverageRSSI")) {
			if (parse_dbm(line, &avg))
				have_avg = 1;
		} else if (strstr(line, "RSSI")) {
			if (parse_dbm(line, &inst))
				have_inst = 1;
		}
	}
	pclose(f);
	if (have_avg) {
		*rssi = avg;
		return 1;
	}
	if (have_inst) {
		*rssi = inst;
		return 1;
	}
	return 0;
}

static int
wifi_rssi(const char *ifname)
{
	int rssi = 0;

	if (rssi_nl80211(ifname, &rssi) || rssi_iwctl(ifname, &rssi))
		return rssi;
	return -100;
}

static int
rssi_bars(int dbm)
{
	if (dbm >= NET_RSSI_BARS[3])
		return 4;
	if (dbm >= NET_RSSI_BARS[2])
		return 3;
	if (dbm >= NET_RSSI_BARS[1])
		return 2;
	if (dbm >= NET_RSSI_BARS[0])
		return 1;
	return 0;
}

static int
tcp_reaches(const char *ip, int timeout_ms)
{
	int fd, err = 0, r;
	socklen_t elen = sizeof(err);
	struct sockaddr_in sa = { .sin_family = AF_INET, .sin_port = htons(443) };
	struct pollfd pfd;

	if (inet_pton(AF_INET, ip, &sa.sin_addr) != 1)
		return 0;
	fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (fd < 0)
		return 0;
	r = connect(fd, (struct sockaddr *)&sa, sizeof(sa));
	if (r == 0) {
		close(fd);
		return 1;
	}
	if (errno != EINPROGRESS) {
		close(fd);
		return 0;
	}
	pfd = (struct pollfd){ .fd = fd, .events = POLLOUT };
	r = poll(&pfd, 1, timeout_ms);
	if (r > 0)
		getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &elen);
	close(fd);
	return r > 0 && err == 0;
}

static int
internet_ok(void)
{
	size_t i, n = sizeof NET_CHECK_IP / sizeof NET_CHECK_IP[0];

	for (i = 0; i < n; i++)
		if (tcp_reaches(NET_CHECK_IP[i], NET_CHECK_MS))
			return 1;
	return 0;
}

static const char *
wifi_icon(const char *ifname, int blocked)
{
	int bars;

	if (!ifname[0] || blocked || !iface_admin_up(ifname))
		return WIFI_DISABLED;
	if (!iface_has_global_ip(ifname))
		return WIFI_NO_IP;
	bars = rssi_bars(wifi_rssi(ifname));
	if (internet_ok()) {
		if (bars < 1)
			bars = 1;
		return WIFI_ON[bars - 1];
	}
	return WIFI_NOINT[bars];
}

static void
find_ifaces(char *wifi, size_t wlen, char *eth, size_t elen)
{
	DIR *root = opendir("/sys/class/net");
	struct dirent *de;

	wifi[0] = eth[0] = '\0';
	if (!root)
		return;
	while ((de = readdir(root))) {
		if (de->d_name[0] == '.' || strcmp(de->d_name, "lo") == 0)
			continue;
		if (!wifi[0] && is_wireless(de->d_name))
			snprintf(wifi, wlen, "%s", de->d_name);
		else if (!eth[0] && name_is_ethernet(de->d_name))
			snprintf(eth, elen, "%s", de->d_name);
	}
	closedir(root);
}

int
main(void)
{
	char wifi[IFNAMSIZ] = { 0 }, eth[IFNAMSIZ] = { 0 };
	int rfkill_present = 0, rfkill_blocked = 0;
	const char *wicon = NULL, *eicon = NULL;

	find_ifaces(wifi, sizeof(wifi), eth, sizeof(eth));
	wifi_rfkill(&rfkill_present, &rfkill_blocked);

	if (wifi[0] || rfkill_present)
		wicon = wifi_icon(wifi, rfkill_blocked);
	if (eth[0])
		eicon = has_carrier(eth) ? ETH_PLUGGED : ETH_UNPLUGGED;

	if (wicon)
		fputs(wicon, stdout);
	if (wicon && eicon)
		fputc(' ', stdout);
	if (eicon)
		fputs(eicon, stdout);
	return 0;
}
