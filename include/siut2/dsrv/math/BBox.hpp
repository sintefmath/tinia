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

#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp> //for lerp

namespace siut2
{
  namespace dsrv
  {

    struct BBox
    {
      glm::vec3 m_min;
      glm::vec3 m_max;
      bool m_empty;

      /** Constructs an empty bounding box.
       *  With member vectors set to 0 
       */
      BBox()
        : m_min(glm::vec3(0)),
          m_max(glm::vec3(0)),
          m_empty(true)
      {
	;
      }

      /** Constructs a bounding box from the vectors specified.
       */
      BBox(glm::vec3 input_min, glm::vec3 input_max)
	:m_min(input_min),
	 m_max(input_max),
	 m_empty(false)
      {
	;
      }     
      
      const glm::vec3 getOrigin() const
      {
	return glm::lerp(m_min, m_max, 0.5f);
      }

      float getRadius() const
      {
	return fabs(m_max.x - m_min.x) > fabs(m_max.y - m_min.y) ? fabs(m_max.x - m_min.x)/2.f : fabs(m_max.y - m_min.y)/2.f;
      }
      
      const glm::vec3& getMin() const
      {
	return m_min;
      }
      
      const glm::vec3& getMax() const
      {
	return m_max;
      }

      void setMin(glm::vec3 const& input_min)
      {
	m_min = input_min;
      }

      void setMax(glm::vec3 const& input_max)
      {
	m_max = input_max;
      }

      const bool& isEmpty() const
      {
	return m_empty;
      }

    };
  }
}

