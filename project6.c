/* CS 361 project6.c
  Team: 07
  Names: Nic Plybon & Adrien Ponce
  Honor Code Statement: This code complies with the JMU Honor Code.
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

mqd_t tasks_mqd, results_mqd; // message queue descriptors

    

void *process_result(void *arg) {
    return NULL;

}

int main(int argc, char *argv[])
{
    int input_fd;
    pid_t pid;
    off_t file_size;
    char tasks_mq_name[16];
    char results_mq_name[18];
    struct task new_task;
    int num_clusters;
    pthread_t processor[NUM_THREADS];
    struct mq_attr attributes;
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = 1000;
    attributes.mq_msgsize = MESSAGE_SIZE_MAX;
    attributes.mq_curmsgs = 0;

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



    if ((tasks_mqd = mq_open(tasks_mq_name, O_RDWR | O_CREAT, 0600, &attributes)) < 0) {
        printf("Error opening message queue %s: %s\n", tasks_mq_name, strerror(errno));
        return 1;
    }
    if ((results_mqd = mq_open(results_mq_name, O_RDWR | O_CREAT, 0600, &attributes)) < 0) {
        printf("Error opening message queue %s: %s\n", results_mq_name, strerror(errno));
        return 1;
    }

    for (int i = 0; i < NUM_THREADS; i++) 
        pthread_create(&(processor[i]), NULL, process_result, NULL);
    
    // Phase 1
    
    // Phase 2

    // Phase 3
    struct task terminate;
    terminate.task_type = TASK_TERMINATE;
    for (int i = 0; i < NUM_PROCESSES; i++) {
        // send to tasks queue
        if (mq_send(tasks_mqd, (const char *) &terminate, sizeof(terminate), 0) < 0) {
            printf("Error sending to tasks queue: %s\n", strerror(errno));
            return 1;
        }
    }
    //wait for children to terminate
    wait(NULL);
    //close any open mqds
    mq_close(tasks_mqd);
    mq_close(results_mqd);
    //unlink mqueues
    mq_unlink(tasks_mq_name);
    mq_unlink(results_mq_name);
    //terminates itself
    return 0;
};
