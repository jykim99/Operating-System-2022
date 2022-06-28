## Project 01-1

- ### Design (xv6내에서 특정 시스템 콜을 구현하는 방식에 대하여...)
  1. 구현할 로직을 담은 함수를 커널 수준에서 실행되도록 변환되는 소스파일에 추가합니다.
     - _이때 새로운 파일을 생성하게 된다면, 이 파일을 **Makefile** 의 **OBJS** 매크로에 추가하여야 합니다.  
       (유저용 프로그램을 나열하는 **UPROGS** 매크로와 구분해야합니다.)_
  2. 해당 함수를 헤더파일에 등록합니다.
  3. 앞서 만든 로직의 wrapper function을 제작합니다. 이 함수는 로직을 실행하기 전에 전처리를 해주는데, 이 중에는 레지스터에서 매개변수를 받아 넘겨주는 작업이 포함됩니다.
  4. **syscall.h** 파일 내에 wrapper function에 고유한 int값을 설정해 정의합니다.
  5. **syscall.c** 파일 내에서 **extern** 키워드를 통해 앞서 구현한 wrapper function을 참조합니다. 참조한 함수를 **syscall.h** 에서 정의한 고유한 int값에 매칭시켜줍니다. **syscall** 함수는 시스템 콜 interrupt가 발생했을 때, **eax** 레지스터에 저장된 숫자값을 읽어 그 숫자값에 매칭되는 시스템 콜의 참조 값을 **eax** 레지스터에 저장합니다.
  6. **user.h** 파일 내에 시스템 콜을 등록해줍니다. 이를 통해 유저모드에서 실행하는 프로세스들이 새로 구현한 시스템 콜에 접근할 수 있습니다.
  7. **usys.S** 파일에 새로 구현한 시스템 콜을 추가합니다. 이 파일 내 구현되어 있는 **SYSCALL** 매크로는 시스템 콜의 이름을 받아 이름에 매칭되는 고유 int값(시스템콜 테이블 index값이 되는 값)을 **eax** 레지스터에 담고 interrupt를 발생시킵니다.
  8. 새로 만든 시스템 콜의 동작을 테스트 하기위해서는 다음 과정을 수행합니다.
     - 테스트 용 유저프로그램을 구현할 파일을 xv6 디렉토리 내에 생성하고, 시스템 콜 호출을 포함하는 main 함수를 작성합니다.
     - **Makefile** 에 생성한 파일을 추가해줍니다. ( **UPROGS** 타겟 및 **EXTRA** 타겟에 지정해야 합니다.)
- ### Preview
  - getpid()
    - **myproc()** 를 호출하여 프로세스 구조를 받아와 이를 통해 프로세스 id에 접근하여 반환한다. 이때 프로세스 구조는 **def.h** 파일에 **proc** 이라는 구조체로 선언되어 있다. **_getppid()도 이 구조를 이용해 구현할 수 있을 것이다!_**
- ### Implement
  1. **sysproc.c** 파일 내에 **sys_getppid()** 함수를 다음과 같이 구현했다.
     <pre><code>
     int sys_getppid(void) {
         struct proc* myproc_copy = myproc();
         return myproc_copy -> parent -> pid;
     }  
     </code></pre>
     **myproc()** 를 호출하여 현재 실행중인 프로세스 정보를 담은 구조체 포인터 값을 받아오고 바로 이전의 포인터 값을 찾는다. (프로세스 정보는 메모리 상에 스택 구조로 저장되어 있기 때문이다.) 이렇게 얻은 포인터 값은 부모 프로세스를 가리키고 있으므로 pid에 접근하여 return하면 부모 프로세스의 pid를 내보낼 수 있다.
  2. **syscall.h** 파일 내에 SYS_getppid 값을 정의해준다. 이를 통해 **usys.S** 에 정의된 **SYSCALL** 매크로에서 system call의 이름을 통해 해당 번호를 eax레지스터에 저장 후 interrupt를 발생시킬 수 있다.
  <pre><code>#define SYS_getppid 23</code></pre>
  3. **syscall.c** 파일에서 **extern** 키워드를 통해 **sys_getppid** 함수를 불러온다.
  <pre><code> extern int sys_getppid(void); </code></pre>
  4. **syscall.c** 파일 내 시스템 콜 포인터 배열에 **sys_getppid**함수를 등록한다.
  <pre><code> [SYS_getppid] sys_getppid, </code></pre>
  5. **user.h** 파일에 **getppid** 함수를 추가해줍니다.
  <pre><code> int getppid(void); </code></pre>
  6. **usys.S** 파일에 매크로를 활용한 코드를 삽입한다.
  <pre><code> SYSCALL(getppid) </code></pre>
  7. 유저가 사용할 수 있는 **project01** 프로그램을 만들기 위해 **project01.c** 파일을 생성해주고, 다음과 같이 main 함수를 작성한다. 이때 **types.h** 와 **user.h** 를 include 해야한다.
  <pre><code>
      int main(int argc, char* argv[]){
          int pid = getpid();
          printf(1, "My pid is %d\n", pid);
          int ppid = getppid();
          printf(1, "My ppid is %d\n", ppid);
          exit();
      }
  </code></pre>
  8. **Makefile** 에서 **project01** 를 인식할 수 있도록 추가한다.
- ### Result
    <pre><code>
        Booting from Hard Disk..xv6...
        cpu0: starting 0
        sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
        init: starting sh
        $ project01
        My pid is 3
        My ppid is 2
    </code></pre>
- ### Troble shooting
  - 부모 프로세스 구조체 포인터 접근할 때 myproc() - myproc() -> sz를 했더니 trap이 발생함.
    - myproc() - 1 로 연산하여 해결함.
  - **project01.c** 파일 **make** 과정에서 인식할 수 없는 타입이라는 에러가 나옴.
    - **types.h** 파일 include하여 해결함.
  - printf가 c 라이브러리의 동일 이름 함수와 호출방법이 상이함을 반영하지 않음.
    - 첫번째 인수로 1을 추가하여 해결함.

## Project 01-2

- ### Makefile 스터디
  각 타겟들에 어떠한 파일들이 연관되어 있는지 확인해보며 각 파일들이 이 os에서 동작하는 위치를 파악할 수 있었다.
  - **make** 명령어를 타겟 지정없이 실행한다면, 가장 앞에 정의된 타겟을 실행한다.
  - xv6의 **Makefile** 에는 **xv6.img** 타겟이 가장 앞에 정의되어있다. 이 타겟은 **bootblock** 타겟과 **kernel** 타겟을 실행한다.
  - 이 타겟들은 유저프로그램을 빌드하지 않기 때문에, 만약 유저프로그램을 빌드하고 싶다면 **fs.img** 타겟을 직접 실행해야한다.
- ### System Call 둘러보기
  이번 주에 주로 공부했던 시스템 콜에 대한 코드들을 분석해보면, xv6의 동작에 대해 더 자세히 알 수 있을 것이라고 판단했다.
  - **sysproc.c**
    - 프로세스 관리와 관련된 시스템 콜들이 위치해 있는 소스 파일이다.
    - 대표적으로 **fork(), exit(), kill(), getpid(), sleep()** 등을 찾아볼 수 있었다.
    - **sbrk()** 는 프로세스에 새로운 메모리 영역을 지정하기 위해 사용한다.
    - **uptime()** 는 시스템이 언제 시작되었는지 알고 싶을 때 사용한다.
  - **sysfile.c**
    - 파일 시스템에 관련된 시스템 콜들이 위치해 있는 소스 파일이다.
    - 대표적으로 **read(), write(), close(), create(), mkdir()** 등을 찾아볼 수 있었다.
    - **fstat()** 이 뜻하는 바를 몰라 서치해보니, 파일의 상태를 얻는 시스템 콜이라는 것을 알 수 있었다.
    - **chdir()** 은 현재 디렉토리를 변경하는 시스템 콜임을 알 수 있었다.
    - **dup()** 은 새로운 파일 디스크립터를 반환할 때 사용한다는 것을 알게 되었다.
      - **파일 디스크립터** 가 무엇인지 몰라 검색해보니, **프로세스가 파일을 다룰 때 사용하는 개념으로, 프로세스에서 특정 파일에 접근할 때 사용하는 추상적인 값** 이라는 것을 알 수 있었다.
    - **pipe()** 는 프로세스간 단방향 통신을 하기 위해 데이터 채널을 만든다.
      - pipefd 배열에 각각 파일 디스크립터가 리턴된다.
    - **link()** 는 파일에 대해 새 이름을 지정하기 위해 사용한다.
- ### 자주 보이는 헤더 파일들 탐구해보기
  - ### **types.h** 파일에 대해서..
    기존 구현되어 있는 로직들은 이 헤더파일에 typedef로 정의된 타입을 활용하고 있다. 그러므로 아래 코드에 정의된 타입을 사용하기 위해서는 반드시 소스파일에서 include 해주어야 한다.
      <pre></code>
          typedef unsigned int   uint;
          typedef unsigned short ushort;
          typedef unsigned char  uchar;
          typedef uint pde_t;
      </code></pre>
  - ### **defs.h** 파일에 대해서...
    os에서 제공하는 시스템콜들이 선언되어 모여있는 헤더파일이다. 이 파일은 user에게 visible하지는 않고 내부 시스템에서 사용하는 것 같다.
  - ### **user.h** 파일에 대해서...
    유저 프로그램에게도 visible한 로직들이 선언되어 있는 헤더파일이다. 각종 시스템콜들이 선언되어 있으며, **ulib.c** 에 정의된 라이브러리 함수들도 이 헤더파일을 통해 제공된다.
  - ### **bio.h** 파일에 대해서...
    버퍼 캐시에 대한 내용이 담겨져있다.
    - **bread** => 특정 디스크 블록에 대한 버퍼를 가져온다.
    - **bwrite** => 버퍼 데이터 변경 후 디스크에 쓴다.
    - **breelse** => 버퍼 처리 완료 후 호출한다.
    - 한 번에 하나의 프로세스만 버퍼를 사용할 수 있다.
    - 내부적으로 두 가지 상태 플래그를 사용한다.
      - B_VALID: 디스크에서 버퍼 데이터를 읽었습니다.
      - B_DISTY : 버퍼 데이터가 수정되었습니다. 그러므로 데이터를 디스크에 써야합니다.
  - ### **file.h** 파일에 대해서...
    파일에 대한 구조체가 선언되어 있는 파일이다.
  - ### **trap.h** 파일에 대해서...
    trap과 interrupt 종류에 대해 명시 되어있는 헤더파일이다.
  - ### **mmu.h** 파일에 대해서...
    **MMU(Memory Management Unit)** 에 관한 내용이 정의되어 있는 헤더 파일이다. 사실 MMU 즉 **메모리 관리 장치** 에 대한 개념이 부족하여 이 부분을 먼저 알아보았다. 메모리 관리 장치는 CPU가 메모리에 접근하는 것을 관리하는 컴퓨터 하드웨어 부품이다. 가상 메모리 주소를 실제 메모리 주소로 변환하며, 메모리 보호, 캐시 관리, 버스 중재 등의 역할을 담당한다. 이 파일 내에서 이론 시간에 배운 적이 있는 반가운 내용들도 보였다.
    - Interrupt 발생을 허용할지 설정하는 **FL_IF** 가 보였다.
    - 이번 실습 슬라이드에 등장한 **SEG_KCODE** 와 **SEG_UCODE** 가 보였다.

---

## Project02

### Multilevel Queue

- ### Design
  - 스케쥴링 정보를 담을 자료구조를 생성 후 이와 관련된 로직을 새로운 파일로 (**schedule.h, schedule.c**) 분리하였다.
    - 스케쥴링 정보를 가질 자료구조는 **원형 큐**로 정했다.
    - Round Robin Queue와 FCFS Queue 모두 원형 큐를 사용했다.
    - **initQueue**는 큐를 초기화 한다. (부팅 시 **pinit**에서 호출한다.)
    - 원형 큐는 기본적인 큐의 동작 (enqueue, dequeue)을 포함해 특정 순서에 있는 원소를 제어하는 동작을 추가로 가지고 있다. (getNth, removeNth)
    - 원형 큐는 **procToSchedule** 구조체 값을 저장한다.
      - **procToSchedule**는 **plistIndex**와 **originPid**를 가지고 있다.
      - **plistIndex**는 **ptable**의 배열에 우리가 스케쥴링 할 프로세스가 저장되어 있는 index값을 나타낸다.
      - **originPid**은 프로세스의 pid를 나타낸다.
      - 추후 이 pid를 스케쥴링하기 이전에 **plistIndex**값에 해당하는 프로세스가 kill된 후 다른 프로세스로 덮어씌워진 상황을 감지하기 위해 필요하다.
    - **_void addScheduleList(struct proc p, int index)_** 는 프로세스 포인터 값과 **ptable** **proc**의 index값을 받아 **procToSchedule**로 포장해 스케쥴링 큐에 추가한다.
      - pid값에 따라 Round Robin Queue 또는 FCFS Queue에 추가한다.
    - **_int isValidPts(procToSchedule pts)_** 는 인자로 받은 **procToSchedule**가 아직 스케쥴링에 유효한지 확인한다. 유효하지 않다고 판단하는 경우는 아래와 같다.
      - 프로세스가 kill 되었을 때
      - 프로세스가 zombie 상태일 때
      - pts.originPid != (&ptable.proc[pts.plistIndex])->pid 일 때
  - **time interrupt** 발생 시 처리 방식을 수정하였다.
    - 현재 실행 중인 프로세스의 pid가 짝수면 Round Robin 방식으로 스케쥴링 하기 위해 **yield**를 호출한다.
    - 현재 실행 중인 프로세스의 pid가 홀수면 따로 처리하지 않는다.
  - **allocproc** 에서 프로세스 생성 후 **addScheduleList**를 호출한다.
  - **scheduler** 에서는 Round Robin Queue와 FCFS Queue를 차례로 실행 가능한 프로세스가 있는지 확인한다.
    1. Round Robin Queue
       - Round Robin Queue의 length만큼 dequeue하며 순회한다.
         - 만약 RUNNABLE한 프로세스가 있어 스케쥴링을 해준다면 endFlag를 true로 설정해 다음 스케쥴링은 처음부터 다시 순회하도록 한다. (roundRobinLen의 값이 초기화 된다.)
         - 만약 순회하는 동안 RUNNABLE한 프로세스를 발견하지 못한다면 FCFS Queue를 검사한다.
       - Round Robin에서는 매번 dequeue후 enqueue를 하여 프로세스를 뒤로 보낸다.
         - dequeue한 procToSchedule의 isValidPts가 false면 enqueue하지 않는다.
    2. FCFS Queue
       - FCFS Queue의 length만큼 dequeue하며 순회한다.
         - 만약 RUNNABLE한 프로세스가 있어 스케쥴링을 해준다면 endFlag를 true로 설정해 다음 스케쥴링은 처음부터 다시 순회하도록 한다. (ROUND ROBIN 부터 다시 확인한다.)
         - 만약 순회하는 동안 RUNNABLE한 프로세스를 발견하지 못한다면 다시 ROUND ROBIN 부터 확인한다.
       - getNth통해 얻은 pts의 isValidPts가 false일 때 removeNth를 통해 스케쥴링 리스트에서 삭제한다.
- ### Implement

  - **schedule.h** 에서는 아래와 같은 구조체와 함수를 선언하며 이에 대한 상세구현은 **schedule.c**에 위치한다.
      <pre><code>
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
        void addScheduleList(struct proc* p, int index);
        void initQueue(Queue *q);
        void enqueue(Queue *q, procToSchedule pts);
        procToSchedule* dequeue(Queue *q);
        procToSchedule* getNth(Queue *q, int n);
        void removeNth(Queue *q, int n);
        int is_empty(Queue *q);
        int is_full(Queue *q);
        int isValidPts(procToSchedule pts);
      </code></pre>
  - **trap.c**에서 time interupt 발생 시 처리를 다음과 같이 변경했다.
      <pre><code>
        if(myproc() && myproc()->state == RUNNING &&
              tf->trapno == T_IRQ0+IRQ_TIMER) {
          if(myproc()->pid % 2 == 0) { //짝수
              yield();
        }
      </code></pre>
  - 부팅 시 호출하는 **pinit()**에서 Round Robin Queue와 FCFS Queue를 초기화해준다.
      <pre><code>
        initQueue(&roundRobinQueue);
        initQueue(&fcfsQueue);
      </code></pre>
  - **allocproc()** 에서 **addScheduleList()**를 호출하여 스케쥴링에 추가한다.
      <pre><code>
        void addScheduleList(struct proc* p, int index) {
          procToSchedule pts = {.originPid = p->pid, .plistIndex = index};
          if(p->pid % 2 == 0){
            enqueue(&roundRobinQueue, pts);
          } else { //FCFS
            enqueue(&fcfsQueue, pts);
          }
        }
      </code></pre>
  - **scheduler()** 함수 내 반복문을 아래와 같이 수정했다.
      <pre><code>
          for (;;) {
    
            int endFlag = 0;
    
            // Enable interrupts on this processor.
            sti();
    
            acquire(&ptable.lock);
    
            // Check RR Queue
            int roundRobinLen = roundRobinQueue.length;
            int fcfsLen = fcfsQueue.length;
    
            for (int i = 0; i < roundRobinLen; i++) {
              if(endFlag){
                break;
              }
    
              procToSchedule* PTS = dequeue(&roundRobinQueue);
    
              if(isValidPts(*PTS)) {
    
                // next scheduling add
                enqueue(&roundRobinQueue, *PTS);
    
                p = &ptable.proc[PTS->plistIndex];
    
                if(p->state != RUNNABLE) {
                  continue;
                }
    
                // Switch to chosen process.  It is the process's job
                // to release ptable.lock and then reacquire it
                // before jumping back to us.
                c->proc = p;
                switchuvm(p);
                p->state = RUNNING;
                endFlag = 1;
    
                swtch(&(c->scheduler), p->context);
                switchkvm();
    
                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
    
              } else {
                // todo
              }
            }
    
            if(endFlag){
              release(&ptable.lock);
              continue;
            }
    
            int dequedFCFS = 0;
            fcfsLen = fcfsQueue.length;
    
            // CheckFCFS
            for (int i = 0; i < fcfsLen; i++) {
              if(endFlag){
                break;
              }
    
              procToSchedule* PTS = getNth(&fcfsQueue, i - dequedFCFS);
    
              if (isValidPts(*PTS)) {
                p = &ptable.proc[PTS->plistIndex];
                if(p->state != RUNNABLE) {
                  continue;
                }
    
                endFlag = 1;
    
                // Switch to chosen process.  It is the process's job
                // to release ptable.lock and then reacquire it
                // before jumping back to us.
                c->proc = p;
                switchuvm(p);
                p->state = RUNNING;
    
                swtch(&(c->scheduler), p->context);
                switchkvm();
    
                // Process is done running for now.
                // It should have changed its p->state before coming back.
                c->proc = 0;
    
              } else {
                removeNth(&fcfsQueue, i - dequedFCFS);
                dequedFCFS++;
              }
            }
            release(&ptable.lock);
          }
      </code></pre>

  - 이 떄 사용되는 **isValidPTS()** 는 아래와 같이 구현되어있다.
    <pre><code>
      int isValidPts(procToSchedule pts){
        if ((&ptable.proc[pts.plistIndex])->killed == 1 || pts.originPid != (&ptable.proc[pts.plistIndex])->pid || (&ptable.proc[pts.plistIndex])->state == ZOMBIE) {
          return 0;
        } else {
          return 1;
        }
      }
    </code></pre>

- ### Result

  - 테스트용으로 제공된 ml_test를 실행한 결과는 아래와 같다.
    <pre>
      Booting from Hard Disk..xv6...
    cpu0: starting 0
    sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
    init: starting sh
    $ ml_test
    Multilevel test start
    [Test 1] without yield / sleep
    Process 4
    Process 4
    PrProcess 6
    Process 6
    Proocess 4
    Process 4
    Processcess 6
    Process 6
    Process 4
    Process 4
    Process 6
    Process 6
    Proce 4
    Process 4
    Process 6
    Process 6
    Process 4
    Process 4
    Procss 6
    Process 6
    Proess 4
    Process 4
    Proccess 6
    Process 6
    Proess 4
    Process 4
    Proccess 6
    Process 6
    Press 4
    Process 4
    Prococess 6
    Process 6
    ess 4
    Process 4
    PrProcess 6
    Process 6
    ocess 4
    Process 4
    Process 6
    Process 6
    Process 4
    Process 4Process 6
    Process 6
    
    Process 4
    ProceProcess 6
    Process 6
    ss 4
    Process 4
    ProProcess 6
    Process 6
    cess 4
    Process 4
    Process 6
    Process 6
    Process 4
    Process 4
    Process 6
    Process Process 4
    Process 4
    6
    Process 6
    ProceProcess 4
    Process 4
    ss 6
    Process 6
    ProProcess 4
    Process 4cess 6
    Process 6
    Pro
    Process 4
    Process 4cess 6
    Process 6
    Process 6
    Process 6
    Pr
    Process 4
    Processocess 6
    4
    Process 4
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 5
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    Process 7
    [Test 1] finished
    [Test 2] with yield
    Process 10 finished
    Process 8 finished
    Process 9 finished
    Process 11 finished
    [Test 2] finished
    [Test 3] with sleep
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    Process 12
    Process 14
    Process 13
    Process 15
    [Test 3] finished
    </pre>

- ### Trouble Shooting
  - ptable의 proc에 있는 프로세스와 스케쥴링 큐가 가지고 있는 프로세스의 동기화 문제
    - 전자에서는 이미 유효하지 않는 프로세스로 변경되었더라도, 스케쥴링 큐는 이를 알아차릴 수가 없다.
    - **_isValidPts_**를 활용해 스케쥴링 시 유효한 프로세스인지 확인해주며 해결하였다!
  - Round Robin과 FCFS 방식에서의 큐 원소 다루는 방식의 차이점에 의한 버그 발생
    - Round Robin방식에서는 항상 맨 앞의 원소를 꺼내서 뒤로 넣기에 (프로세스의 상태가 valid 하면 runnable하지 않더라도 스케쥴링 없이 뒤에 다시 넣어주면 된다.)
    - 하지만 FCFS에서는 맨 앞의 원소가 valid하다면 runnable하지 않더라도 이 위치를 보존하며 다음 원소를 검사해야한다.
    - 또한 그 과정에서 중간 원소가 invalid하다면 큐의 중간에 위치한 이 원소를 삭제해야한다.
    - 구현 초기에 이를 간과하고 FCFS에서도 invalid한 프로세스가 있을 때 dequeue를 호출시켰다. 즉 runnable하지 않던 첫 번째 원소가 영문도 모른채 스케쥴링 대상에서 지워지고 있었다.
    - 원형큐의 동작에 **removeNth**를 추가하여 이를 해결하였다.

### Multilevel Queue

- ### Design
  - **struct proc** 구조체는 **자신의 우선순위, 속한 피드백 큐 레벨, 해당 큐에서의 총 time quantum** 을 가지고 있다.
  - **scheduler()** 함수는 다음과 같은 과정을 반복한다.
    1. **L0** 큐에서부터 마지막 큐까지 아래 과정을 반복한다.
    2. 마지막으로 **boost**를 한 tick값과 현재 tick값을 비교한다. 만약 마지막으로 **boost**를 한지 100tick 이상 지났다면 모든 프로세스를 **LO**큐로 이동시킨 후 **1번**으로 돌아간다.
    3. 선택된 큐에 가장 최신에 실행한 프로그램이 존재하는지 확인한다. 부팅 후 해당 큐에서 스케쥴링이 되기 이전 또는 boost가 된 후 처음 큐에 접근할 때는 존재하지 않게된다. 만약 존재한다면 이 프로세스가 **RUNNABLE**한지 확인 후 그렇다면 이 프로세스를 선택하여 **5번**으로 바로 이동한다. 이 때 가장 마지막 큐라면 이미 time_quantum을 다 사용한 프로세스를 선택하지 않도록 처리한다.
    4. 선택된 큐에서 가장 우선순위가 높은 프로세스를 선택한다.
    5. 선택된 큐의 가장 최근 진행 프로세스를 선택된 프로세스로 설정한다.
    6. 해당 프로세스의 **usedTime**(현재 큐에서 실행한 수)를 **1** 증가시킨다.
    7. 해당 프로세스에 제어권을 넘겨준다.
    8. 제어권이 다시 넘어오면 프로세스의 **usedTime**을 확인해 해당 큐의 한계를 넘어섰다면 다음 큐로 이동시킨다. 이 때 이미 마지막 큐라면 추가로 증가시키지 않는다. 이러한 경우에 해당되는 프로세스는 **3번** 과정에 의해 **boost** 이전에는 스케쥴링이 되지 않는다.
  - **getlev** 시스템콜은 현재 실행 중인 프로세스가 속한 큐의 레벨을 반환한다.
  - **serpriority** 시스템콜은 자식 프로세스의 우선순위를 설정할 수 있다.
  - **yield(), sleep()** 시스템콜 호출 시 해당 프로세스는 **L0**큐로 이동한다.
  - **Multi Level Queue**의 경우와 다르게, **time interrupt**발생 시 모든 경우에 **yield()** 를 호출시킨다.
- ### Implement

  - **struct proc** 구조체에 다음 세 속성을 추가하였다.
    <pre><code>
      int level;
      int priority;
      int usedTime;
    </code></pre>
  - **allocproc()** 에서 다음과 같이 프로세스를 초기화 해준다.
    <pre><code>
      p->level = 0;
      p->priority = 0;
      p->usedTime = 0;
    </code></pre>
  - **sleep(), yield()** 시스템콜 호출 시 다음과 같은 코드로 프로세스를 **L0**으로 이동시킨다.
    <pre><code>
      myproc()->level = 0;
      myproc()->usedTime = 0;
    </code></pre>
  - **setpriority()** 시스템콜은 아래와 같이 구현했다.
    <pre><code>
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
          if (pid == p->pid){
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
    </code></pre>
  - **getlev()** 시스템콜은 아래와 같이 구현했다.
    <pre><code>
      int
      sys_getlev(void)
      {
        return myproc()->level;
      }
    </code></pre>
  - **scheduler()** 함수의 기존 무한루프 자리에 다음과 같은 코드를 배치했다.
    <pre><code>
    struct proc* recentProc[MLFQ_K];
    
    int recentPid[MLFQ_K];
    
      int isStarted[MLFQ_K];
    
      for(int k = 0; k < MLFQ_K; k++) {
        isStarted[k] = 0;
      }
    
      for(;;) {
        // Enable interrupts on this processor.
        sti();
    
        // Loop over process table looking for process to run.
        acquire(&ptable.lock);
    
        int endFlag = 0;
    
        for (int level = 0; level < MLFQ_K; level++) {
    
          //boost
          if (lastBoost + 100 < ticks) {
            for (struct proc* tmpProc = ptable.proc; tmpProc < &ptable.proc[NPROC]; tmpProc++) {
              tmpProc->usedTime = 0;
              tmpProc->level = 0;
            }
    
            for(int k = 0; k < MLFQ_K; k++) {
              isStarted[k] = 0;
            }
    
            endFlag = 1;
            lastBoost = ticks;
          }
    
          if (endFlag) {
            break;
          }
    
          int selectedFlag = 0;
          int isNotRecent = 0;
    
          if(isStarted[level]) {
            p = recentProc[level];
    
            if (!(p->level == MLFQ_K - 1 && p->usedTime >= 2 + 4 * p->level)) {
              if(p->state == RUNNABLE && recentPid[level] == level) {
                selectedFlag = 1;
              }
            }
          }
    
          struct proc* selectedProc;
    
          if(!selectedFlag) {
            int max_priority = -1;
            for (p = ptable.proc; p < &ptable.proc[NPROC]; p++) {
              if (p->state != RUNNABLE)
                continue;
    
              if (p->level != level)
                continue;
    
              if (p->level == MLFQ_K - 1 && p->usedTime >= 2 + 4 * p->level) {
                continue;
              }
    
              if (max_priority >= p->priority) {
                continue;
              } else {
                max_priority = p->priority;
                selectedProc = p;
                isNotRecent = 1;
                selectedFlag = 1;
              }
            }
          }
    
          if (!selectedFlag) {
            continue;
          }
    
          if(isNotRecent) {
            p = selectedProc;
          }
    
          isStarted[level] = 1;
          recentProc[level] = p;
          recentPid[level] = p->pid;
    
          // Switch to chosen process.  It is the process's job
          // to release ptable.lock and then reacquire it
          // before jumping back to us.
          c->proc = p;
          switchuvm(p);
          p->state = RUNNING;
    
          p->usedTime += 1;
          endFlag = 1;
    
          swtch(&(c->scheduler), p->context);
          switchkvm();
    
          if (p->usedTime >= 2 + (4 * level) && level < MLFQ_K - 1) {
            p->usedTime = 0;
            p->level = p->level + 1;
          }
    
          // Process is done running for now.
          // It should have changed its p->state before coming back.
          c->proc = 0;
        }
        release(&ptable.lock);
      }
    </code></pre>

- ### Result
  - **mlfq_test** 프로그램을 k값 5로 지정 후 실행한 결과이다.
    <pre>
      Booting from Hard Disk..xv6...
      cpu0: starting 0
      sb: size 1000 nblocks 941 ninodes 200 nlog 30 logstart 2 inodestart 32 bmap start 58
      init: starting sh
      $ mlfq_test
      MLFQ test start
      [Test 1] default
      Process 4
      L0: 8355
      L1: 18338
      L2: 30471
      L3: 42836
      L4: 0
      Process 5
      L0: 10782
      L1: 21213
      L2: 30211
      L3: 37794
      L4: 0
      Process 6
      L0: 10448
      L1: 26534
      L2: 45928
      L3: 17090
      L4: 0
      Process 7
      L0: 10850
      L1: 27487
      L2: 45961
      L3: 15702
      L4: 0
      [Test 1] finished
      [Test 2] priorities
      Process 11
      L0: 10658
      L1: 27985
      L2: 40011
      L3: 21346
      L4: 0
      Process 10
      L0: 10757
      L1: 27485
      L2: 29359
      L3: 32399
      L4: 0
      Process 9
      L0: 10935
      L1: 27674
      L2: 41182
      L3: 20209
      L4: 0
      Process 8
      L0: 10602
      L1: 27549
      L2: 44995
      L3: 16854
      L4: 0
      [Test 2] finished
      [Test 3] yield
      Process 15
      L0: 20000
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 14
      L0: 20000
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 13
      L0: 20000
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 12
      L0: 20000
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      [Test 3] finished
      [Test 4] sleep
      Process 19
      L0: 500
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 18
      L0: 500
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 17
      L0: 500
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      Process 16
      L0: 500
      L1: 0
      L2: 0
      L3: 0
      L4: 0
      [Test 4] finished
      [Test 5] max level
      Process 20
      L0: 99969
      L1: 31
      L2: 0
      L3: 0
      L4: 0
      Process 21
      L0: 34991
      L1: 65003
      L2: 6
      L3: 0
      L4: 0
      Process 22
      L0: 17969
      L1: 36435
      L2: 45593
      L3: 3
      L4: 0
      Process 23
      L0: 15040
      L1: 28778
      L2: 30283
      L3: 25898
      L4: 1
      [Test 5] finished
      [Test 6] setpriority return value
      done
      [Test 6] finished
    </pre>
- ### Trouble Shooting

  - **mlfq_test 3번** 에서 각 프로세스마다 **L2**큐에서 1회 스케쥴링이 되는 것으로 보이는 문제

    - 다음과 같이 **L2** 큐에 **1**이 찍히는 문제가 있었다.
      <pre>
        [Test 4] sleep
        Process 20
        L0: 499
        L1: 0
        L2: 1
        L3: 0
        L4: 0
        Process 19
        L0: 499
        L1: 0
        L2: 1
        L3: 0
        L4: 0
        Process 18
        L0: 499
        L1: 0
        L2: 1
        L3: 0
        L4: 0
        Process 17
        L0: 499
        L1: 0
        L2: 1
        L3: 0
        L4: 0
      </pre>
    - 로그를 찍어보며 **fork_children2**의 반복문 내 **i==0** 상황에서 **getlev**시스템콜의 반환값이 2로 나오는 것을 확인했다.
    - **i==0**의 상황에만 일어나니 프로세스 전환과정에서의 문제라고 유추했다.
    - **sys_sleep()**에서 프로세스를 **LO**으로 올려주는 코드의 위치를 변경하여 문제를 해결했다.
      <pre><code>
        int
        sys_sleep(void)
        {
          int n;
          uint ticks0;
          if(argint(0, &n) < 0)
            return -1;
            
          //변경전위치
          
          acquire(&tickslock);
          ticks0 = ticks;
          while(ticks - ticks0 < n){
            if(myproc()->killed){
              release(&tickslock);
              return -1;
            }
            sleep(&ticks, &tickslock);
          }
          //변경 후 위치
          myproc()->level = 0;
          myproc()->usedTime = 0;
      
          release(&tickslock);
          return 0;
        }
      </code></pre>

  - 큐에서 최근에 선택했던 프로세스를 스케쥴링할 때 **time quantum**을 증가시키지 않은 실수
    - 결과가 이상하게 나와 로그를 찍어보다가 발견하고 수정했다.
  - **우선순위 큐**를 이용한 구현 실패
    - **Multi Level Queue**를 이용했을 때 조금 다르게 **우선순위 큐**를 사용해볼려고 시도해보았다.
    - 약 3일만 매달려봤으나 미묘하게 결과가 이상하게 나왔다.
    - 어디선가 미묘하게 어긋났는데 이를 찾는데 너무 과도한 시간을 사용하며 결국 번아웃이 왔다.
    - 때문에 이 방법은 학기가 끝난 후 다시 처음부터 코드를 짜보는 것으로 결정하고 다른 방법으로 구현해보기로 했다.
