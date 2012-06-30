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

#include <boost/test/unit_test.hpp>
#include "tinia/model/ExposedModel.hpp"
#include "tinia/model/ExposedModelLock.hpp"
#include "tinia/model/StateListener.hpp"
#include <memory>
using namespace tinia;
namespace {
class ExposedModelListenerFixture : model::StateListener
{
public:
   ExposedModelListenerFixture() : model(new model::ExposedModel()),
      hasSeenEvent(false)
   {
      model->addStateListener(this);
   }

   ~ExposedModelListenerFixture() { model->removeStateListener(this); }

   void stateElementModified(model::StateElement *stateElement)
   {
      hasSeenEvent = true;
   }

   std::shared_ptr<model::ExposedModel> model;
   bool hasSeenEvent;

};
}

BOOST_FIXTURE_TEST_CASE(SimpleLockTest, ExposedModelListenerFixture)
{
   {// Scoped lock
      BOOST_CHECK(!hasSeenEvent);
      model::ExposedModelLock modelLock(model);
      BOOST_CHECK(!hasSeenEvent);
      model->addElement("AValueKey", "AValue");
      BOOST_CHECK(!hasSeenEvent);
      model->updateElement("AValueKey", "ANewValue");
      BOOST_CHECK(!hasSeenEvent);
   }
   BOOST_CHECK(hasSeenEvent);

}

