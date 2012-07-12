/** [headers] */
#include "Tutorial1_Job.hpp"
#include "tinia/qtcontroller/QTController.hpp"
/** [headers] */

/** [main] */
int main(int argc, char** argv) {
    /** [job] */
    tinia::tutorial::Tutorial1Job job;
    /** [job] */

    /** [controller] */
    tinia::qtcontroller::QTController controller;
    /** [controller] */

    /** [jobtocontroller] */
    controller.setJob(&job);
    /** [jobtocontroller] */

    /** [run]*/
    return controller.run(argc, argv);
    /** [run]*/
}
/** [main] */
