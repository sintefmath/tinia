#include "MyJob.hpp"
#include <tinia/trell/IPCGLJobController.hpp>

int main(int argc, char** argv) {
  MyJob job;
  tinia::trell::IPCGLJobController controller;
  controller.setJob(&job);
  controller.run(argc, argv);
}
