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

#include "tinia/qtcontroller/impl/utils.hpp"

namespace tinia {
namespace qtcontroller {
namespace impl {

std::string prettyName(std::string key, boost::shared_ptr<model::ExposedModel> model)
{
   model::StateSchemaElement element = model->getStateSchemaElement(key);
   if(element.emptyAnnotation())
   {
      return key;
   }
   else
   {
       std::map<std::string, std::string> annotation =  element.getAnnotation();
      return annotation["en"];
   }

}

}
}
}
