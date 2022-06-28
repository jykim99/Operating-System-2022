#include "types.h"
#include "fs.h"
#include "defs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "auth.h"

userinfo current_user = {.name = "root", .password = "0000", .id = 0};
userinfo user_list[10];

char last_id = 0;

struct inode* info_file;

struct inode* create(char *path, short type, short major, short minor);

int add_user(char* name, char* password);
int delete_user(char* name);
int login(char* name, char* password);

void save_userinfo(userinfo* user_list);
void read_userinfo(userinfo* user_list);

int
init_user_info(void)
{
  struct inode *ip;

  begin_op();

  // 파일을 읽는다
  info_file = namei("/userinfo");

  if (info_file == 0) {
//    cprintf("userinfo 없어서 생성해준다.\n");

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
//    cprintf("userinfo 있음.\n");
    read_userinfo(user_list);
  }

  end_op();

  return 0;
}

int
sys_login(void)
{
  char* name;
  char* password;

  if(argstr(0, &name) < 0)
    return -1;
  if(argstr(1, &password) < 0)
    return -1;

  return login(name, password);
}

int
sys_add_user(void)
{
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

int login(char* name, char* password) {
  for (int i = 0; i < 10; i++) {
    if (!strncmp(user_list[i].name, name, sizeof (user_list[i].name)) && !strncmp(user_list[i].password, password, sizeof (user_list[i].password))) {
      current_user = user_list[i];
      return 0;
    }
  }

  cprintf("LOGIN: There is no user info\n");
  return -1;
}

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

struct inode*
create(char *path, short type, short major, short minor)
{
  struct inode *ip, *dp;
  char name[DIRSIZ];

  if((dp = nameiparent(path, name)) == 0)
    return 0;
  ilock(dp);

  if((ip = dirlookup(dp, name, 0)) != 0){
    iunlockput(dp);
    ilock(ip);
    if(type == T_FILE && ip->type == T_FILE)
      return ip;
    iunlockput(ip);
    return 0;
  }

  if((ip = ialloc(dp->dev, type)) == 0)
    panic("create: ialloc");

  ilock(ip);
  ip->major = major;
  ip->minor = minor;
  ip->nlink = 1;
  iupdate(ip);

  if(type == T_DIR){  // Create . and .. entries.
    dp->nlink++;  // for ".."
    iupdate(dp);
    // No ip->nlink++ for ".": avoid cyclic ref count.
    if(dirlink(ip, ".", ip->inum) < 0 || dirlink(ip, "..", dp->inum) < 0)
      panic("create dots");
  }

  if(dirlink(dp, name, ip->inum) < 0)
    panic("create: dirlink");

  iunlockput(dp);

  return ip;
}

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

int isRoot(){
  if (current_user.id == 0) {
    return 1;
  }
  return 0;
}

int isOwner(struct inode ip){
  if (ip.owner == current_user.id) {
    return 1;
  }
  return 0;
}

int hasAuthority(short fileAuthority, short authority){
  if (fileAuthority & authority) {
    return 1;
  }

  return 0;
}

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




