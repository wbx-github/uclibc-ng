#ifndef _TIME64_HELPERS_H
#define _TIME64_HELPERS_H

#include <linux/version.h>
#include <bits/types.h>
#include <time.h>
#include <stddef.h>

#if defined(__UCLIBC_USE_TIME64__) && __TARGET_ARCH_BITS__ == 32 && LINUX_VERSION_CODE < KERNEL_VERSION(5,1,0)
#error 64bit time on 32bit targets is not supported on Linux < 5.1.0
#endif

struct __ts64_struct {
   __S64_TYPE tv_sec;
   __S64_TYPE tv_nsec;
};

#define TO_TS64_P(__ts) (((struct timespec *)(__ts)) ? \
                        (&(struct __ts64_struct) {.tv_sec = ((struct timespec *)(__ts))->tv_sec, \
                                                  .tv_nsec = ((struct timespec *)(__ts))->tv_nsec}) : NULL)

struct __its64_struct {
    __S64_TYPE interval_tv_sec;
    __S64_TYPE interval_tv_nsec;
    __S64_TYPE value_tv_sec;
    __S64_TYPE value_tv_nsec;
};

#define TO_ITS64_P(__its) (((struct itimerspec *)(__its)) ? \
                          (&(struct __its64_struct) {.interval_tv_sec = ((struct itimerspec *)(__its))->it_interval.tv_sec, \
                                                     .interval_tv_nsec = ((struct itimerspec *)(__its))->it_interval.tv_nsec, \
                                                     .value_tv_sec = ((struct itimerspec *)(__its))->it_value.tv_sec, \
                                                     .value_tv_nsec = ((struct itimerspec *)(__its))->it_value.tv_nsec}) : NULL)

/* struct itimerval as the kernel's getitimer and setitimer use it: there
   is no time64 variant of either call.  */
struct __itimerval32_struct {
    __S32_TYPE __interval_sec;
    __S32_TYPE __interval_usec;
    __S32_TYPE __value_sec;
    __S32_TYPE __value_usec;
};

/* The other way, for what the kernel hands back.  */
static __inline__ void __ts64_to_ts(const struct __ts64_struct *__src, struct timespec *__dst)
{
    __dst->tv_sec = __src->tv_sec;
    __dst->tv_nsec = __src->tv_nsec;
}

static __inline__ void __its64_to_its(const struct __its64_struct *__src, struct itimerspec *__dst)
{
    __dst->it_interval.tv_sec = __src->interval_tv_sec;
    __dst->it_interval.tv_nsec = __src->interval_tv_nsec;
    __dst->it_value.tv_sec = __src->value_tv_sec;
    __dst->it_value.tv_nsec = __src->value_tv_nsec;
}

#endif /* _TIME64_HELPERS_H */
