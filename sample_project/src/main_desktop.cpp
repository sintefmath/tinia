#include "MyJob.hpp"
#include <tinia/qtcontroller/QTController.hpp>

int main(int argc, char** argv) {
  MyJob job;
  tinia::qtcontroller::QTController controller;
  controller.setJob(&job);
  controller.run(argc, argv);
}
