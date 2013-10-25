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

#define BOOST_TEST_MODULE qtcontrollerTest
#include <boost/test/unit_test.hpp>
#include <QApplication>

/*
namespace tinia {
namespace test {
*/

struct SetupQtApplication {
    QCoreApplication app;

    static char* argv;
    static int argc;

    SetupQtApplication()
        : app(argc, &argv)
    {
    Q_INIT_RESOURCE(resources);
    }
};


#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6 // Only available on 4.6 and newer
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wwrite-strings"
#endif
#endif

char* SetupQtApplication::argv = "bleh";
int SetupQtApplication::argc = 1;

#ifdef __GNUC__
#if __GNUC__ >= 4 && __GNUC_MINOR__ >= 6 // Only available on 4.6 and newer
#pragma GCC diagnostic pop
#endif
#endif
/*}}*/

//using namespace tinia::test;
BOOST_GLOBAL_FIXTURE( SetupQtApplication );

