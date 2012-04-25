#include <sstream>
#include <typeinfo>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Action.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetFramebufferState.hpp>
#include <tinia/renderlist/SetFramebuffer.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetLight.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/SetPixelState.hpp>
#include <tinia/renderlist/SetRasterState.hpp>

#include <tinia/renderlist/XMLWriter.hpp>

namespace tinia {
namespace renderlist {

template<typename TYPE>
void
encodeArray( std::stringstream& o, const Encoding encoding, const TYPE* data, const size_t count )
{
    switch( encoding ) {
    case ENCODING_PLAIN:
        for(size_t i=0; i<count; i++ ) {
            o << data[i] << ((i+1<count)?" ":"");
            if( (i!=0) && (i%16==0) ) {
                o << std::endl;
            }
        }
        break;
    case ENCODING_JSON:
        o << "[";
        for(size_t i=0; i<count; i++ ) {
            o << data[i] << ((i+1<count)?", ":"");
            if( (i!=0) && (i%16==0) ) {
                o << std::endl;
            }
        }
        o << "]";
        break;
    }
}

void
encodePrimitiveType( std::stringstream& o, const PrimitiveType primitive_type )
{
    switch( primitive_type ) {
    case PRIMITIVE_POINTS:          o << "GL_POINTS"; break;
    case PRIMITIVE_LINES:           o << "GL_LINES"; break;
    case PRIMITIVE_LINE_STRIP:      o << "GL_LINE_STRIP"; break;
    case PRIMITIVE_LINE_LOOP:       o << "GL_LINE_LOOOP"; break;
    case PRIMITIVE_TRIANGLES:       o << "GL_TRIANGLES"; break;
    case PRIMITIVE_TRIANGLE_STRIP:  o << "GL_TRIANGLE_STRIP"; break;
    case PRIMITIVE_TRIANGLE_FAN:    o << "GL_TRIANGLE_FAN"; break;
    case PRIMITIVE_QUADS:           o << "GL_QUADS"; break;
    case PRIMITIVE_QUAD_STRIP:      o << "GL_QUAD_STRIP"; break;
    }
}


void
encodeDepthFunc( std::stringstream& o, const DepthFunc func )
{
    switch( func ) {
    case DEPTH_FUNC_NEVER:      o << "GL_NEVER"; break;
    case DEPTH_FUNC_LESS:       o << "GL_LESS"; break;
    case DEPTH_FUNC_EQUAL:      o << "GL_EQUAL"; break;
    case DEPTH_FUNC_LEQUAL:     o << "GL_LEQUAL"; break;
    case DEPTH_FUNC_GREATER:    o << "GL_GREATER"; break;
    case DEPTH_FUNC_NOTEQUAL:   o << "GL_NOTEQUAL"; break;
    case DEPTH_FUNC_GEQUAL:     o << "GL_GEQUAL"; break;
    case DEPTH_FUNC_ALWAYS:     o << "GL_ALWAYS"; break;
    }
}

void
encodeBlendFunc( std::stringstream& o, const BlendFunc func )
{
    switch( func ) {
    case BLEND_FUNC_ZERO:                       o << "GL_ZERO"; break;
    case BLEND_FUNC_ONE:                        o << "GL_ONE"; break;
    case BLEND_FUNC_SRC_COLOR:                  o << "GL_SRC_COLOR"; break;
    case BLEND_FUNC_ONE_MINUS_SRC_COLOR:        o << "GL_ONE_MINUS_SRC_COLOR"; break;
    case BLEND_FUNC_DST_COLOR:                  o << "GL_DST_COLOR"; break;
    case BLEND_FUNC_ONE_MINUS_DST_COLOR:        o << "GL_ONE_MINUS_DST_COLOR"; break;
    case BLEND_FUNC_SRC_ALPHA:                  o << "GL_SRC_ALPHA"; break;
    case BLEND_FUNC_ONE_MINUS_SRC_ALPHA:        o << "GL_ONE_MINUS_SRC_ALPHA"; break;
    case BLEND_FUNC_DST_ALPHA:                  o << "GL_DST_ALPHA"; break;
    case BLEND_FUNC_ONE_MINUS_DST_ALPHA:        o << "GL_ONE_MINUS_DST_ALPHA"; break;
    case BLEND_FUNC_CONSTANT_COLOR:             o << "GL_CONSTANT_COLOR"; break;
    case BLEND_FUNC_ONE_MINUS_CONSTANT_COLOR:   o << "GL_ONE_MIMUS_CONSTANT_COLOR"; break;
    case BLEND_FUNC_CONSTANT_ALPHA:             o << "GL_CONSTANT_ALPHA"; break;
    case BLEND_FUNC_ONE_MINUS_CONSTANT_ALPHA:   o << "GL_ONE_MINUS_CONSTANT_ALPHA"; break;
    case BLEND_FUNC_SRC_ALPHA_SATURATE:         o << "GL_SRC_ALPHA_SATURATE"; break;
    }
}

std::string
getUpdateXML( const DataBase* database, const Encoding encoding, const Revision has_revision )
{
    std::list<renderlist::Buffer*> buffers;
    std::list<renderlist::Image*>  images;
    std::list<renderlist::Shader*> shaders;
    std::list<renderlist::Action*> actions;
    std::list<renderlist::Action*> draworder;
    std::list<renderlist::Item*>   keep;
    bool new_draworder;
    bool needs_pruning;
    Revision revision = database->changes( buffers,
                                           images,
                                           shaders,
                                           actions,
                                           new_draworder,
                                           draworder,
                                           needs_pruning,
                                           keep,
                                           has_revision );
    std::stringstream o;
    o << "<?xml version=\"1.0\"?>" << std::endl;
    o << "<renderList" <<
         " xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" <<
         " xmlns=\"http://cloudviz.sintef.no/renderlist/1.0\"" <<
         " xsi:schemaLocation=\"http://cloudviz.sintef.no/renderlist/1.0 renderlist.xsd\"" <<
         " from=\"" << has_revision << "\"" <<
         " to=\"" << revision << "\"" <<
         ">" << std::endl;
    if( revision != has_revision ) {
        o << "  <updateItems>" << std::endl;
        if( !buffers.empty() ) {
            o << "    <buffers>" << std::endl;
            for( auto it=buffers.begin(); it!=buffers.end(); ++it ) {
                const Buffer* b = *it;
                switch( b->type() ) {
                case ELEMENT_INT:
                    o << "      <update id=\"" << b->id() << "\" type=\"int\" count=\"" << b->count() << "\">" << std::endl;
                    encodeArray<int>( o, encoding, b->intData(), b->count() );
                    o << std::endl << "      </update>" << std::endl;
                    break;

                case ELEMENT_FLOAT:
                    o << "      <update id=\"" << b->id() << "\" type=\"float\" count=\"" << b->count() << "\">" << std::endl;
                    encodeArray<float>( o, encoding, b->floatData(), b->count() );
                    o << std::endl << "      </update>" << std::endl;
                    break;
                }
            }
            o << "    </buffers>" << std::endl;
        }
        if( !images.empty() ) {
            o << "    <images>" << std::endl;
            o << "    </images>" << std::endl;
        }
        if( !shaders.empty() ) {
            o << "    <shaders>" << std::endl;
            for( auto it=shaders.begin(); it!=shaders.end(); ++it ) {
                const Shader* s = *it;
                const std::string& vs = s->vertexStage();
                const std::string& tc = s->tessCtrlStage();
                const std::string& te = s->tessEvalStage();
                const std::string& gs = s->geometryStage();
                const std::string& fs = s->fragmentStage();

                o << "      <update id=\"" << s->id() << "\">" << std::endl;
                if( !vs.empty() ) {
                    o << "        <vertex>" << std::endl << vs << "        </vertex>" << std::endl;
                }
                if( !tc.empty() ) {
                    o << "        <tessControl>" << std::endl << tc << "        </tessControl>" << std::endl;
                }
                if( !te.empty() ) {
                    o << "        <tessEvaluation>" << std::endl << te << "        </tessEvaluation>" << std::endl;
                }
                if( !gs.empty() ) {
                    o << "        <geometry>" << std::endl << gs << "        </geometry>" << std::endl;
                }
                if( !fs.empty() ) {
                    o << "        <fragment>" << std::endl << fs << "        </fragment>" << std::endl;
                }
                o << "      </update>" << std::endl;
            }
            o << "    </shaders>" << std::endl;
        }
        if( !actions.empty() ) {
            o << "    <actions>" << std::endl;
            for(auto it=actions.begin(); it!=actions.end(); ++it ) {

                // --- <draw> --------------------------------------------------
                if( typeid(**it) == typeid(Draw) ) {
                    const Draw* a = static_cast<const Draw*>( *it );
                    o << "      <draw id=\"" << a->id() << "\" mode=\"";
                    encodePrimitiveType( o, a->primitiveType() );
                    o << "\"";
                    if( a->isIndexed() ) {
                        o << " indices=\"" << a->indexBufferId() << "\"";
                    }
                    o << " first=\"" << a->first() << "\" count=\"" << a->count()<< "\" />" << std::endl;
                }
                // --- <setPixelState> -----------------------------------------
                else if( typeid(**it) == typeid(SetPixelState) ) {
                    const SetPixelState* a = static_cast<const SetPixelState*>( *it );
                    o << "      <setPixelState id=\"" << a->id() << "\">" << std::endl;
                    if( a->isDepthTestEnabled() ) {
                        o << "        <depthTest>1</depthTest>"<< std::endl;
                        o << "        <depthFunc>";
                        encodeDepthFunc( o, a->depthFunc() );
                        o << "</depthFunc>" << std::endl;
                    }
                    else {
                        o << "        <depthTest>0</depthTest>" << std::endl;
                    }
                    if( a->isBlendingEnabled() ) {
                        o << "        <blend>1</blend>" << std::endl;
                        o << "        <blendSrcRGB>";
                        encodeBlendFunc( o, a->blendSrcRGB() );
                        o << "</blendSrcRGB>" << std::endl;
                        o << "        <blendDstRGB>";
                        encodeBlendFunc( o, a->blendDstRGB() );
                        o << "</blendDstRGB>" << std::endl;
                        o << "        <blendSrcAlpha>";
                        encodeBlendFunc( o, a->blendSrcAlpha() );
                        o << "</blendSrcAlpha>" << std::endl;
                        o << "        <blendDstAlpha>";
                        encodeBlendFunc( o, a->blendDstAlpha() );
                        o << "</blendDstAlpha>" << std::endl;
                    }
                    else {
                        o << "        <blend>0</blend>" << std::endl;
                    }
                    o << "      </setPixelState>" << std::endl;
                }
                // --- <setRasterState> ----------------------------------------
                else if( typeid(**it) == typeid(SetRasterState) ) {
                    const SetRasterState* a = static_cast<const SetRasterState*>( *it );
                    o << "      <setRasterState id=\"" << a->id() << "\" />" << std::endl;
                }
                // --- <setFramebufferState> -----------------------------------
                else if( typeid(**it) == typeid(SetFramebufferState) ) {
                    const SetFramebufferState* a = static_cast<const SetFramebufferState*>( *it );
                    o << "      <setFramebufferState id=\"" << a->id() << "\">" << std::endl;
                    o << "        <colorWriteMask>";
                    encodeArray( o, encoding, a->colorWriteMask(), 4 );
                    o << "</colorWriteMask>" << std::endl;
                    o << "        <depthWriteMask>" << a->depthWriteMask() << "</depthWriteMask>" << std::endl;
                    o << "      </setFramebufferState>" << std::endl;
                }
                // --- <setFramebuffer> ----------------------------------------
                else if( typeid(**it) == typeid(SetFramebuffer) ) {
                    const SetFramebuffer* a = static_cast<const SetFramebuffer*>( *it );
                    const renderlist::Id i = a->imageId();
                    o << "      <setFramebuffer id=\"" << a->id() << "\" ";
                    if( i != ~0u ) {
                        o << "image=\"" << i << "\" ";
                    }
                    o << "/>" << std::endl;
                }
                // --- <setInputs> ---------------------------------------------
                else if( typeid(**it) == typeid(SetInputs) ) {
                    const SetInputs* a = static_cast<const SetInputs*>( *it );
                    o << "      <setInputs id=\""<< a->id() <<
                         "\" shader=\"" << a->shaderId() << "\">" << std::endl;
                    for( size_t i=0; i<a->count(); i++ ) {
                        o << "        <input symbol=\"" << a->symbol(i) <<
                             "\" buffer=\"" << a->bufferId(i) <<
                             "\" components=\"" << a->components(i) <<
                             "\" offset=\"" << a->offset(i) <<
                             "\" stride=\"" << a->stride(i) <<
                             "\" />" << std::endl;
                    }
                    o << "      </setInputs>" << std::endl;
                }
                // --- <setLight> ----------------------------------------------
                else if( typeid(**it) == typeid(SetLight) ) {
                    const SetLight* a = static_cast<const SetLight*>( *it );
                    o << "      <setLight id=\"" << a->id() << "\" ";
                    o << "index=\"" << a->index() << "\" ";
                    o << "type=\"";
                    switch( a->type() ) {
                    case LIGHT_AMBIENT:     o << "AMBIENT"; break;
                    case LIGHT_DIRECTIONAL: o << "DIRECTIONAL"; break;
                    case LIGHT_POINT:       o << "POINT"; break;
                    case LIGHT_SPOT:        o << "SPOT"; break;
                    }
                    o << "\">" << std::endl;
                    o << "        <color>";
                    encodeArray( o, encoding, a->color(), 4 );
                    o << "</color>" << std::endl;
                    if( a->type() != LIGHT_AMBIENT ) {
                        if( a->type() != LIGHT_DIRECTIONAL ) {
                            o << "        <attenuation>";
                            encodeArray( o, encoding, a->attenuation(), 3 );
                            o << "</attenuation>" << std::endl;
                        }
                        if( a->type() == LIGHT_SPOT ) {
                            o << "        <falloff>";
                            encodeArray( o, encoding, a->falloff(), 2 );
                            o << "</falloff>" << std::endl;
                        }
                        o << "        <fromWorld>";
                        encodeArray( o, encoding, a->fromWorld(), 16 );
                        o << "</fromWorld>" << std::endl;
                        o << "        <toWorld>";
                        encodeArray( o, encoding, a->toWorld(), 16 );
                        o << "</toWorld>" << std::endl;
                    }
                    o << "      </setLight>" << std::endl;
                }
                // --- <setLocalCoordSys> --------------------------------------
                else if( typeid(**it) == typeid(SetLocalCoordSys) ) {
                    const SetLocalCoordSys* a = static_cast<const SetLocalCoordSys*>( *it );
                    o << "      <setLocalCoordSys id=\""<< a->id() << "\">" << std::endl;
                    o << "        <fromWorld>";
                    encodeArray( o, encoding, a->fromWorld(), 16 );
                    o << "</fromWorld>" << std::endl;
                    o << "        <toWorld>";
                    encodeArray( o, encoding, a->toWorld(), 16 );
                    o << "</toWorld>" << std::endl;
                    o << "      </setLocalCoordSys>" << std::endl;
                }
                // --- <setShader> ---------------------------------------------
                else if( typeid(**it) == typeid(SetShader) ) {
                    const SetShader* a = static_cast<const SetShader*>( *it );
                    o << "      <setShader id=\"" << a->id() << "\" shader=\"" << a->shaderId() << "\" />" << std::endl;
                }
                // --- <setUniforms> -------------------------------------------

                else if( typeid(**it) == typeid(SetUniforms) ) {
                    const SetUniforms* a = static_cast<const SetUniforms*>( *it );
                    o << "      <setUniforms id=\"" << a->id() << "\"";
                    o << " shader=\"" << a->shaderId() << "\">" << std::endl;
                    for(size_t i=0; i<a->count(); i++ ) {
                        o << "        <uniform symbol=\"" << a->symbol( i ) << "\"";
                        if( a->type(i) == UNIFORM_SEMANTIC ) {
                            o << " semantic=\"";
                            switch( a->semantic(i) ) {
                            case SEMANTIC_MODELVIEW_PROJECTION_MATRIX: o << "MODELVIEW_PROJECTION_MATRIX"; break;
                            case SEMANTIC_NORMAL_MATRIX: o << "NORMAL_MATRIX"; break;
                            }
                            o << "\"/>" << std::endl;
                        }
                        else {
                            o << ">";
                            switch( a->type(i) ) {
                            case UNIFORM_SEMANTIC: break;
                            case UNIFORM_INT:
                                o << "<int>";
                                encodeArray( o, encoding, a->intData(i), 1 );
                                o << "</int>";
                                break;
                            case UNIFORM_FLOAT:
                                o << "<float>";
                                encodeArray( o, encoding, a->floatData(i), 1 );
                                o << "</float>";
                                break;
                            case UNIFORM_FLOAT2:
                                o << "<float2>";
                                encodeArray( o, encoding, a->floatData(i), 2 );
                                o << "</float2>";
                                break;
                            case UNIFORM_FLOAT3:
                                o << "<float3>";
                                encodeArray( o, encoding, a->floatData(i), 3 );
                                o << "</float3>";
                                break;
                            case UNIFORM_FLOAT4:
                                o << "<float4>";
                                encodeArray( o, encoding, a->floatData(i), 4 );
                                o << "</float4>";
                                break;
                            case UNIFORM_FLOAT3X3:
                                o << "<float3x3>";
                                encodeArray( o, encoding, a->floatData(i), 9 );
                                o << "</float3x3>";
                                break;
                            case UNIFORM_FLOAT4X4:
                                o << "<float4x4>";
                                encodeArray( o, encoding, a->floatData(i), 16 );
                                o << "</float4x4>";
                                break;
                            }
                            o << "</uniform>" << std::endl;
                        }
                    }
                    o << "      </setUniforms>" << std::endl;
                }
                // --- <SetViewCoordSys> ---------------------------------------
                else if( typeid(**it) == typeid(SetViewCoordSys) ) {
                    SetViewCoordSys* a = static_cast<SetViewCoordSys*>( *it );
                    o << "      <SetViewCoordSys id=\"" << a->id() << "\">" << std::endl;
                    o << "        <projection>";
                    encodeArray( o, encoding, a->projection(), 16 );
                    o << "</projection>" << std::endl;
                    o << "        <projectionInverse>";
                    encodeArray( o, encoding, a->projectionInverse(), 16 );
                    o << "</projectionInverse>" << std::endl;
                    o << "        <fromWorld>";
                    encodeArray( o, encoding, a->fromWorld(), 16 );
                    o << "</fromWorld>" << std::endl;
                    o << "        <toWorld>";
                    encodeArray( o, encoding, a->toWorld(), 16 );
                    o << "</toWorld>" << std::endl;
                    o << "      </SetViewCoordSys>" << std::endl;
                }
            }
            o << "    </actions>" << std::endl;
        }
        o << "  </updateItems>" << std::endl;

        if( needs_pruning ) {
            o << "  <pruneItems>" << std::endl;
            for( auto it=keep.begin(); it!=keep.end(); ++it ) {
                o << "    <keep id=\"" << (*it)->id() << "\"/>" << std::endl;
            }
            o << "  </pruneItems>" << std::endl;
        }
        if( new_draworder ) {
            o << "  <drawOrder>" << std::endl;
            for( auto it=draworder.begin(); it!=draworder.end(); ++it ) {
                o << "    <invoke action=\"" << (*it)->id() << "\"/>" << std::endl;
            }
            o << "  </drawOrder>" << std::endl;
        }
    }
    o << "</renderList>" << std::endl;
    return o.str();
}



} // of renderlist
} // of namespace tinia
