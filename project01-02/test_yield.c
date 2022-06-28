#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  int pid = fork();

  while (1) {
    if (pid < 0) {
      exit();
    } else if (pid == 0) {
      printf(0, "Child\n");
      yield();
    } else {
      printf(0, "Parent\n");
      yield();
    }
  }
}