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

#include "CubeJob.hpp"
#include "tinia/trell/FRVGLJobController.hpp"
#include "tinia/trell/FRVQtController.hpp"
#include <QCoreApplication>


int main(int argc, char** argv)
{
    using namespace tinia::example;
    tinia::trell::FRVGLJobController *trellController = new tinia::trell::FRVGLJobController();

    CubeJob *testJob = new CubeJob();
    trellController->setJob(testJob);

    trellController->run(argc, argv);
    tinia::trell::FRVQtController* server  = new tinia::trell::FRVQtController( trellController );

    QCoreApplication blah(argc, argv );
    QObject::connect( server, &tinia::trell::FRVQtController::closed, &blah, &QCoreApplication::quit );
    auto retVal = blah.exec();

    exit( retVal );
}
