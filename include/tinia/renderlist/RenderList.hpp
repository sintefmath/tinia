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
#include <string>
#include <vector>

// forward declarations
namespace tinia {
namespace renderlist {
    typedef unsigned int Id;
    typedef unsigned int Revision;
    class Item;
    class Action;
    class DataBase;
    class Buffer;
    class Image;
    class Shader;

    class Draw;
    class SetFramebuffer;
    class SetInputs;
    class SetLight;
    class SetLocalCoordSys;
    class SetShader;
    class SetUniforms;
    class SetViewCoordSys;
    class SetFramebufferState;
    class SetPixelState;
    class SetRasterState;

    enum Encoding {
        ENCODING_PLAIN,
        ENCODING_JSON
    };

    enum ElementType {
        ELEMENT_INT,
        ELEMENT_FLOAT
    };

    enum UniformSemantic {
        SEMANTIC_MODELVIEW_PROJECTION_MATRIX,
        SEMANTIC_NORMAL_MATRIX
    };

    enum LightType {
        LIGHT_AMBIENT,
        LIGHT_DIRECTIONAL,
        LIGHT_POINT,
        LIGHT_SPOT
    };

    enum UniformType {
        UNIFORM_SEMANTIC,
        UNIFORM_INT,
        UNIFORM_FLOAT,
        UNIFORM_FLOAT2,
        UNIFORM_FLOAT3,
        UNIFORM_FLOAT4,
        UNIFORM_FLOAT3X3,
        UNIFORM_FLOAT4X4
    };

    enum DepthFunc {
        DEPTH_FUNC_NEVER,
        DEPTH_FUNC_LESS,
        DEPTH_FUNC_EQUAL,
        DEPTH_FUNC_LEQUAL,
        DEPTH_FUNC_GREATER,
        DEPTH_FUNC_NOTEQUAL,
        DEPTH_FUNC_GEQUAL,
        DEPTH_FUNC_ALWAYS
    };

    enum CullFace {
        CULL_FRONT,
        CULL_BACK,
        CULL_FRONT_AND_BACK
    };

    enum PolygonMode {
        POLYGON_MODE_POINT,
        POLYGON_MODE_LINE,
        POLYGON_MODE_FILL
    };

    enum BlendFunc {
        BLEND_FUNC_ZERO,
        BLEND_FUNC_ONE,
        BLEND_FUNC_SRC_COLOR,
        BLEND_FUNC_ONE_MINUS_SRC_COLOR,
        BLEND_FUNC_DST_COLOR,
        BLEND_FUNC_ONE_MINUS_DST_COLOR,
        BLEND_FUNC_SRC_ALPHA,
        BLEND_FUNC_ONE_MINUS_SRC_ALPHA,
        BLEND_FUNC_DST_ALPHA,
        BLEND_FUNC_ONE_MINUS_DST_ALPHA,
        BLEND_FUNC_CONSTANT_COLOR,
        BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR,
        BLEND_FUNC_CONSTANT_ALPHA,
        BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA,
        BLEND_FUNC_SRC_ALPHA_SATURATE
    };

    enum PrimitiveType {
        PRIMITIVE_POINTS,
        PRIMITIVE_LINES,
        PRIMITIVE_LINE_STRIP,
        PRIMITIVE_LINE_LOOP,
        PRIMITIVE_TRIANGLES,
        PRIMITIVE_TRIANGLE_STRIP,
        PRIMITIVE_TRIANGLE_FAN,
        PRIMITIVE_QUADS,
        PRIMITIVE_QUAD_STRIP
    };


    /*




class SetPixelOps : public Action
{
public:

protected:
    bool    m_depth_test;
    bool    m_blend;
};




class SetFrameBuffer : public Action
{
public:
protected:
};


class SetLight : public Action
{
public:
protected:
    enum Type { TYPE_SPOT };
    Type    m_type;
    float   m_color[3];
    float   m_attenuation[3];
    float   m_falloff[2];
    float   m_from_world[16];
    float   m_to_world[16];
};


*/


namespace Util {


/** Creates items and actions necessary to render a unit cube.
  *
  * \param db       The database to modify
  * \param actions  The sequence of actions required to render the object.
  */
void
createUnitCube( DataBase db, std::vector<Action*>& actions );


} // of namespace renderlist::Util
} // of namespace renderlist
} // of namespace tinia



