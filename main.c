#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/socket.h>

#define error(MSG) printf(MSG); exit(1);
#define FIFO_FILE "myfifo.txt"
#define LOGIN_FILE "login"
#define MAX_DUPLICATE_FILES 100

#define WRITE_ERROR "Eroare la write\n"
#define READ_ERROR "Eroare la read\n"
#define CLOSE_ERROR "Eroare la close\n"
#define FORK_ERROR "Eroare la fork\n"
#define OPEN_ERROR "Eroare la open\n"
#define PIPE_ERROR "Eroare la pipe\n"
#define SOCKET_ERROR "Eroare la socket\n"
#define MKFIFO_ERROR "Eroare la MkFifo\n"
#define CREATE_ERROR "Eroare la crearea fisierului\n"
#define FOPEN_ERROR "Eroare la FOpen\n"
#define READDIR_ERROR "Eroare la readdir\n"
#define STAT_ERROR "Eroare la stat\n"
#define ACCESS_ERROR "Eroare la access\n"
#define READ_FILE_ERROR "Eroare la readfile\n"

#ifdef HAVE_ST_BIRTHTIME
#define birthtime(x) x.st_birthtime
#else
#define birthtime(x) x.st_ctime
#endif

int isLogged = 0;

int prim(int);
void trimString(char*, char c);
void getBlocks(char [10][100], char *, int*);
int process(char [10][100], int);

void showError();
void showHelp();

int Login(char*);
void showFileStatus(struct stat);
struct stat MyStat(char*, int*);
void MyFind(char*, char*, char[MAX_DUPLICATE_FILES][PATH_MAX], int*);

int main(int argc, char** argv) {
    
    printf("\n\n");
    printf("              ------================================================================------             \n\n");
    printf("              ------==========   Shell-ul Meu Pufos :: Stamate Valentin   ==========------             \n\n");
    printf("              ------================================================================------             \n\n\n");

    char rawCommand[100];

    repeat:

    fgets(rawCommand, sizeof(rawCommand), stdin);
    rawCommand[(int)strlen(rawCommand) - 1] = '\0'; // removing newline

    trimString(rawCommand, ' ');

    char command[10][100];
    int blocks = 0;

    getBlocks(command, rawCommand, &blocks);

    int result = process(command, blocks);

    int pi[2];
    pid_t pid;

    switch (result) {
        case 0:
            showHelp();
            break;
        case 1:

            if (pipe(pi) == -1) {
                error(PIPE_ERROR);
            }
            
            if ( (pid = fork()) == -1 ) {
                error(FORK_ERROR);
            }

            if ( pid == 0 ) {
                int status = Login(command[2]);

                if ( close(pi[0]) == -1) {
                    error(CLOSE_ERROR);
                }

                if (write(pi[1], &status, 4) == -1) {
                    error(WRITE_ERROR);
                }

                if( close(pi[1]) == -1) {
                    error(PIPE_ERROR);
                }
                exit(1);
            } 
                
            if ( close(pi[1]) == -1 ) {
                error(CLOSE_ERROR);
            }

            int status;

            if ( read(pi[0], &status, 4) == -1 ) {
                error(READ_ERROR);
            }
            
            switch (status) {
                case 1:
                    printf("\nSuccesfully logged in\n\n");
                    isLogged = 1;
                    break;
                case 2:
                    printf("Wrong password\n\n");
                    isLogged = 0;
                    break;
                default:
                    printf("New user created. Please login.\n\n");
                    isLogged = 0;
                    break;
            }

            if ( close(pi[0]) == -1 ) {
                error(CLOSE_ERROR);
            }
            break;
        case 2:

            if (isLogged == 0) {
                printf("You cannot run this command unless you're logged in\n\n");
                break;
            }

            int skt[2];
            if ( socketpair(AF_UNIX, SOCK_STREAM, 0, skt) == -1 ) {
                error(SOCKET_ERROR);
            }

            if ( (pid = fork()) == -1 ) {
                error(FORK_ERROR);
            }

            if (pid == 0) {
                
                if ( close(skt[0]) == -1 ) {
                    error(CLOSE_ERROR);
                }
                
                struct stat result;

                int status;
                result = MyStat(command[1], &status);

                if ( write(skt[1], &status, 4) == -1 ) {
                    error(WRITE_ERROR);
                }

                if ( write(skt[1], &result, sizeof(struct stat)) == -1 ) {
                    error(WRITE_ERROR);
                }

                if ( close(skt[1]) == -1 ) {
                    error(CLOSE_ERROR);
                }
                exit(2);
            }

            if ( close(skt[1]) == -1 ) {
                error(CLOSE_ERROR);
            }

            struct stat result;
            int found;

            if ( read(skt[0], &found, 4) == -1 ) {
                error(READ_ERROR);
            }

            if (found == 0) {
                printf("The file doesn't exist.\n\n");
                if ( close(skt[0]) == -1) {
                    error(CLOSE_ERROR);
                }
                break;
            }

            if ( read(skt[0], &result, sizeof(struct stat)) == -1 ) {
                error(READ_ERROR);
            }

            showFileStatus(result);

            if ( close(skt[0]) == -1 ) {
                error(CLOSE_ERROR);
            }
            
            break;
        case 3: 

            if (isLogged == 0) {
                printf("You cannot run this command unless you're logged in\n\n");
                break;
            }

            printf("Searching... \n\n");

            remove(FIFO_FILE);

            if ( mkfifo(FIFO_FILE, 0666) == -1 ) {
                error(MKFIFO_ERROR);
            } 

            if ( (pid = fork()) == -1 ) {
                error(FORK_ERROR);
            }

            if (pid == 0) {
                
                char outputLines[MAX_DUPLICATE_FILES][PATH_MAX];
                int resultLength = 0;

                char start[100] = "/home";

                MyFind(start, command[1], outputLines, &resultLength);

                int fd = open(FIFO_FILE, O_WRONLY);

                if ( write(fd, &resultLength, 4) == -1 ) {
                    error(WRITE_ERROR);
                }

                for (int i = 0; i < resultLength; i++) {
                    int lineLength = (int)strlen(outputLines[i]);
                    if ( write(fd, &lineLength, 4) == -1 ) {
                        error(WRITE_ERROR);
                    }
                    if ( write(fd, outputLines[i], lineLength) == -1 ) {
                        error(WRITE_ERROR);
                    }
                    int found;
                    struct stat fileStatus = MyStat(outputLines[i], &found);
                    if ( write(fd, &fileStatus, sizeof(struct stat)) == -1 ) {
                        error(WRITE_ERROR);
                    }
                }

                if ( close(fd) == -1 ) {
                    error(CLOSE_ERROR);
                }
                exit(3);
            }

            int fdR = open(FIFO_FILE, O_RDONLY);

            if (fdR == -1) {
                error(OPEN_ERROR);
            }

            int lineNumber;
            if ( read(fdR, &lineNumber, 4) == -1 ) {
                error(READ_ERROR);
            }

            if (lineNumber == 0) {
                printf("The file %s cannot be found in the system.\n\n", command[1]);
                break;
            }

            printf("File(s) found:\n\n");

            for (int i = 0; i < lineNumber; i++) {
                int lineSize;
                if ( read(fdR, &lineSize, 4) == -1 ) {
                    error(READ_ERROR);
                }
                char line[PATH_MAX];
                if ( read(fdR, line, lineSize) == -1 ) {
                    error(READ_ERROR);
                }
                
                struct stat lineStatus;
                if ( read(fdR, &lineStatus, sizeof(struct stat)) == -1 ) {
                    error(READ_ERROR);
                }
                
                char goodLine[100];
                strcpy(goodLine, "");
                strncat(goodLine, line, lineSize);

                printf("%s\n", goodLine);
                showFileStatus(lineStatus);
            }

            if ( remove(FIFO_FILE) == -1) {
                printf("Error removing fifo file\n");
            }
            break;

        case 4:

            if ( pipe(pi) == -1 ) {
                error(PIPE_ERROR);
            }

            if ( (pid = fork()) == -1 ) {
                error(FORK_ERROR);
            }

            if (pid == 0) {
                if ( close(pi[0]) == -1 ) {
                    error(CLOSE_ERROR);
                }
                
                char succesLog[100] = "Logged out\0";
                char failLog[100] = "You have to be logged in first in order to log out\0";

                int len = strlen(succesLog);
                char returnMessage[100];
                
                strcpy(returnMessage, succesLog);

                if (isLogged == 0) {
                    strcpy(returnMessage, failLog);    
                    len = strlen(failLog);
                }

                int logStat = 0;

                if ( write(pi[1], &logStat, 4) == -1 ) {
                    error(WRITE_ERROR);
                }
                if ( write(pi[1], &len, 4) == -1 ) {
                    error(WRITE_ERROR);
                }
                if ( write(pi[1], returnMessage, len) == -1 ) {
                    error(WRITE_ERROR);
                }

                if ( close(pi[1]) == -1 ) {
                    error(WRITE_ERROR);
                }
                exit(4);
            }

            if ( close(pi[1]) == -1 ) {
                error(CLOSE_ERROR);
            }

            int newStatus;
            int len;
            char logStatus[100];

            if ( read(pi[0], &newStatus, 4) == -1 ) {
                error(READ_ERROR);
            }
            if ( read(pi[0], &len, 4) == -1 ) {
                error(READ_ERROR);
            }
            if ( read(pi[0], logStatus, len) == -1) { // for some reasons, in output I get more characters
                error(READ_ERROR);
            }

            char logMessage[100]; // so, i rebuild the message
            strcpy(logMessage, "");
            strncat(logMessage, logStatus, len);

            isLogged = newStatus;
            printf("\n\n%s\n\n", logMessage);


            if ( close(pi[0]) == -1 ) {
                error(CLOSE_ERROR);
            }
            break;

        default:
            showError();
            break;
    }

    goto repeat;

    return 0;
}

int Login(char *username) {
    if (access(LOGIN_FILE, F_OK) == -1) {
        int fd = open(LOGIN_FILE, O_RDWR | O_CREAT);

        if (fd == -1) {
            error(CREATE_ERROR);
        }

        if ( close(fd) == -1) {
            error(CLOSE_ERROR);
        }
    }

    FILE *file = fopen(LOGIN_FILE, "r");

    if (file == NULL) {
        error(FOPEN_ERROR);
    }

    char line[256];
    char password[100];

    char *poit;
    while ( (poit = fgets(line, sizeof(line), file)) ) {
        if (poit == NULL) {
            error(READ_FILE_ERROR);
        }
        line[(int)strlen(line) - 1] = '\0';

        char *p = strstr(line, username);

        if (p != NULL && p == line) {
            printf("Welcome %s. Enter your password\n", username);
            scanf("%s", password);

            char storedPassword[100];
            strcpy(storedPassword, line + strlen(username) + 1);

            if (strcmp(password, storedPassword) == 0) {
                isLogged = 1;
                return 1;
            } 
            
            return 2;
        }

    }
    if ( fclose(file) == EOF ) {
        error(CLOSE_ERROR);
    }
    
    // in case user doesn't exist, create a new one

    printf("Welcome %s, enter you new password. (The password cannot contain spaces)\n", username);
    scanf("%s", password);

    sprintf(line, "%s %s\n", username, password);

    file = fopen("login", "a");

    if (file == NULL) {
        error(OPEN_ERROR);
    }

    fprintf(file, "%s", line);

    if ( fclose(file) == EOF) {
        error(CLOSE_ERROR);
    }
    return 3;
}
struct stat MyStat(char *path, int* status) {
    struct stat st;

    int acc = access(path, F_OK);

    if (acc == -1) {
        error(ACCESS_ERROR);
    }

    (*status) = acc == 0;

    if ( stat(path, &st) == -1 ) {
        error(STAT_ERROR);
    }

    return st;
}

void MyFind(char *dirname, char *target, char result[MAX_DUPLICATE_FILES][PATH_MAX], int *lineNumber) {
    DIR *dd = opendir(dirname);

    struct dirent *de;
    
    if (dd != NULL) {
        de = readdir(dd);
    } else {
        return;
    }

    while (de != NULL) {

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            de = readdir(dd);

            continue;
        }

        char name[PATH_MAX];
        sprintf(name, "%s/%s", dirname, de->d_name);

        if (de->d_type == 8 && strcmp(de->d_name, target) == 0) {
            sprintf(result[(*lineNumber)++], "%s", name);
        } else {
            MyFind(name, target, result, lineNumber);
        }

        de = readdir(dd);
    }

    closedir(dd);
}

int process(char command[10][100], int blocks) {
    if ( blocks == 1 && strcmp(command[0], "-h" ) == 0) {
        return 0;
    }

    if ( blocks == 3 && strcmp(command[0], "login") == 0 && strcmp(command[1], ":") == 0 ) {
        return 1;
    }

    if ( blocks == 2 && strcmp(command[0], "mystat") == 0 ) {
        return 2;
    }

    if ( blocks == 2 && strcmp(command[0], "myfind") == 0 ) {
        return 3;
    }

    if ( blocks == 1 && strcmp(command[0], "logout") == 0) {
        return 4;
    }

    if (blocks == 1 && strcmp(command[0], "quit") == 0) {
        printf("Process killed\n");
        exit(1);
    }

    return -1;
}

void showHelp() {
    printf("                     Help                     \n\n");
    printf("              ------===============================   Hello World    ===============================------             \n\n");
    printf("To see the commands for this program use -h and yo'll see this page again\n\n");
    printf("Before using any commands, you have to be logged in. To do this use login : username and then enter your password.\n\n");
    printf("To find a file(s) use myfind filename.\n\n");
    printf("The status of a file can be seen with mystat file_path command.\n\n");
}

void showError() {
    printf("Please enter a valid command. For more information press -h\n");
}

void trimString(char* s, char c) {

    int i = 0;
    while (i < strlen(s)) {
        if (s[i] == c && s[i + 1] == c) {
            strcpy(s + i, s + i + 1);
            i--;
        }

        i++;
    }

    if (s[strlen(s) - 1] == c) {
        s[strlen(s) - 1] = '\0';
    }

}

void showFileStatus(struct stat st) {
    char perm[10];
    strcpy(perm, "");
    strcat(perm, "---------");
    if( S_IRUSR & st.st_mode )  perm[0]='r';
    if( S_IWUSR & st.st_mode )  perm[1]='w';
    if( S_IXUSR & st.st_mode )  perm[2]='x';
    if( S_IRGRP & st.st_mode )  perm[3]='r';
    if( S_IWGRP & st.st_mode )  perm[4]='w';
    if( S_IXGRP & st.st_mode )  perm[5]='x';
    if( S_IROTH & st.st_mode )  perm[6]='r';
    if( S_IWOTH & st.st_mode )  perm[7]='w';
    if( S_IXOTH & st.st_mode )  perm[8]='x';

    if (strcmp(perm, "---------") == 0) {
        printf("File doesn't exist\n");
        return;
    }

    printf("\nFile status:\n"); 
    printf("Total size: %d\n", (int)st.st_size);
    printf("Tile of last access: %d\n", (int)st.st_atim.tv_sec);
    printf("Time of last modification: %d\n", (int)st.st_mtim.tv_sec);
    printf("ID of device containing file: %d\n", (int)st.st_dev);
    int t = birthtime(st);
    printf("Creation date: %d\n", t);

    printf("File permissions: %s\n", perm);

    printf("\n");
}

void getBlocks(char destination[10][100], char *source, int *len) {
    char *p = strchr(source, ' ');

    *len = 0;
    while (p != NULL) {
        int blockLen = p - source;

        strcpy(destination[*len], "");
        strncat(destination[(*len)++], source, blockLen);

        strcpy(source, source + (blockLen + 1));

        p = strchr(source, ' ');
    }
    
    strcpy(destination[*len], "");
    strcat(destination[(*len)++], source);

}
