/* common.h */

#ifndef __CS361_COMMON__
#define __CS361_COMMON__

#define CLUSTER_SIZE 4096

#define TYPE_UNCLASSIFIED 0
#define TYPE_IS_JPG 1
#define TYPE_JPG_HEADER 2
#define TYPE_JPG_FOOTER 4
#define TYPE_IS_HTML 8
#define TYPE_HTML_HEADER 16
#define TYPE_HTML_FOOTER 32
#define TYPE_UNKNOWN 128

#define CLASSIFICATION_FILE "classification"
#define MAP_FILE "map"

#define NUM_PROCESSES 5
#define NUM_THREADS 3

#define MESSAGE_SIZE_MAX 32
#define TASK_CLASSIFY 1
#define TASK_MAP 2
#define TASK_TERMINATE 3

struct result {
    int res_cluster_number;
    unsigned char res_cluster_type;
};

struct task {
    int task_type;
    int task_cluster;
    char task_filename[13];
};

#endif
