/* sem_timedwait is in assembly here (sem_timedwait.S); take only
   sem_clockwait from the C version.  */
#define SEM_CLOCKWAIT_ONLY
#include "../sem_timedwait.c"
