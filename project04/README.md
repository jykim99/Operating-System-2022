## **Design**

---

### **User Account**

- **auth.h** 에는 **userinfo** 라는 구조체가 선언되어 있습니다.
    <pre><code>
    typedef struct {
        char name[15];
        char password[15];
        char id;
    } userinfo;
    </code></pre>
  - id값은 0부터 autoincrease 합니다. ( **root** 는 반드시 0입니다. )
- **login** 이라는 인증 과정을 수행하는 유저 프로그램을 생성하였습니다.
  - **init** 에서 **sh** 대신 **login** 을 **exec** 합니다.
  - **login** 프로그램은 로그인에 성공 시 **current_user** 를 해당 유저로 설정 후 **sh** 를 실행합니다.
    - **auth.c** 파일에 선언된 **current_user** 는 현재 로그인된 유저를 저장합니다. (부팅 후 로그인 전까지는 root로 설정합니다.)
  - **sh** 에서 **logout** 을 입력 시 다시 **login** 프로그램으로 돌아옵니다.
- **auth.c** 에는 유저 생성 시스템콜 **(sys_add_user)** 과 유저 삭제 시스템콜 **(sys_delete_user)** 이 정의되어 있습니다.
  - **root** 유저만이 해당 시스템콜을 정상적으로 실행할 수 있습니다.

### **File Mode**

- **inode** , **dinode** 구조체에 다음 데이터를 추가했습니다.
  - **owner (short)** owner user id값
    - 이름이 아닌 id값을 따로 가지고 구분하는 이유는 다음과 같습니다.
      - 특정 user 삭제 후 같은 이름의 user를 생성하면 서로 다른 유저임에도 불구하고 이전에 있던 유저의 권한을 그대로 물려받는 문제가 생길 수 있습니다.
  - **authority (short)** 파일 권한 설정 ex) rwxrwx => 63
  - **ownername (char[16])** owner의 이름 (ls modification 시 필요)
- 과제 명세와 같이 다음 상황에서 권한을 확인합니다.
  - **namex** 함수에서 경로를 한 단계씩 따라갈 때 마다 각 디렉토리에 **execute** 권한이 있는지 확인
  - **exec** 함수에서 실행하려는 파일에 **execute** 권한이 있는지 확인
  - **create** 함수에서 생성하려는 파일이 이미 존재하는 경우, 해당 파일에 **write** 권한이 있는지 확인 + **create** 함수에서 생성하려는 파일이 존재하지 않는 경우, 그 디렉토리에 **write** 권한이 있는지 확인
  - **sys_open** 함수에서 열기 모드가 **O_RDONLY** 또는 **O_RDWR** 일 때 read 권한이 있는지 확인
  - **sys_open** 함수에서 열기 모드가 **O_WRONLY** 또는 **O_RDWR** 일 때 write 권한이 있는지 확인
  - **sys_chdir** 함수의 목적지에 **execute** 권한이 있는지 확인
  - **sys_unlink** 함수에서 삭제하려는 파일이 있는 디렉토리에 **write** 권한이 있는지 확인

### **Change Mode**

- **auth.c** 파일에 **sys_chmod()** 를 통해 유저의 권한 수정이 가능합니다.
  - owner 또는 root 유저만 해당 동작을 수행할 수 있습니다.

### **Modification of ls**

- 기존 출력 항목에 **ownername** 과 **권한** 을 추가로 출력하며 **Result** 에 첨부했듯 출력 포맷을 조정하였습니다.

---

## **Implement**

### **User Account**

- **init_user_info()** => 부팅 시 실행하여 유저 정보 파일이 없으면 생성 후 **root** user를 생성합니다. 만약 파일이 있다면 이를 읽어들입니다.

  ```
  int
  init_user_info(void){
  struct inode *ip;
  begin_op();

  // 파일을 읽는다
  info_file = namei("/userinfo");
  if (info_file == 0) {
      // 파일 생성
      info_file = create("/userinfo", T_FILE, 0, 0);
      info_file->owner = 0;
      info_file->authority = MODE_RUSR | MODE_WUSR | MODE_ROTH;
      safestrcpy(info_file->ownername, "root", sizeof (info_file->ownername));

      // 파일 내용 추가
      safestrcpy(user_list[0].name, "root", sizeof(user_list[0].name));
      safestrcpy(user_list[0].password, "0000", sizeof(user_list[0].password));
      user_list[0].id = last_id++;

      if((ip = create("/root", T_DIR, 0, 0)) == 0){
          iunlock(info_file);
          end_op();
          return -1;
      }

      ip->owner = current_user.id;
      ip->authority = MODE_RUSR | MODE_WUSR | MODE_XUSR | MODE_ROTH | MODE_XOTH;
      safestrcpy(ip->ownername, "root", sizeof (ip->ownername));

      iupdate(ip);
      iunlock(ip);

      for (int i = 1; i < 10; i++) {
          safestrcpy(user_list[i].name, "f", sizeof(user_list[i].name));
          safestrcpy(user_list[i].password, "", sizeof(user_list[i].password));
          user_list[i].id = -1;
      }

      // 파일 저장
      save_userinfo(user_list);
      iunlock(info_file);
      } else {
          read_userinfo(user_list);
      }
          end_op();
          return 0;
      }
  ```

- **sys_add_user()** => 유저 추가 시스템콜 ( **add_user()** 호출)

  ```
  int sys_add_user(void) {
      char* name;
      char* password;

      if(argstr(0, &name) < 0)
          return -1;
      if(argstr(1, &password) < 0)
          return -1;

      if (strncmp(current_user.name, "root", sizeof(current_user.name))) {
          cprintf("ADD_USER: not a root user");
          return -1;
      }

      return add_user(name, password);
  }
  ```

- **sys_delete_user()** => 유저 삭제 시스템콜 ( **delete_user()** 호출)

  ```
      int
      sys_delete_user(void)
      {
      char* name;

      if(argstr(0, &name) < 0)
          return -1;

      if (strncmp(current_user.name, "root", sizeof(current_user.name))) {
          cprintf("DELETE_USER: not a root user\n");
          return -1;
      }

      return delete_user(name);
      }
  ```

- **add_user()** => 유저 추가 함수

  ```
      int add_user(char* name, char* password){
      struct inode *ip;

      userinfo* free = 0;

      for (int i = 0; i < 10; i++) {
          if (!strncmp(user_list[i].name, name, sizeof (user_list[i].name))) {
          cprintf("ADD_USER : %s is exist name\n", name);
          return -1;
          }

          if (!strncmp(user_list[i].name, "f", sizeof (user_list[i].name)) && !free) {
          free = &user_list[i];
          }
      }

      if(!free) {
          cprintf("ADD_USER : no more user space\n");
          return -1;
      }

      safestrcpy(free->name, name, sizeof (free->name));
      safestrcpy(free->password, password, sizeof (free->password));
      free->id = last_id++;

      begin_op();
      ilock(info_file);

      char tmp[100];
      tmp[0] = '/';
      safestrcpy(&tmp[1], free->name, sizeof (free->name));

      if((ip = create(tmp , T_DIR, 0, 0)) == 0){
          iunlock(info_file);
          end_op();
          return -1;
      }

      ip->owner = free->id;
      ip->authority = MODE_RUSR | MODE_WUSR | MODE_XUSR | MODE_ROTH | MODE_XOTH;
      safestrcpy(ip->ownername, free->name, sizeof (ip->ownername));

      iupdate(ip);
      iunlock(ip);

      save_userinfo(user_list);
      iunlock(info_file);
      end_op();

      return 0;
  }
  ```

- **delete_user()** => 유저 삭제 함수

  ```
  int delete_user(char* name) {
      if (!strncmp(name, "root", sizeof (name))) {
          cprintf("DELETE_USER : root user cannot be deleted \n");
          return -1;
      }

      for (int i = 0; i < 10; i++) {
          if (!strncmp(user_list[i].name, name, sizeof (user_list[i].name))) {

          safestrcpy(user_list[i].name, "f", sizeof (user_list[i].name));
          safestrcpy(user_list[i].password, "", sizeof (user_list[i].password));
          user_list[i].id = -1;

          begin_op();
          ilock(info_file);
          save_userinfo(user_list);
          iunlock(info_file);
          end_op();

          return 0;
          }
      }

      cprintf("DELETE_USER : %s user cannot be found \n", name);
      return -1;
  }
  ```

- **save_userinfo()** => 유저 배열을 디스크에 저장한다.
  ```
  void save_userinfo(userinfo* user_list) {
      int tmp = 0;
      for (int i = 0; i < 10; i++){
          writei(info_file, user_list[i].name, tmp, ( sizeof (user_list[i].name)));
          tmp += sizeof (user_list[i].name);
          writei(info_file, user_list[i].password, tmp, ( sizeof (user_list[i].password)));
          tmp += sizeof (user_list[i].password);
          writei(info_file, &user_list[i].id, tmp, (sizeof (char)));
          tmp += sizeof (char );
      }
      writei(info_file, &last_id, tmp, (sizeof (char)));
      iupdate(info_file);
  }
  ```
- **read_userinfo()** => 유저 배열을 디스크로부터 읽어들인다.
  ```
  void read_userinfo(userinfo* user_list) {
      int tmp = 0;
      ilock(info_file);
      for (int i = 0; i < 10; i++){
          readi(info_file, user_list[i].name, tmp, ( 15 * sizeof(char)));
          tmp += 15 * sizeof(char);
          readi(info_file, user_list[i].password, tmp, ( 15 * sizeof(char)));
          tmp += 15 * sizeof(char);
          readi(info_file, &user_list[i].id, tmp, ( sizeof(char)));
          tmp += sizeof(char);
      }
      readi(info_file, &last_id, tmp, (sizeof (char )));
      iunlock(info_file);
  }
  ```
- **login.c** => 로그인 과정을 수행하는 유저 프로그램

  ```
  #include "types.h"
  #include "user.h"

  char *argv[] = { "sh", 0 };

  int main(int argc, char* argv[]){
  char name[15];
  char password[15];

  inituserinfo();

  do {
      printf(0, "Enter the user name.\n");
      gets(name, 20);
      name[strlen(name) - 1] = '\0';

      printf(0, "Enter the password.\n");
      gets(password, 20);
      password[strlen(password) - 1] = '\0';
  } while (login(name, password));

  exec("sh", argv);
  }
  ```

- **sh** 에 **logout** 을 추가하였습니다.
  ```
  if(!strcmp(buf, "logout\n")) {
      printf(0, "logout\n");
      exit();
  }
  ```

### **File Mode**

- **isRoot()** => **current_user** 가 **root** 인지 확인합니다.
  ```
  int isRoot(){
  if (current_user.id == 0) {
      return 1;
  }
  return 0;
  }
  ```
- **isOwner()** => **current_user** 가 파일의 **owner** 인지 확인합니다.
  ```
  int isOwner(struct inode ip){
  if (ip.owner == current_user.id) {
      return 1;
  }
  return 0;
  }
  ```
- **hasAuthority()** => 권한 비트를 비교합니다.
  ```
  int hasAuthority(short fileAuthority, short authority){
  if (fileAuthority & authority) {
      return 1;
  }
  return 0;
  }
  ```
- **checkReadAuthority()** => **current_user** 가 파일의 read 권한을 가지고 있는지 검사합니다.

  ```
  int checkReadAuthority(struct inode ip) {
      if (ip.authority == 0){
          return 0;
      }

      if (isRoot()) {
          if(!hasAuthority(ip.authority, MODE_RUSR)) {
          cprintf("checkReadAuthority: error\n");
          return -1;
          }
          return 0;
      }

      if(isOwner(ip)){
          if(!hasAuthority(ip.authority, MODE_RUSR)) {
          cprintf("checkReadAuthority: error\n");
          return -1;
          }
      } else {
          if(!hasAuthority(ip.authority, MODE_ROTH)) {
          cprintf("checkReadAuthority: error\n");
          return -1;
          }
      }

      return 0;
  }
  ```

- **checkWriteAuthority()** => **current_user** 가 파일의 write 권한을 가지고 있는지 검사합니다.

  ```
  int checkWriteAuthority(struct inode ip) {
      if (ip.authority == 0){
          return 0;
      }

      if (isRoot()) {
          if(!hasAuthority(ip.authority, MODE_WUSR)) {
          cprintf("checkWriteAuthority: error\n");
          return -1;
          }
          return 0;
      }

      if(isOwner(ip)){
          if(!hasAuthority(ip.authority, MODE_WUSR)) {
          cprintf("checkWriteAuthority: error\n");
          return -1;
          }
      } else {
          if(!hasAuthority(ip.authority, MODE_WOTH)) {
          cprintf("checkWriteAuthority: error\n");
          return -1;
          }
      }

      return 0;
  }
  ```

- **checkExecAuthority()** => **current_user** 가 파일의 exec 권한을 가지고 있는지 검사합니다.

  ```
  int checkExecAuthority(struct inode ip) {
      if (ip.authority == 0){
          return 0;
      }

      if (isRoot()) {
          if(!hasAuthority(ip.authority, MODE_XUSR)) {
          cprintf("checkExecAuthority: error\n");
          return -1;
          }
      //    cprintf("checkExecAuthority: by root\n");
          return 0;
      }

      if(isOwner(ip)){
      //    cprintf("checkExecAuthority: owner\n");
          if(!hasAuthority(ip.authority, MODE_XUSR)) {
          cprintf("checkExecAuthority: error\n");
          return -1;
          }
      } else {
      //    cprintf("checkExecAuthority: no owner\n");
          if(!hasAuthority(ip.authority, MODE_XOTH)) {
          cprintf("checkExecAuthority: error\n");
          return -1;
          }
      }

      //  cprintf("checkExecAuthority: pass\n");
      return 0;
  }
  ```

- **namex** 함수에서 경로를 한 단계씩 따라갈 때 마다 각 디렉토리에 **execute** 권한이 있는지 확인
  ```
  if(checkExecAuthority(*ip)){
      cprintf("namex: no authority\n");
      iunlockput(ip);
      return 0;
  }
  ```
- **exec** 함수에서 실행하려는 파일에 **execute** 권한이 있는지 확인
  ```
  if(checkExecAuthority(*ip)){
      cprintf("exec: fail (no authority)\n");
      end_op();
      return -1;
  }
  ```
- **create** 함수에서 생성하려는 파일이 이미 존재하는 경우, 해당 파일에 **write** 권한이 있는지 확인 + **create** 함수에서 생성하려는 파일이 존재하지 않는 경우, 그 디렉토리에 **write** 권한이 있는지 확인

  ```
  if((ip = dirlookup(dp, name, 0)) != 0){
      iunlockput(dp);

      if(checkWriteAuthority(*ip)){
      cprintf("create: no authority (ip)\n");
      return 0;
      }

      ilock(ip);
      if(type == T_FILE && ip->type == T_FILE)
      return ip;
      iunlockput(ip);
      return 0;
  }

  if(checkWriteAuthority(*dp)){
      cprintf("create: no authority (ip)dn\n");
      iunlockput(dp);
      return 0;
  }
  ```

- **sys_open** 함수에서 열기 모드가 **O_RDONLY** 또는 **O_RDWR** 일 때 read 권한이 있는지 확인
  ```
  if(omode & O_RDONLY || omode & O_RDWR) {
      if(checkReadAuthority(*ip)){
          cprintf("open: fail (no authority R)\n");
          iunlockput(ip);
          end_op();
          return -1;
      }
  }
  ```
- **sys_open** 함수에서 열기 모드가 **O_WRONLY** 또는 **O_RDWR** 일 때 write 권한이 있는지 확인
  ```
  if(omode & O_WRONLY || omode & O_RDWR) {
      if(checkWriteAuthority(*ip)){
          cprintf("open: fail (no authority W)\n");
          iunlockput(ip);
          end_op();
          return -1;
      }
  }
  ```
- **sys_chdir** 함수의 목적지에 **execute** 권한이 있는지 확인
  ```
  if(checkExecAuthority(*ip)){
      iunlockput(ip);
      end_op();
      return -1;
  }
  ```
- **sys_unlink** 함수에서 삭제하려는 파일이 있는 디렉토리에 **write** 권한이 있는지 확인
  ```
  if(checkWriteAuthority(*dp)){
      cprintf("unlink: no authority\n");
      iunlockput(dp);
      end_op();
      return -1;
  }
  ```
- **sys_open** 시스템콜로 생성되는 파일의 권한정보를 다음과 같이 설정
  ```
  ip->owner = current_user.id;
  ip->authority = MODE_RUSR | MODE_WUSR | MODE_ROTH;
  safestrcpy(ip->ownername, current_user.name, sizeof (ip->ownername));
  ```
- **sys_mkdir** 시스템콜로 생성되는 파일의 권한정보를 다음과 같이 설정
  ```
  ip->owner = current_user.id;
  ip->authority = MODE_RUSR | MODE_WUSR | MODE_XUSR | MODE_ROTH | MODE_XOTH;
  safestrcpy(ip->ownername, current_user.name, sizeof (ip->ownername));
  ```
- **mkfs.c** 파일 내 **ialloc()** 에 다음 코드 추가 (권한 설정)
  ```
  din.owner = xshort(0);
  din.authority = xshort(MODE_RUSR | MODE_WUSR | MODE_XUSR | MODE_ROTH | MODE_XOTH);
  strcpy(din.ownername, "root");
  ```
- **fs.c** 파일 내 **ilock()** 에 다음 코드 추가 (권한 설정)
  ```
  ip->owner = dip->owner;
  ip->authority = dip->authority;
  safestrcpy(ip->ownername, dip->ownername, sizeof (ip->ownername));
  ```

### **Change Mode**

- **auth.c** 내 **sys_chmod** 구현

  ```
  int sys_chmod(void){
      char* pathname;
      int mode;
      struct inode* ip;

      if(argstr(0, &pathname) < 0)
          return -1;

      if(argint(1, &mode) < 0)
          return -1;


      begin_op();

      if((ip = namei(pathname)) == 0) {
          end_op();
          return -1;
      }

      ilock(ip);

      if(!(isRoot() || isOwner(*ip))) {
          cprintf("chmod: no authority\n");
          iunlock(ip);
          end_op();
          return -1;
      }

      //  cprintf("mode is %d\n", mode);
      ip->authority = mode;

      iupdate(ip);

      iunlock(ip);
      end_op();

      return 0;
  }
  ```

### **Modification of ls**

- **ls.c** 에서 다음과 같은 방법으로 권한비트를 문자열로 변환

  ```
  for (int i = 0; i < 6; i++) {
      if(powOfTwo(5 - i) & st.authority){
          if (i % 3 == 0) {
              authority[i + 1] = 'r';
          } else if (i % 3 == 1) {
              authority[i + 1] = 'w';
          } else {
              authority[i + 1] = 'x';
          }
      } else {
          authority[i + 1] = '-';
      }
  }

  if (st.type == T_DIR){
      authority[0] = 'd';
  } else {
      authority[0] = '-';
  }
  ```

---

## **Result**

### **login**

<pre>
Enter the user name.
root
Enter the password.
1234
LOGIN: There is no user info
Enter the user name.
root
Enter the password.
0000
</pre>

### **ls**

<pre>
.              drwxr-x
owner: root type: 1 ino: 1 size:512

..             drwxr-x
owner: root type: 1 ino: 1 size:512

README         -rwxr-x
owner: root type: 2 ino: 2 size:2286

cat            -rwxr-x
owner: root type: 2 ino: 3 size:16528

echo           -rwxr-x
owner: root type: 2 ino: 4 size:15384

forktest       -rwxr-x
owner: root type: 2 ino: 5 size:9696

grep           -rwxr-x
owner: root type: 2 ino: 6 size:18744

init           -rwxr-x
owner: root type: 2 ino: 7 size:15972

kill           -rwxr-x
owner: root type: 2 ino: 8 size:15412

ln             -rwxr-x
owner: root type: 2 ino: 9 size:15264

ls             -rwxr-x

owner: root type: 2 ino: 10 size:20456

mkdir          -rwxr-x

owner: root type: 2 ino: 11 size:15508

rm             -rwxr-x

owner: root type: 2 ino: 12 size:15488

sh             -rwxr-x
owner: root type: 2 ino: 13 size:28272

stressfs       -rwxr-x
owner: root type: 2 ino: 14 size:16400

usertests      -rwxr-x
owner: root type: 2 ino: 15 size:67504

wc             -rwxr-x
owner: root type: 2 ino: 16 size:17260

zombie         -rwxr-x
owner: root type: 2 ino: 17 size:15084

login          -rwxr-x
owner: root type: 2 ino: 18 size:15860

useradd_test   -rwxr-x
owner: root type: 2 ino: 19 size:15628

userdelete_tes -rwxr-x
owner: root type: 2 ino: 20 size:15536

chmod_test     -rwxr-x
owner: root type: 2 ino: 21 size:16044

console        -------
owner:  type: 3 ino: 22 size:0

userinfo       -rw-r--
owner: root type: 2 ino: 23 size:311

root           drwxr-x
owner: root type: 1 ino: 24 size:32

</pre>

---

## **Trouble Shooting**

- ### id(short) 기반의 user 구분 방식으로 인한 ls 출력 문제
  **userinfo** 구조체 내 **id** 를 기반으로 user를 구분하도록 **inode** 와 **dinode** 에 데이터를 저장하여 **ls** 에서 **owner** 의 이름을 출력할 수 없는 문제가 생겼다. **inode** , **dinode**, **stat** 에 **username** 이라는 값을 추가로 저장하여 이를 해결했다.
- ### 부팅 시 존재하는 프로그램들의 owner와 authority가 설정되지 않는 문제
  **fs.c** 파일 내의 **ilock()** 에서 새로 추가된 데이터들에 대응을 해주지 않아 생기는 문제였다.
- ### 비트 연산자 실수로 인한 오류
  **hasAuthority()** 내에서, **&** 대신 **|** 를 실수로 사용하여 권한 확인이 제대로 되지 않는 문제가 있어 수정했다.

---

## **Additional Information**

---

- **project04** 디렉토리를 새로 생성하여 과제를 구현했습니다.
- **sys_open()** 호출 시 권한 검사를 테스팅 하기위해 다음과 같은 테스트 코드를 작성했습니다.

  ```
  void open_test();

  int main(int argc, char* argv[]){

      open("/testfile", O_CREATE);

      printf(0, "MODE_XOTH | MODE_XUSR | MODE_RUSR | MODE_ROTH\n");
      chmod("/testfile", MODE_XOTH | MODE_XUSR | MODE_RUSR | MODE_ROTH);

      open_test();

      printf(0, "MODE_XOTH | MODE_XUSR | MODE_WUSR | MODE_WOTH\n");
      chmod("/testfile", MODE_XOTH | MODE_XUSR | MODE_WUSR | MODE_WOTH);

      open_test();

      printf(0, "MODE_XOTH | MODE_XUSR | MODE_WUSR | MODE_WOTH\n");
      chmod("/testfile", MODE_XOTH | MODE_XUSR | MODE_WUSR | MODE_WOTH | MODE_ROTH | MODE_RUSR);

      open_test();

      exit();
      }

      void open_test(){
      if(open("/testfile", O_RDONLY) != -1){
          printf(0, "O_RDONLY open pass\n");
      } else {
          printf(0, "O_RDONLY open fail\n");
      }

      if(open("/testfile", O_WRONLY) != -1){
          printf(0, "O_WRONLY open pass\n");
      } else {
          printf(0, "O_WRONLY open fail\n");
      }

      if(open("/testfile", O_RDWR) != -1){
          printf(0, "O_RDWR open pass\n");
      } else {
          printf(0, "O_RDWR open fail\n");
      }

  }
  ```

---
