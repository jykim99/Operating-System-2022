#include "types.h"
#include "defs.h"
#include "param.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"
#include "schedule.h"

#ifdef SCHED_POLICY
#if SCHED_POLICY == 0
// RR queue
Queue roundRobinQueue;
// FCFS queue
Queue fcfsQueue;

extern struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

int is_empty(Queue *q) {
  if (q->front == q->rear) return 1;
  else return 0;
}

int is_full(Queue *q) {
  if (((q->rear + 1) % MAX_SIZE) == q->front) return 1;
  else return 0;
}

void initQueue(Queue *q) {
  q->front = 0;
  q->rear = 0;
  q->length = 0;
}

void enqueue(Queue *q, procToSchedule pts) {
  if (is_full(q)) {
    panic("full queue");
    return;
  }
  else {
    q->rear = (q->rear + 1) % (MAX_SIZE);
    q->data[q->rear] = pts;
    q->length = q->length + 1;
  }
//  cprintf("ENQUEUE : pid %d, plist index %d, queue length : %d, queue front : %d, queue rear: %d\n", pts.originPid, pts.plistIndex, q->length, q->front, q->rear);
//  cprintf("%d, %d\n", pts.originPid, pts.plistIndex);
  return;
}

procToSchedule* dequeue(Queue *q) {
  if (is_empty(q)) {
    if (q == &roundRobinQueue){
      cprintf("RRQ\n");
    } else if (q == &fcfsQueue){
      cprintf("FCFSQ\n");
    }
    panic("empty queue");
  }
  q->front = (q->front + 1) % (MAX_SIZE);
  q->length = q->length - 1;
  if (q == &fcfsQueue){
      cprintf("FCFSQ dequeue pid : %d\n", q->data[q->front].originPid);
    }
//  cprintf("dequeue pid : %d\n", q->data[q->front].originPid);
  return &(q->data[q->front]);
}

int isValidPts(procToSchedule pts){
  // process was killed
  if ((&ptable.proc[pts.plistIndex])->killed == 1 || pts.originPid != (&ptable.proc[pts.plistIndex])->pid || (&ptable.proc[pts.plistIndex])->state == ZOMBIE) {
    if((&ptable.proc[pts.plistIndex])->killed == 1){
    //  cprintf("isValidPts: %d was killed\n");
    } else if (pts.originPid != (&ptable.proc[pts.plistIndex])->pid){
    //  cprintf("isValidPts: origin pid %d was overriden with %d\n", pts.originPid, (&ptable.proc[pts.plistIndex])->pid);
    } else if ((&ptable.proc[pts.plistIndex])->state == ZOMBIE){
      // cprintf("isValidPts ZOMBIE %d\n", pts.originPid);
    }

    if((&ptable.proc[pts.plistIndex])->pid % 2 == 1){
      // cprintf("isValidPts pid %d\n", (&ptable.proc[pts.plistIndex])->pid);
    }
    return 0;
  } else {
    return 1;
  }
}

// MultilevelQueue
void addScheduleList(struct proc* p, int index) {
  procToSchedule pts = {.originPid = p->pid, .plistIndex = index};
  if(p->pid % 2 == 0){
    enqueue(&roundRobinQueue, pts);
//    cprintf("pid %d was added to roundRobinQueue\n", p->pid);
  } else { //FCFS
    enqueue(&fcfsQueue, pts);
//    cprintf("pid %d was added to fcfsQueue\n", p->pid);
  }
}

procToSchedule* getNth(Queue *q, int n) {
//  cprintf("%d\n",q->data[(q->front + n + 1) % (MAX_SIZE)].originPid);
  return &(q->data[(q->front + n + 1) % (MAX_SIZE)]);
}

void removeNth(Queue *q, int n){
  if(is_empty(q)){
    panic("removeNth on empty list");
  }

  if(q->length <= n){
    panic("removeNth q->length <= n");
  }

  int cnt = (q->front + n + 1) % (MAX_SIZE);
  int swapNum = q->length - n + 1;

  for (int i = 0; i < swapNum; i++){
    int nextIndex = (cnt + 1) % MAX_SIZE;
    q->data[cnt] = q->data[nextIndex];
    cnt = nextIndex;
  }

  q->length--;
  q->rear = (q->rear + MAX_SIZE - 1) % MAX_SIZE;
}


#elif SCHED_POLICY == 1

#endif

#endif

