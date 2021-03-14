#include <jobs.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static size_t BufferSize(int capacity) { return capacity * sizeof(job *); }

work_stealing_queue *work_stealing_queue_init(int capacity, void *buffer,
                                              size_t bufferSize) {
  work_stealing_queue *queue = malloc(sizeof(work_stealing_queue));
  if ((capacity & (capacity - 1)) != 0) {
    perror("Capacity must be a power of 2");
    return NULL;
  }

  size_t minBufferSize = BufferSize(capacity);
  if (bufferSize < minBufferSize) {
    perror("Inadequate buffer size");
    return NULL;
  }
  uint8_t *bufferNext = (uint8_t *)buffer;
  queue->entries = (job **)bufferNext;
  bufferNext += capacity * sizeof(job *);
  assert(bufferNext - (uint8_t *)buffer == (intptr_t)minBufferSize);

  for (int iEntry = 0; iEntry < capacity; iEntry += 1) {
    queue->entries[iEntry] = NULL;
  }

  queue->top = 0;
  queue->bottom = 0;
  queue->capacity = capacity;

  return queue;
}

int work_stealing_queue_push(work_stealing_queue *queue, job *job) {
  // TODO: assert that this is only ever called by the owning thread
  uint64_t jobIndex = queue->bottom;
  queue->entries[jobIndex & (queue->capacity - 1)] = job;

  // Ensure the job is written before the m_bottom increment is published.
  // A StoreStore memory barrier would also be necessary on platforms with a
  // weak memory model.
  __sync_synchronize();

  queue->bottom = jobIndex + 1;
  return 0;
}

job *work_stealing_queue_pop(work_stealing_queue *queue) {
  // TODO: assert that this is only ever called by the owning thread
  uint64_t bottom = queue->bottom - 1;
  queue->bottom = bottom;

  // Make sure m_bottom is published before reading top.
  // Requires a full StoreLoad memory barrier, even on x86/64.
  __sync_synchronize();

  uint64_t top = queue->top;
  if (top <= bottom) {
    job *job = queue->entries[bottom & (queue->capacity - 1)];
    if (top != bottom) {
      // still >0 jobs left in the queue
      return job;
    } else {

      // popping the last element in the queue
      if (!__atomic_compare_exchange_n(&queue->top, &top, top, false, 0, 0)) {
        // failed race against Steal()
        job = NULL;
      }
      queue->bottom = top + 1;
      return job;
    }
  } else {
    // queue already empty
    queue->bottom = top;
    return NULL;
  }
}

job *work_stealing_queue_steal(work_stealing_queue *queue) {
  // TODO: assert that this is never called by the owning thread
  uint64_t top = queue->top;

  // Ensure top is always read before bottom.
  // A LoadLoad memory barrier would also be necessary on platforms with a weak
  // memory model.
  __sync_synchronize();

  uint64_t bottom = queue->bottom;
  if (top < bottom) {
    job *job = queue->entries[top & (queue->capacity - 1)];
    // CAS serves as a compiler barrier as-is.
    if (!__atomic_compare_exchange_n(&queue->top, &top, top + 1, false, 0, 0)) {
      // concurrent Steal()/Pop() got this entry first.
      return NULL;
    }
    queue->entries[top & (queue->capacity - 1)] = NULL;
    return job;
  } else {
    return NULL; // queue empty
  }
}