#include "TestJob.hpp"
#include "tinia/qtobserver/QTObserver.hpp"
int main(int argc, char** argv)
{
   tinia::qtobserver::QTObserver *qtObserver = new tinia::qtobserver::QTObserver();
   TestJob *testJob = new TestJob();
   qtObserver->setJob(testJob);

   qtObserver->run(argc, argv);

}
