// #include <jobs.h>

// Job *CreateJob(JobFunction function) {

//   Job newJob = {0};
//   Job *job = &newJob;
//   job->function = function;
//   job->parent = NULL;
//   job->unfinishedJobs = 1;

//   return job;
// }

// Job *CreateJobAsChild(Job *parent, JobFunction function) {
//   (void)__sync_fetch_and_add(&parent->unfinishedJobs, 1);

//   Job newJob = {0};
//   Job *job = &newJob;
//   job->function = function;
//   job->parent = parent;
//   job->unfinishedJobs = 1;

//   return job;
// }