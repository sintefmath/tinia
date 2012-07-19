/** [headers] */
#include "tutorial5_job.hpp"
#include "tinia/qtcontroller/QTController.hpp"
#include <QFile>
#include <string>
#include <exception>
/** [headers] */

/** [main] */
int main(int argc, char** argv) {
    /** [job] */
    tinia::tutorial5::TextureDrawer job;
    /** [job] */

    /** [controller] */
    tinia::qtcontroller::QTController controller;
    /** [controller] */
    //Q_INIT_RESOURCE( tutorial5 );
    QFile viewerSourceLoc( ":/tutorial5.js" );
    if( !viewerSourceLoc.open( (QIODevice::ReadOnly | QIODevice::Text) ) ) {
        std::string err = std::string(viewerSourceLoc.errorString().toAscii() );
        throw std::runtime_error( "Could not open file in QRC, aborting."  + err);
    }

    std::string viewerSource ( QString(viewerSourceLoc.readAll()).toAscii() );
    controller.addScript( viewerSource );

    /** [jobtocontroller] */
    controller.setJob(&job);
    /** [jobtocontroller] */

    /** [run]*/
    return controller.run(argc, argv);
    /** [run]*/
}
/** [main] */
