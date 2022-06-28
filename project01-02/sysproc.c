#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

extern struct {
    struct spinlock lock;
    struct proc proc[NPROC];
} ptable;

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int sys_getppid(void)
{
  struct proc* myproc_copy = myproc();
  return myproc_copy -> parent -> pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;

  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }

#ifdef SCHED_POLICY
#if SCHED_POLICY == 1
  myproc()->level = 0;
  myproc()->usedTime = 0;
#endif
#endif


  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}



extern void yield(void);
void
sys_yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
#ifdef SCHED_POLICY
#if SCHED_POLICY == 1
  myproc()->level = 0;
  myproc()->usedTime = 0;
#endif
#endif
  release(&ptable.lock);
}

#ifdef SCHED_POLICY
#if SCHED_POLICY == 1
int
sys_getlev(void)
{
  return myproc()->level;
}



int
sys_setpriority(void)
{
  int pid;
  int priority;

  if(argint(0, &pid) < 0)
    return -3;

  if(argint(1, &priority) < 0)
    return -3;

  if(priority > 10 || priority < 0){
    return -2;
  }

  for (struct proc* p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
    if (pid == p->pid) {
      if(p->parent->pid == myproc()->pid){
        p->priority = priority;
        return 0;
      } else {
        return -1;
      }
    }
  }

  return -1;
}
#else
int
sys_getlev(void) {
  return -1;
}

int
sys_setpriority(void)
{
  return -1;
}
#endif
#else
int
sys_getlev(void) {
  return -1;
}

int
sys_setpriority(void)
{
  return -1;
}
#endif


