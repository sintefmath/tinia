#include "TestJob.hpp"
#include "trell/IPCGLJobObserver.hpp"
int main(int argc, char** argv)
{
  Trell::IPCGLJobObserver *trellObserver = new Trell::IPCGLJobObserver();
   TestJob *testJob = new TestJob();
   trellObserver->setJob(testJob);

   trellObserver->run(argc, argv);
   exit(EXIT_SUCCESS);
}
