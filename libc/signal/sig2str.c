/*
 * sig2str and str2sig, POSIX.1-2024.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <signal.h>
#include <stdlib.h>
#include <string.h>

/* Where two names share a number, the first is the one sig2str gives.  */
static const struct {
	unsigned char num;
	char name[7];
} sigtab[] = {
	{ SIGHUP, "HUP" }, { SIGINT, "INT" }, { SIGQUIT, "QUIT" },
	{ SIGILL, "ILL" }, { SIGTRAP, "TRAP" }, { SIGABRT, "ABRT" },
#ifdef SIGIOT
	{ SIGIOT, "IOT" },
#endif
#ifdef SIGEMT
	{ SIGEMT, "EMT" },
#endif
	{ SIGBUS, "BUS" }, { SIGFPE, "FPE" }, { SIGKILL, "KILL" },
	{ SIGUSR1, "USR1" }, { SIGSEGV, "SEGV" }, { SIGUSR2, "USR2" },
	{ SIGPIPE, "PIPE" }, { SIGALRM, "ALRM" }, { SIGTERM, "TERM" },
#ifdef SIGSTKFLT
	{ SIGSTKFLT, "STKFLT" },
#endif
	{ SIGCHLD, "CHLD" },
#ifdef SIGCLD
	{ SIGCLD, "CLD" },
#endif
	{ SIGCONT, "CONT" }, { SIGSTOP, "STOP" }, { SIGTSTP, "TSTP" },
	{ SIGTTIN, "TTIN" }, { SIGTTOU, "TTOU" }, { SIGURG, "URG" },
	{ SIGXCPU, "XCPU" }, { SIGXFSZ, "XFSZ" }, { SIGVTALRM, "VTALRM" },
	{ SIGPROF, "PROF" }, { SIGWINCH, "WINCH" },
#ifdef SIGIO
	{ SIGIO, "IO" },
#endif
#ifdef SIGPOLL
	{ SIGPOLL, "POLL" },
#endif
#ifdef SIGPWR
	{ SIGPWR, "PWR" },
#endif
#ifdef SIGINFO
	{ SIGINFO, "INFO" },
#endif
#ifdef SIGLOST
	{ SIGLOST, "LOST" },
#endif
	{ SIGSYS, "SYS" },
#ifdef SIGUNUSED
	{ SIGUNUSED, "UNUSED" },
#endif
};

static void name_off(char *str, const char *base, char sign, int n)
{
	char *p;
	int k;

	strcpy(str, base);
	p = str + 5;
	*p++ = sign;
	for (k = n; k >= 10; k /= 10)
		p++;
	p[1] = '\0';
	do {
		*p-- = '0' + n % 10;
		n /= 10;
	} while (n);
}

int sig2str(int signum, char *str)
{
	int min = SIGRTMIN, max = SIGRTMAX;
	size_t i;

	for (i = 0; i < sizeof(sigtab) / sizeof(sigtab[0]); i++)
		if (sigtab[i].num == signum) {
			strcpy(str, sigtab[i].name);
			return 0;
		}
	/* Without __NR_rt_sigaction both ends are -1 and there is no
	   realtime range at all; "RTMIN" would otherwise name signal -1.  */
	if (min < 1 || max < min || signum < min || signum > max)
		return -1;
	if (signum == min)
		strcpy(str, "RTMIN");
	else if (signum == max)
		strcpy(str, "RTMAX");
	else if (signum - min <= (max - min) / 2)
		name_off(str, "RTMIN", '+', signum - min);
	else
		name_off(str, "RTMAX", '-', max - signum);
	return 0;
}

/* A decimal number without sign, as str2sig accepts it.  */
/* Read the digits here instead of with strtol: strtol leaves ERANGE in
   errno for a long run of digits, and this never needs more than _NSIG.  */
static int number(const char *s, int *n)
{
	unsigned int v = 0;

	if (*s < '0' || *s > '9')
		return -1;
	do {
		if (v > (unsigned int) _NSIG)
			return -1;
		v = v * 10 + (unsigned int) (*s++ - '0');
	} while (*s >= '0' && *s <= '9');
	if (*s || v > (unsigned int) _NSIG)
		return -1;
	*n = (int) v;
	return 0;
}

int str2sig(const char *__restrict str, int *__restrict pnum)
{
	int min = SIGRTMIN, max = SIGRTMAX;
	size_t i;
	int n;

	for (i = 0; i < sizeof(sigtab) / sizeof(sigtab[0]); i++)
		if (!strcmp(str, sigtab[i].name)) {
			*pnum = sigtab[i].num;
			return 0;
		}
	if (!strncmp(str, "RTMIN", 5) || !strncmp(str, "RTMAX", 5)) {
		if (min < 1 || max < min)	/* no realtime range */
			return -1;
		n = 0;
		if (str[5] && ((str[5] != (str[4] == 'N' ? '+' : '-'))
			       || number(str + 6, &n) || n > max - min))
			return -1;
		*pnum = str[4] == 'N' ? min + n : max - n;
		return 0;
	}
	if (number(str, &n))
		return -1;
	/* Take exactly the numbers sig2str can name: the ones in the table,
	   and the realtime range.  Anything else is unassigned on this port
	   -- hppa, say, starts its realtime signals at 37 and leaves 32..36
	   with no name at all.  */
	for (i = 0; i < sizeof(sigtab) / sizeof(sigtab[0]); i++)
		if (sigtab[i].num == n) {
			*pnum = n;
			return 0;
		}
	if (min < 1 || max < min || n < min || n > max)
		return -1;
	*pnum = n;
	return 0;
}
