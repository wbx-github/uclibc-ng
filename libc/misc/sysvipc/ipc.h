#ifndef IPC_H
#define IPC_H
#include <syscall.h>
#include <bits/kernel-features.h>
#include <bits/wordsize.h>

#ifndef __ARCH_HAS_DEPRECATED_SYSCALLS__
#  define __IPC_64	0x0
#elif defined __mips__ || defined __m68k__
# if __LINUX_KERNEL_VERSION < 0x050100
#  define __IPC_64      0x100
# else
#  define __IPC_64      0x0
# endif
#elif defined __hppa__
/* parisc has no ARCH_WANT_IPC_PARSE_VERSION: the kernel never strips
   IPC_64, so passing it makes semctl & co. fail with EINVAL.  */
#  define __IPC_64	0x0
#else
# if __WORDSIZE == 32 || defined __alpha__
#  define __IPC_64	0x100
# else
#  define __IPC_64	0x0
# endif
#endif

#ifdef __NR_ipc

/* The actual system call: all functions are multiplexed by this.  */
extern int __syscall_ipc (unsigned int __call, long __first, long __second,
					  long __third, void *__ptr, void *__fifth) attribute_hidden;


/* The codes for the functions to use the multiplexer `__syscall_ipc'.  */
#define IPCOP_semop	 1
#define IPCOP_semget	 2
#define IPCOP_semctl	 3
#define IPCOP_semtimedop 4
#define IPCOP_msgsnd	11
#define IPCOP_msgrcv	12
#define IPCOP_msgget	13
#define IPCOP_msgctl	14
#define IPCOP_shmat	21
#define IPCOP_shmdt	22
#define IPCOP_shmget	23
#define IPCOP_shmctl	24

#endif

#endif							/* IPC_H */
