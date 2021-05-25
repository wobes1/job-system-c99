
#include <jobs.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>

#define NUM_WORKERS_THREADS 8
#define TOTAL_JOB_COUNT (64 * 1024)
static const int MAX_JOBS_PER_THREAD = (TOTAL_JOB_COUNT / NUM_WORKERS_THREADS);
static int_fast32_t finished_job_count = 0;

static void empty_job(job *job, const void *data) {
  (void)job;
  (void)data;
  __atomic_fetch_add(&finished_job_count, 1, __ATOMIC_SEQ_CST);
}

static void *empty_worker_test(void *ctx) {
  int worker_id = worker_init(ctx);

  const int job_count = ((context *)ctx)->max_jobs_per_thread;
  int jobId = (worker_id << 16) | 0;
  job *root = job_create(empty_job, NULL, &jobId, sizeof(int));
  job_enqueue(root);
  for (int job_id = 1; job_id < job_count; job_id += 1) {
    int jobId = (worker_id << 16) | job_id;
    job *job = job_create(empty_job, root, &jobId, sizeof(int));
    int error = job_enqueue(job);
    assert(!error);
  }
  job_wait_for(root);
}

int main(void) {
  {
    context *context = context_create(NUM_WORKERS_THREADS, MAX_JOBS_PER_THREAD);

    clock_t start = clock();
    pthread_t workers[NUM_WORKERS_THREADS];

    for (int thread_id = 0; thread_id < NUM_WORKERS_THREADS; thread_id++) {
      pthread_create(&workers[thread_id], NULL, empty_worker_test, context);
    }

    for (int thread_id = 0; thread_id < NUM_WORKERS_THREADS; thread_id++) {

      pthread_join(workers[thread_id], NULL);
    }

    clock_t stop = clock();
    double elapsed = (double)(stop - start) * 1000.0 / CLOCKS_PER_SEC;

    printf("%d jobs complete in %.3fms\n", (int)finished_job_count, elapsed);
    context_destroy(context);
  }

  getchar();
}