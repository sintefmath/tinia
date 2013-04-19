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

#pragma once
#include "tinia/model/ExposedModel.hpp"
// QT's moc doesn't like BOOST_JOIN ( can be removed in QT 5.0 we think)
#ifndef Q_MOC_RUN 
#include <boost/shared_ptr.hpp>
#endif

namespace tinia {
namespace jobcontroller {

/** Job is responsible for the basic interactions between the Controller and
 * the underlying program logic in a Tinia program.
 *
 * Users will typically reimplement the init method to connect it to their program
 * code.
 */
class Job
{
public:
   Job( );

   virtual ~Job(  ) {};

   virtual bool init();

   virtual
   void
   cleanup();

   virtual
   bool
   periodic();

   void quit();

   virtual boost::shared_ptr<model::ExposedModel> getExposedModel();

protected:
   boost::shared_ptr<model::ExposedModel> m_model;
};
}
}

