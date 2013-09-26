/* Copyright STIFTELSEN SINTEF 2012
 *
 * This file is part of the Tinia Framework.
 *
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

/** [headers] */
#include "tutorial5_job.hpp"
#include <Qt>
#include "tinia/trell/IPCGLJobController.hpp"
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
    tinia::trell::IPCGLJobController controller;
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
