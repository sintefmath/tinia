#include "TestJob.hpp"
#include "tinia/trell/IPCGLJobObserver.hpp"
int main(int argc, char** argv)
{
  tinia::Trell::IPCGLJobObserver *trellObserver = new tinia::Trell::IPCGLJobObserver();
   TestJob *testJob = new TestJob();
   trellObserver->setJob(testJob);

   trellObserver->run(argc, argv);
   exit(EXIT_SUCCESS);
}
