#include "TestJob.hpp"
#include "qtobserver/QTObserver.hpp"
int main(int argc, char** argv)
{
   qtobserver::QTObserver *qtObserver = new qtobserver::QTObserver();
   TestJob *testJob = new TestJob();
   qtObserver->setJob(testJob);

   qtObserver->run(argc, argv);

}
