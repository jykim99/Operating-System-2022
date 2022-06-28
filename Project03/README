## Design & Implement

---

### Thread를 담을 자료구조에 대한 고민

**thread** 는 기존의 **proc** 구조체와 상당한 유사점을 가지고 있다. 그렇기에 기존 **proc** 구조체를 그대로 활용하여 이를 활용하기로 했다. **struct proc** 에 새로 추가된 항목은 다음과 같다. 이때 기존의 **parent** 는 아래 추가된 **main** 과 달리 반드시 프로세스가 아닌 스레드를 가르킬 수 있다.

- **int isthread**
  - 자신이 스레드인지 프로세스인지 구별한다.
- **struct proc \*main**
  - 자신을 포함하는 프로세스를 가르킨다.
  - 만약 자신이 프로세스라면 자기 자신을 가르킨다.
- **void\* retval**
  - 스레드의 경우 리턴값을 저장하는데 사용된다.

---

### thread_create

<pre><code>
int
thread_create(thread_t *thread, void *(*start_routine)(void *), void *arg) {
  uint sz, sp, newsz;
  struct proc *nt;
  struct proc *curproc;

  curproc = myproc();

  if((nt = allocproc()) == 0) {
    return -1;
  }

  // 쓰레드 번호 저장 (pid값 활용함)
  *thread = nt->pid;

  // 쓰레드 생성하니까 1로 저장
  nt->isthread = 1;

  // 메인 프로세스 설정
  nt->main = curproc->main;

  // 얘네는 그냥 복사해오면 된다.
  nt->parent = curproc;
  safestrcpy(nt->name, curproc->name, sizeof(curproc->name));
  for(int i = 0; i < NOFILE; i++){
    if(curproc->ofile[i]){
      nt->ofile[i] = filedup(curproc->ofile[i]);
    }
  }
  nt->cwd = idup(curproc->cwd);
  nt->pgdir = curproc->pgdir;

  // 유저 스택 공간 확보
  sz = curproc->main->sz;
  sz = PGROUNDUP(sz);
  newsz = allocuvm(curproc->pgdir, sz, sz + 2*PGSIZE);
  nt->sz = newsz;
  curproc->main->sz = newsz;

  clearpteu(curproc->pgdir, (char*)(newsz - 2*PGSIZE));

  sp = newsz;

  //유저 스택 값 넣기
  sp -= 4;
  *((uint*)sp) = (uint) arg;
  sp -= 4;
  *((uint*)sp) = 0xffffffff;

  // eip와 esp 설정하기
  *nt->tf = *curproc->tf;
  nt->tf->eip = (uint)start_routine;
  nt->tf->esp = sp;

  //생성했으니 스케쥴링이 가능하도록 설정
  acquire(&ptable.lock);
  nt->state = RUNNABLE;
  release(&ptable.lock);

  return 0;
}
</code></pre>

1. **allocproc()** 을 이용해 커널스택 할당등을 하여 proc 구조체를 생성한다.
2. 스레드 번호를 저장한다. (allocproc에서 반환된 proc의 pid값을 이용해 지정한다.)
3. **main** 의 값을 지정한다. 이는 생성하는 프로세스 혹은 스레드와 같게 설정된다.
4. **parent** 는 현재 생성하는 프로세스 또는 스레드로 설정된다.
5. **pgdir** 을 복사하여 메모리 공간을 공유시킨다.
6. **cwd** 와 **ofile** 을 공유시킨다.
7. **allocuvm()** 을 활용하여 유저 스택 공간을 확보한다.
   - 이때 **main** 의 **sz** 값도 새로운 **sz** 값으로 변경해준다. (**sbrk** 등에 필요하다.)
8. **clearpteu** 를 통해 가드 페이지를 설정한다.
9. 유저 스택에 값을 넣어준다. (인자 값과 fack PC값)
10. **eip** (다음 실행 주소)와 **esp** (스택 프레임)를 설정해준다.
11. 스케쥴링이 가능하도록 설정해준다.

---

### thread_exit

<pre><code>
void
thread_exit(void *retval)
{

  struct proc *curthread;

  curthread = myproc();

  acquire(&ptable.lock);

  curthread->state = ZOMBIE;
  curthread->retval = retval;

  wakeup1(curthread->parent);

  sched();
}
</code></pre>

1. 현재 스레드를 ZOMBIE 상태로 설정한다.
2. 리턴 값을 설정한다. (**retval** 를 활용한다.)
3. 자신의 부모를 **wakeup** 한다.

---

### thread_join

<pre><code>
int
thread_join(thread_t thread, void **retval)
{
  struct proc *curthread;
  struct proc *endthread;

  while (1)
  {
    int ended = 0;
    curthread = myproc();

    acquire(&ptable.lock);
    for(endthread = ptable.proc; endthread < &ptable.proc[NPROC]; endthread++){
      if (endthread->parent == curthread && endthread->state == ZOMBIE && endthread->pid == (int) thread){
        *retval = endthread->retval;
        deallocuvm(endthread->pgdir, endthread->sz, (endthread->sz - 2 * PGSIZE));
        kfree(endthread->kstack);
        endthread->kstack = 0;
        endthread->main = 0;
        endthread->parent = 0;
        endthread->name[0] = 0;
        endthread->killed = 0;
        endthread->state = UNUSED;
        endthread->retval = 0;
        endthread->isthread = 0;
        endthread->chan = 0;
        ended = 1;
        break;
      }
    }
    release(&ptable.lock);

    if(ended) {
      return 0;
    }

    acquire(&tickslock);
    sleep(curthread, &tickslock);
    release(&tickslock);
  }
}
</code></pre>

1. 아래와 같은 내용을 반복한다.
2. ptable를 순회하며 자신의 자식 중 ZOMBIE 상태가 있는지 확인한다.
3. 만약 있다면 자식의 자원을 반납
4. ended값이 true(1)라면 반복문을 종료한다.

---

### fork

<pre><code>
np->main = np;
</code></pre>

새로운 프로세스의 main을 np로 설정하는 코드를 추가했다.

---

### exec

<pre><code>
struct proc *curprocmain = curproc->main;
struct proc *curprocmainparent = curproc->main->parent;

...

// 기존 스레드 모두 정리하기
acquire(&ptable.lock);
for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->main == curprocmain && p != curproc){
        (자원할당해제)
    }
}
release(&ptable.lock);

curproc->main = curproc;
curproc->parent = curprocmainparent;
</code></pre>

- **exec** 호출 시 **curprocmain** 에 현재 스레드의 **main** 을 저장하고, **curprocmainparent** 에 그 프로세스의 **parent** 를 지정한다.
- 호출을 종료하기 전, **curprocmain** 을 **main** 으로 가르키는 모든 스레드(main 프로세스일 수도 있다)를 자원할당해제(**UNUSED** 상태로 설정)한다.
- 새로 **exec** 하는 프로세스의 **main** 을 자기 자신으로 설정 후, **parent** 를 **curprocmainparent** 로 설정해준다.

---

### sbrk

**growproc()** 에서 기준이 되는 **sz** 값을 현재 실행중인 스레드(프로세스)가 자체가 아닌 그의 **main** 기준으로 설정하도록 변경했다.

<pre><code>
sz = curproc->main->sz;
...
curproc->main->sz = sz;
</code></pre>

**sys_sbrk()** 의 리턴값 또한 **main** 의 **sz** 를 리턴한다. (트러블 슈팅 항목에서도 등장할 내용이다)

<pre><code>
addr = myproc()->main->sz;
</code></pre>

---

### exit

<pre><code>
struct proc *curproc = myproc()->main;
struct proc *th;

...

for(th = ptable.proc; th < &ptable.proc[NPROC]; th++){
    if(th->main == curproc && th != curproc){
        (자원할당해제)
    }
}

...

</code></pre>

**exit()** 를 호출하면 현재 실행중인 프로세스 혹은 스레드의 **main** 을 가르키는 모든 스레드를 자원할당해제한다. 그 이후 해당 **main** 을 기존 로직을 이용해 **exit** 한다.

---

### kill

**kill** 호출 시 **trap.c** 에서 **exit** 을 호출한다. **exit** 에서 스레드에 대한 자원할당해제 처리를 해두어 **kill** 에 대해 추가적인 처리는 하지 않았다.

---

### sleep

프로세스 기반으로 스레드를 구현했기에 **sleep** 를 위해 따로 처리할 내용은 없었다.

---

### pipe

스레드 구현 후 추가적인 처리 없이도 잘 동작하였다.

---

## Result

---

### thread_test

<pre>
$ thread_test
Test 1: Basic test
Thread 0 start
Thread 0 end
Thread 1 start
Parent waiting for children...
Thread 1 end
Test 1 passed

Test 2: Fork test
Thread 0 start
Thread 1 start
Thread 2 start
Thread 3 start
Child of thread 0 start
Child of thread 2 start
Child of thread 3 start
Thread 4 start
Child of thread 1 start
Child of thread 4 start
Child of thread 0 end
Child of thread 2 end
Thread 0Thread 2 end
Child of thread 3 end
 end
Thread 3 end
Child of thread 1 end
Thread 1 end
Child of thread 4 end
Thread 4 end
Test 2 passed

Test 3: Sbrk test
Thread 0 start
Thread 0 malloc startThread 1 start
Thread 2 start
Thread 3 start
Threa
Thread 0 malloc success
d 4 start
Test 3 passed

All tests passed!
</pre>

---

### thread_exec

<pre>
$ thread_exec
Thread exec test start
Thread 0 start
Thread 1 start
Thread Thread 3 start
Thread 4 start
2 start
Executing...
Hello, thread!
</pre>

---

### thread_exit

<pre>
$ thread_exit
Thread exit test start
Thread 0 start
Thread 1 start
Thread 2 start
Thread 3 stThread 4 start
art
Exiting...
</pre>

---

### thread_kill

<pre>
$ thread_kill
Thread kill test start
Killing process 35
This code should be executed 5 times.
This code should be executed 5 times.
This code should be executed 5 times.
This code should be executed 5 times.
This code should be executed 5 times.
Kill test finished
</pre>

---

## Trouble Shooting

---

### 스레드가 일정한 확률로 잘못된 값을 반환하는 문제

**thread_join** 에서 자원을 회수하고 리턴값을 받을 스레드를 탐색하는 과정에서 실수가 있었다. 스레드 번호를 검사하지 않아 다른 스레드들을 join하는 경우가 생겼다. 문제 해결을 위해 다음과 같은 조건을 추가했다.

<pre><code>
endthread->pid == (int) thread
</code></pre>

---

### panic remap

**sbrk** 실행 시 실행 중인 스레드(프로세스)의 **main** 의 **sz** 를 기준으로 잡지 않으면, **remap** 오류가 날 수 있다는 것을 확인했다. (스레드의 **sz** 의 윗부분이 비어있다는 보장이 없다!)

---

### malloc 실행 시 trap 14

꽤나 오랜 시간을 빼앗긴 문제이다. 유저가 **malloc** 을 호출 시 중간에 **trap 14** 가 뜨면서 테스트가 정상적으로 수행되지 않았다. 이는 malloc 함수에서 리턴하는 주소가 **main** 이 아닌 현재 스레드의 **sz** 값이였기에 발생하는 문제였다. **growproc** 에서 **main** 을 기준으로 메모리를 할당했으니 **malloc** 에서도 **main** 을 기준으로 **sz** 를 반환했더니 문제가 해결되었다.
