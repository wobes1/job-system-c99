#include "api_types.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct work_stealing_queue {
  job **entries;
  uint64_t top;
  uint64_t bottom;
  int capacity;
} work_stealing_queue;

static size_t buffer_size_get(int capacity);
extern work_stealing_queue *work_stealing_queue_init(int capacity, void *buffer,
                                                     size_t bufferSize);
extern int work_stealing_queue_push(work_stealing_queue *queue, job *job);
extern job *work_stealing_queue_pop(work_stealing_queue *queue);

extern job *work_stealing_queue_steal(work_stealing_queue *queue);