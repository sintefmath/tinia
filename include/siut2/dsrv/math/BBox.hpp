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

