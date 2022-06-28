#define MAX_SIZE 1000



#ifdef SCHED_POLICY
#if SCHED_POLICY == 0 //MULTILEVEL_SCHED
typedef struct pts {
    int originPid;
    int plistIndex;
} procToSchedule;

typedef struct circleQueue {
    int rear;
    int front;
    int length;
    procToSchedule data[MAX_SIZE];
} Queue;

procToSchedule* getNth(Queue *q, int n);

void removeNth(Queue *q, int n);

void addScheduleList(struct proc* p, int index);
void removeScheduleList(struct proc* p);

void initQueue(Queue *q);

void enqueue(Queue *q, procToSchedule pts);

procToSchedule* dequeue(Queue *q);

int is_empty(Queue *q);

int is_full(Queue *q);

int isValidPts(procToSchedule pts);

#elif SCHED_POLICY == 1
#endif

#endif



