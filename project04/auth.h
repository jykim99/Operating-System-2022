#define MAX_USER 10

typedef struct {
    char name[15];
    char password[15];
    char id;
} userinfo;

int
init_user_info(void);

int isRoot();
int isOwner(struct inode ip);

int hasAuthority(short fileAuthority, short authority);

int checkReadAuthority(struct inode ip);
int checkWriteAuthority(struct inode ip);
int checkExecAuthority(struct inode ip);

