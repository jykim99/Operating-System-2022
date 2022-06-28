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