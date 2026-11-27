#include <errno.h>
#include <sys/msg.h>
#include <stddef.h>
#include "ipc.h"
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include "sysdep-cancel.h"
#else
#define SINGLE_THREAD_P 1
#endif

#ifdef L_msgctl

#ifdef __NR_msgctl
#define __NR___libc_msgctl __NR_msgctl
static __inline__ _syscall3(int, __libc_msgctl, int, msqid, int, cmd, struct msqid_ds *, buf)
#endif
/* Message queue control operation.  */
int msgctl(int msqid, int cmd, struct msqid_ds *buf)
{
	struct msqid_ds set;
	int __ret;

	if (cmd == IPC_SET && buf != NULL && __IPC_MODE_TO_KERNEL_CHANGES) {
		set = *buf;
		__ipc_mode_to_kernel(&set.msg_perm.mode);
		buf = &set;
	}
#ifdef __NR_msgctl
	__ret = __libc_msgctl(msqid, cmd | __IPC_64, buf);
#else
	__ret = __syscall_ipc(IPCOP_msgctl, msqid, cmd | __IPC_64, 0, buf, 0);
#endif
	/* Only IPC_STAT, MSG_STAT and MSG_STAT_ANY fill a struct msqid_ds;
	   IPC_SET reads it and IPC_INFO hands over a struct msginfo.  A
	   failed call leaves the buffer alone.  */
	if (__ret == -1 || buf == NULL
	    || (cmd != IPC_STAT && cmd != MSG_STAT && cmd != MSG_STAT_ANY))
		return __ret;
#if (__WORDSIZE == 32) && defined(__UCLIBC_USE_TIME64__) && (defined(__mips) || defined(__MSQID_DS_TIME64_SPLIT)) && defined(__NR_msgctl)
	buf->msg_stime = (__time_t)buf->msg_stime_internal_1 | (__time_t)(buf->msg_stime_internal_2) << 32;
	buf->msg_rtime = (__time_t)buf->msg_rtime_internal_1 | (__time_t)(buf->msg_rtime_internal_2) << 32;
	buf->msg_ctime = (__time_t)buf->msg_ctime_internal_1 | (__time_t)(buf->msg_ctime_internal_2) << 32;
#endif
	__ipc_mode_from_kernel(&buf->msg_perm.mode);
	return __ret;
}
#endif


#ifdef L_msgget
#ifdef __NR_msgget
_syscall2(int, msgget, key_t, key, int, msgflg)
#else
/* Get messages queue.  */
int msgget (key_t key, int msgflg)
{
    return __syscall_ipc(IPCOP_msgget ,key ,msgflg ,0 ,0, 0);
}
#endif
#endif


struct new_msg_buf{
    struct msgbuf * oldmsg;
    long int r_msgtyp;       /* the fifth arg of __syscall_ipc */
};
/* Receive message from message queue.  */


#ifdef L_msgrcv
#ifdef __NR_msgrcv
#define __NR___syscall_msgrcv __NR_msgrcv
static inline _syscall5(ssize_t, __syscall_msgrcv, int, msqid, void *, msgp,
			size_t, msgsz, long int, msgtyp, int, msgflg)
#endif
static inline ssize_t do_msgrcv (int msqid, void *msgp, size_t msgsz,
			    long int msgtyp, int msgflg)
{
#ifdef __NR_msgrcv
    return __syscall_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
#else
# if defined __sparc__ && defined __arch64__
    /* sys_sparc_ipc() passes ptr and fifth straight to sys_msgrcv(), where the
       generic sys_ipc() expects the ipc_kludge struct below and reads msgp and
       msgtyp out of it.  Handing it &temp there makes the kernel deliver the
       message into that 16-byte struct instead of the caller's buffer.  */
    return __syscall_ipc(IPCOP_msgrcv, msqid, msgsz, msgflg, msgp,
			 (void *) msgtyp);
# else
    struct new_msg_buf temp;

    temp.r_msgtyp = msgtyp;
    temp.oldmsg = msgp;
    return __syscall_ipc(IPCOP_msgrcv ,msqid ,msgsz ,msgflg ,&temp, 0);
# endif
#endif
}
ssize_t msgrcv (int msqid, void *msgp, size_t msgsz,
	    long int msgtyp, int msgflg)
{
    if (SINGLE_THREAD_P)
	return do_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
    int oldtype = LIBC_CANCEL_ASYNC ();
    int result = do_msgrcv(msqid, msgp, msgsz, msgtyp, msgflg);
    LIBC_CANCEL_RESET (oldtype);
    return result;
#endif
}
#endif



#ifdef L_msgsnd
#ifdef __NR_msgsnd
#define __NR___syscall_msgsnd __NR_msgsnd
static inline _syscall4(int, __syscall_msgsnd, int, msqid, const void *, msgp,
			size_t, msgsz, int, msgflg)
#endif
/* Send message to message queue.  */
static inline int do_msgsnd (int msqid, const void *msgp, size_t msgsz,
			    int msgflg)
{
#ifdef __NR_msgsnd
    return __syscall_msgsnd(msqid, msgp, msgsz, msgflg);
#else
    return __syscall_ipc(IPCOP_msgsnd, msqid, msgsz, msgflg, (void *)msgp, 0);
#endif
}
int msgsnd (int msqid, const void *msgp, size_t msgsz, int msgflg)
{
    if (SINGLE_THREAD_P)
	return do_msgsnd(msqid, msgp, msgsz, msgflg);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
    int oldtype = LIBC_CANCEL_ASYNC ();
    int result = do_msgsnd(msqid, msgp, msgsz, msgflg);
    LIBC_CANCEL_RESET (oldtype);
    return result;
#endif
}
#endif

