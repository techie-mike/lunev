#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
    for (int i = 0; i < 100; i++) {
        pid_t child_pid = fork();

        pid_t parent_pid = getppid();
        pid_t my_pid = getpid();

        for (long long i = 0; i < 100000; i++) ;
        if (child_pid == 0) {
            printf("Id: %6d Pid: %6d PPid: %6d\n", i, my_pid, parent_pid);
            return 0;
        }
        
        // const char* file_name = "/home/techie/Documents/lunev/lesson_2_1"; 
        // execv("/home/techie/Documents/lunev/lesson_2_1", )

    }
    return 0;
}