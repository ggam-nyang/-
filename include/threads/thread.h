#ifndef THREADS_THREAD_H
#define THREADS_THREAD_H

#include <debug.h>
#include <list.h>
#include <stdint.h>
#include "threads/interrupt.h"
#include "threads/synch.h"
#ifdef VM
#include "vm/vm.h"
#endif

#define NICE_DEFAULT 0
#define RECENT_CPU_DEFAULT 0
#define LOAD_AVG_DEFAULT 0


/* States in a thread's life cycle. */
enum thread_status {
	THREAD_RUNNING,     /* Running thread. */
	THREAD_READY,       /* Not running but ready to run. */
	THREAD_BLOCKED,     /* Waiting for an event to trigger. */
	THREAD_DYING        /* About to be destroyed. */
};

/* Thread identifier type.
   You can redefine this to whatever type you like. */
typedef int tid_t;
#define TID_ERROR ((tid_t) -1)          /* Error value for tid_t. */

/* Thread priorities. */
#define PRI_MIN 0                       /* Lowest priority. */
#define PRI_DEFAULT 31                  /* Default priority. */
#define PRI_MAX 63                      /* Highest priority. */

#define NICE_DEFAULT 0					/* Default nice */
#define RECENT_CPU_DEFAULT 0			/* Default recent_cput */
#define LOAD_AVG_DEFAULT 0				/* Deafult load_avg */

/* A kernel thread or user process.
 *
 * Each thread structure is stored in its own 4 kB page.  The
 * thread structure itself sits at the very bottom of the page
 * (at offset 0).  The rest of the page is reserved for the
 * thread's kernel stack, which grows downward from the top of
 * the page (at offset 4 kB).  Here's an illustration:
 *
 *      4 kB +---------------------------------+
 *           |          kernel stack           |
 *           |                |                |
 *           |                |                |
 *           |                V                |
 *           |         grows downward          |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           |                                 |
 *           +---------------------------------+
 *           |              magic              |
 *           |            intr_frame           |
 *           |                :                |
 *           |                :                |
 *           |               name              |
 *           |              status             |
 *      0 kB +---------------------------------+
 *
 * The upshot of this is twofold:
 *
 *    1. First, `struct thread' must not be allowed to grow too
 *       big.  If it does, then there will not be enough room for
 *       the kernel stack.  Our base `struct thread' is only a
 *       few bytes in size.  It probably should stay well under 1
 *       kB.
 *
 *    2. Second, kernel stacks must not be allowed to grow too
 *       large.  If a stack overflows, it will corrupt the thread
 *       state.  Thus, kernel functions should not allocate large
 *       structures or arrays as non-static local variables.  Use
 *       dynamic allocation with malloc() or palloc_get_page()
 *       instead.
 *
 * The first symptom of either of these problems will probably be
 * an assertion failure in thread_current(), which checks that
 * the `magic' member of the running thread's `struct thread' is
 * set to THREAD_MAGIC.  Stack overflow will normally change this
 * value, triggering the assertion. */
/* The `elem' member has a dual purpose.  It can be an element in
 * the run queue (thread.c), or it can be an element in a
 * semaphore wait list (synch.c).  It can be used these two ways
 * only because they are mutually exclusive: only a thread in the
 * ready state is on the run queue, whereas only a thread in the
 * blocked state is on a semaphore wait list. */
struct thread {
	/* Owned by thread.c. */
	tid_t tid;                          /* Thread identifier. */
	enum thread_status status;          /* Thread state. */
	char name[16];                      /* Name (for debugging purposes). */
	int priority;                       /* Priority. */
	int ori_priority;                   /* Original priority */
	int64_t awake_time;

	int nice;
    int recent_cpu;

	/* Shared between thread.c and synch.c. */
	struct list_elem elem;              /* List element. */
	struct lock *wish_lock;             /* Have lock list */
	struct list donations;              /* Donation list*/
	struct list_elem donation_elem;     /* Donation element list*/
	struct list_elem allelem;           /* List element for all threads list. */

	/* Project2 */
	struct list child_list;
	struct list_elem child_elem;
	// wait syscall
	struct semaphore wait_sema;  	// child를 기다리기 위해 parent가 사용
	int exit_status;			 	// child의 exit_stauts를 부모에게 전달하기 위해
	// fork syscall
	struct intr_frame parent_if; 	// 내 intr_frame을 보관하고, child에게 주기 위함
	struct semaphore fork_sema;  	// 자식이 끝날 때까지, 부모 process가 기다려줌  (__do fork)
	struct semaphore free_sema;  	// 자식의 termination을 연기함! 부모가 exit_status를 받을 때 까지!! (process_wait)
	// file descripter
	struct file **fdTable;			/* *****주석을 달아주세요****** */
	int fdIdx;
	// deny exec writes
	struct file *running;
	// extra!! count open stdin/stdout
	// dup2 may copy stdin / stdout, 
	int stdin_count;
	int stdout_count;

	


	int nice;							/* nice */
	int recent_cpu;						/* recent_cpu for thread */
	struct list_elem all_elem;			/* all_list elem */
#ifdef USERPROG
	/* Owned by userprog/process.c. */
	uint64_t *pml4;                     /* Page map level 4 */
#endif
#ifdef VM
	/* Table for whole virtual memory owned by thread. */
	struct supplemental_page_table spt;
#endif

	/* Owned by thread.c. */
	struct intr_frame tf;               /* Information for switching */
	unsigned magic;                     /* Detects stack overflow. */
};

/* If false (default), use round-robin scheduler.
   If true, use multi-level feedback queue scheduler.
   Controlled by kernel command-line option "-o mlfqs". */
extern bool thread_mlfqs;

void thread_init (void);
void thread_start (void);

void thread_sleep(int64_t);
void thread_awake(int64_t);
bool thread_compare_awake(const struct list_elem *,
						  const struct list_elem *,
						  void *);

int64_t get_next_tick_to_awake(void);
void update_next_tick_to_awake(int64_t);
int64_t first_next_tick_to_awake(void);

void thread_tick (void);
void thread_print_stats (void);

typedef void thread_func (void *aux);
tid_t thread_create (const char *name, int priority, thread_func *, void *);

void thread_block (void);
void thread_unblock (struct thread *);

struct thread *thread_current (void);
tid_t thread_tid (void);
const char *thread_name (void);

void thread_exit (void) NO_RETURN;
void thread_yield (void);

int thread_get_priority (void);
void thread_set_priority (int);

int thread_get_nice (void);
void thread_set_nice (int);
int thread_get_recent_cpu (void);
int thread_get_load_avg (void);

void do_iret (struct intr_frame *tf);

bool thread_compare_priority (const struct list_elem *a_, const struct list_elem *b_, void *aux UNUSED); 

bool sema_compare_priority (const struct list_elem *a_, const struct list_elem *b_, void *aux UNUSED); 

bool thread_compare_donate_priority (const struct list_elem *a_, const struct list_elem *b_, void *aux UNUSED); 

void thread_preemption(void); 
void refresh_priority(void);

void mlfqs_calculate_priority (struct thread *);
void mlfqs_calculate_recent_cpu (struct thread *);
void mlfqs_calculate_load_avg (void);
void mlfqs_increment_recent_cpu (void);
void mlfqs_recalculate_recent_cpu (void);
void mlfqs_recalculate_priority (void);

#endif /* threads/thread.h */