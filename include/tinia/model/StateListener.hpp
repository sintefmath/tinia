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
#include "tinia/model/StateElement.hpp"
#include <memory>


namespace tinia {
namespace model {
/**
  The basic listener for any changes in the State of the model. I.e. this
  listener is notified whenever specific properties are changed.
  */
class StateListener
{
public:

   /**
     This is called whenever a specific element is changed.
     \param stateElement pointer to the StateElement being changed. The sender is
     responsible for the memory of this pointer
     */
   virtual void stateElementModified(model::StateElement * stateElement) = 0;
};
}
}

