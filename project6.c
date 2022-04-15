/* CS 361 project6.c

  Team: 
  Names: 
  Honor Code Statement: 

*/

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <mqueue.h>
#include <pthread.h>
#include <time.h>

#include "common.h"
#include "classify.h"
#include "intqueue.h"


void *process_result(void *arg) {

    return NULL;

}

int main(int argc, char *argv[])
{
    int input_fd;
    pid_t pid;
    off_t file_size;
    struct mq_attr attr;
    char tasks_mq_name[16];
    char results_mq_name[18];
    struct task new_task;
    int num_clusters;
    pthread_t processor[NUM_THREADS];

    if (argc != 2) {
        printf("Usage: %s data_file\n", argv[0]);
        return 1;
    }

    input_fd = open(argv[1], O_RDONLY);
    if (input_fd < 0) {
        printf("Error opening file \"%s\" for reading: %s\n", argv[1], strerror(errno));
        return 1;
    }

    file_size = lseek(input_fd, 0, SEEK_END);
    close(input_fd);

    num_clusters = file_size / CLUSTER_SIZE;
    snprintf(tasks_mq_name, 16, "/tasks_%s", getlogin());
    tasks_mq_name[15] = '\0';
    snprintf(results_mq_name, 18, "/results_%s", getlogin());
    results_mq_name[17] = '\0';

    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();
        if (pid == -1)
            exit(1);
        else if (pid == 0) {
            execlp("./worker", "./worker", argv[1], NULL);
            printf("execlp failed: %s\n", strerror(errno));
        }
    }

    for (int i = 0; i < NUM_THREADS; i++)
        pthread_create(&(processor[i]), NULL, process_result, NULL);
    
    // Phase 1
    
    // Phase 2

    // Phase 3

    return 0;
};
