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
#include <sys/wait.h>
#include <stdint.h>

#include "common.h"
#include "classify.h"
#include "intqueue.h"
    
// Global variables used by both main and process_result
mqd_t tasks_mqd, results_mqd; // message queue descriptors
struct mq_attr attributes;
struct intqueue headerq;
int classification_fd;
int num_clusters;           
int clusters_processed = 0;  // counter used to break out of process loop
static pthread_mutex_t counter_lock = PTHREAD_MUTEX_INITIALIZER;
int num_clusters;
struct intqueue headerq;


void *process_result(void *arg) {
    char recv_buffer[MESSAGE_SIZE_MAX]; // receiving buffer
    struct result *new_result; 

    // Initialize an empty queue to store the clusters that have file headers.
    initqueue(&headerq);

    // Keep receiving from the results queue until there are no clusters left to process
    while(clusters_processed != num_clusters) {

        clusters_processed++;

        // Receive from the results message queue
        if (mq_receive(results_mqd, recv_buffer, attributes.mq_msgsize, NULL) < 0) {
            printf("Error receiving message from results queue: %s\n", strerror(errno));
            return NULL;
        }   
        // Start of critical section
        pthread_mutex_lock(&counter_lock);

        // Cast contents of the receiving buffer into the results struct
        new_result = (struct result *)recv_buffer;

        // seek to location in classification file specified by res_cluster_number
        lseek(classification_fd, new_result->res_cluster_number, SEEK_SET);

        // write the classification type to the location specified by result
        write(classification_fd, &new_result->res_cluster_type, 1);

        // If an HTML or JPG header is found, save the value in the queue
        if (new_result->res_cluster_type & TYPE_HTML_HEADER) { 
            enqueue(&headerq, new_result->res_cluster_number);
        } else if (new_result->res_cluster_type & TYPE_JPG_HEADER) {
            enqueue(&headerq, new_result->res_cluster_number);
        }
        // End of critical section
        pthread_mutex_unlock(&counter_lock);

    }
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
    pthread_t processor[NUM_THREADS];

    // Initialize the message queue attributes
    attributes.mq_flags = 0;
    attributes.mq_maxmsg = 1000;
    attributes.mq_msgsize = MESSAGE_SIZE_MAX;
    attributes.mq_curmsgs = 0;

    // The user must supply a data file to be used as input
    if (argc != 2) {
        printf("Usage: %s data_file\n", argv[0]);
        return 1;
    }

    // Open input file for reading, exiting with error if open() fails
    input_fd = open(argv[1], O_RDONLY);
    if (input_fd < 0) {
        printf("Error opening file \"%s\" for reading: %s\n", argv[1], strerror(errno));
        return 1;
    }

    // Open classification file for writing. Create file if it does not
    // exist. Exit with error if open() fails.
    classification_fd = open(CLASSIFICATION_FILE, O_RDWR | O_CREAT, 0600);
    if (classification_fd < 0) {
        printf("Error creating file \"%s\": %s\n", CLASSIFICATION_FILE, strerror(errno));
        return 1;
    }

    // Determine the file size of the input file
    file_size = lseek(input_fd, 0, SEEK_END);
    close(input_fd);

    // Calculate how many clusters are present
    num_clusters = file_size / CLUSTER_SIZE;

    // Generate the names for the tasks and results queue
    snprintf(tasks_mq_name, 16, "/tasks_%s", getlogin());
    tasks_mq_name[15] = '\0';
    snprintf(results_mq_name, 18, "/results_%s", getlogin());
    results_mq_name[17] = '\0';

    // Create the child processes
    for (int i = 0; i < NUM_PROCESSES; i++) {
        pid = fork();
        if (pid == -1)
            exit(1);
        else if (pid == 0) {
            execlp("./worker", "./worker", argv[1], NULL);
            printf("execlp failed: %s\n", strerror(errno));
        }
    }

    // Opens the task messages and results queue in blocking mode
    if ((tasks_mqd = mq_open(tasks_mq_name, O_RDWR | O_CREAT, 0600, &attributes)) < 0) {
        printf("Error opening message queue %s: %s\n", tasks_mq_name, strerror(errno));
        return 1;
    }
    if ((results_mqd = mq_open(results_mq_name, O_RDWR | O_CREAT, 0600, &attributes)) < 0) {
        printf("Error opening message queue %s: %s\n", results_mq_name, strerror(errno));
        return 1;
    }

    // Create NUM_THREADS threads 
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&(processor[i]), NULL, process_result, NULL);
    }
        
    // Phase 1: Generate classification tasks and process results

    new_task.task_type = TASK_CLASSIFY;
    new_task.task_cluster = 0;

    // Send tasks for each cluster of the input file
    for (int i = 0; i < num_clusters; i++) {
        if (mq_send(tasks_mqd, (const char *) &new_task, sizeof(new_task), 0) < 0) {
            printf("Error sending to tasks queue: %s\n", strerror(errno));
            return 1;
        }
        new_task.task_cluster++;
    }

    // Wait then terminate NUM_THREADS threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(processor[i], NULL);
    }

    // Phase 2: Generate map tasks 

    int cluster_number;

    // While the queue is not empty
    while(isempty(&headerq) != 1) {

        // Get the next cluster number from the queue
        cluster_number = dequeue(&headerq);

        unsigned char cluster_type;

        uint32_t joffset = 0000;
        uint32_t hoffset = 0000;
        char filename[13];

        // Seek to the specified cluster in the classification file
        lseek(classification_fd, cluster_number, SEEK_SET);
        read(classification_fd, &cluster_type, 1);

        // If the cluster is a JPG Header:
        if (cluster_type & TYPE_JPG_HEADER) {
            // Generate a new JPG file name
            printf("JHEADER: %x\n", cluster_type);
            snprintf(filename, sizeof(filename), "file%04d.jpg", joffset);
            joffset++;
        } else {
            // Generate a new HTML file name
            printf("HHEADER: %x\n", cluster_type);
            snprintf(filename, sizeof(filename), "file%04d.htm", hoffset);
            hoffset++;
        }

        // Generate the task message for the cluster, and send it to the tasks queue
        new_task.task_type = TASK_MAP;
        strncpy(&new_task.task_filename, &filename, sizeof(filename));
        if (mq_send(tasks_mqd, (const char *) &new_task, sizeof(new_task), 0) < 0) {
            printf("Error sending to tasks queue: %s\n", strerror(errno));
            return 1;   
        } 
    }

    // Phase 3: Generate termination tasks

    new_task.task_type = TASK_TERMINATE;

    // Send NUM_PROCESSES tasks to the tasks message queue
    for (int i = 0; i < NUM_PROCESSES; i++) {
        if (mq_send(tasks_mqd, (const char *) &new_task, sizeof(new_task), 0) < 0) {
            printf("Error sending to tasks queue: %s\n", strerror(errno));
            return 1;
        }
    }

    // Wait for all children to terminate
    for (int i = 0; i < NUM_PROCESSES; i++) {
        wait(NULL);
    }

    // Close any open mqds
    mq_close(tasks_mqd);
    mq_close(results_mqd);

    // Unlink mqueues
    mq_unlink(tasks_mq_name);
    mq_unlink(results_mq_name);

    // Close files
    close(classification_fd);
    close(input_fd);

    // Terminates itself
    return 0;
};
