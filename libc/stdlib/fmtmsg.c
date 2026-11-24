/*
 * fmtmsg, POSIX.1-2008 (XSI).
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <fmtmsg.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

enum { V_LABEL = 1, V_SEVERITY = 2, V_TEXT = 4, V_ACTION = 8, V_TAG = 16,
       V_ALL = 31 };

/* The components MSGVERB selects for standard error: all of them if it
   is unset, empty or has a word it does not know.  A trailing colon is
   accepted, as in glibc.  */
static int msgverb(void)
{
	static const char words[] = "label\0severity\0text\0action\0tag\0";
	const char *v = getenv("MSGVERB"), *w;
	int sel = 0, bit;
	size_t n;

	if (v == NULL || *v == '\0')
		return V_ALL;
	for (;;) {
		n = strcspn(v, ":");
		for (w = words, bit = 1; *w; w += strlen(w) + 1, bit <<= 1)
			if (strlen(w) == n && memcmp(v, w, n) == 0)
				break;
		if (*w == '\0')
			return V_ALL;
		sel |= bit;
		if (v[n] == '\0' || v[n + 1] == '\0')
			return sel;
		v += n + 1;
	}
}

#define FMT "%s%s%s%s%s%s%s%s%s%s\n"

/* The ten strings of FMT for "label: SEVERITY: text\nTO FIX: action tag",
   leaving out what is null or not selected and the separators that are
   then not needed.  */
static void parts(const char **p, int sel, const char *label,
		  const char *sev, const char *text, const char *action,
		  const char *tag)
{
	if (!(sel & V_LABEL))
		label = NULL;
	if (!(sel & V_SEVERITY))
		sev = NULL;
	if (!(sel & V_TEXT))
		text = NULL;
	if (!(sel & V_ACTION))
		action = NULL;
	if (!(sel & V_TAG))
		tag = NULL;

	p[0] = label ? label : "";
	p[1] = label && (sev || text || action || tag) ? ": " : "";
	p[2] = sev ? sev : "";
	p[3] = sev && (text || action || tag) ? ": " : "";
	p[4] = text ? text : "";
	p[5] = text && (action || tag) ? "\n" : "";
	p[6] = action ? "TO FIX: " : "";
	p[7] = action ? action : "";
	p[8] = action && tag ? " " : "";
	p[9] = tag ? tag : "";
}

int fmtmsg(long classification, const char *label, int severity,
	   const char *text, const char *action, const char *tag)
{
	static const char sevs[] = "\0HALT\0ERROR\0WARNING\0INFO";
	static const unsigned char sevoff[] = { 0, 1, 6, 12, 20 };
	const char *sev, *colon, *p[10];
	int failed = 0, fd;

	if (label != MM_NULLLBL) {
		/* Two fields of at most 10 and 14 bytes.  */
		colon = strchr(label, ':');
		if (colon == NULL || colon - label > 10 || strlen(colon + 1) > 14)
			return MM_NOTOK;
	}
	if (severity < MM_NOSEV || severity > MM_INFO)
		return MM_NOTOK;
	sev = severity == MM_NOSEV ? NULL : sevs + sevoff[severity];

	if (classification & MM_PRINT) {
		parts(p, msgverb(), label, sev, text, action, tag);
		if (fprintf(stderr, FMT, p[0], p[1], p[2], p[3], p[4], p[5],
			    p[6], p[7], p[8], p[9]) < 0)
			failed |= MM_PRINT;
	}
	if (classification & MM_CONSOLE) {
		parts(p, V_ALL, label, sev, text, action, tag);
		fd = open("/dev/console", O_WRONLY | O_NOCTTY | O_CLOEXEC);
		if (fd < 0 || dprintf(fd, FMT, p[0], p[1], p[2], p[3], p[4],
				      p[5], p[6], p[7], p[8], p[9]) < 0)
			failed |= MM_CONSOLE;
		if (fd >= 0)
			close(fd);
	}

	if (failed == 0)
		return MM_OK;
	if (failed == (MM_PRINT | MM_CONSOLE))
		return MM_NOTOK;
	return failed == MM_PRINT ? MM_NOMSG : MM_NOCON;
}
