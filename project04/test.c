#include "types.h"
#include "user.h"
#include "fcntl.h"
#include "fs.h"

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
