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
#include <typeinfo>
#include <tinia/renderlist/DataBase.hpp>
#include <tinia/renderlist/Buffer.hpp>
#include <tinia/renderlist/Draw.hpp>
#include <tinia/renderlist/SetViewCoordSys.hpp>
#include <tinia/renderlist/Shader.hpp>
#include <tinia/renderlist/SetShader.hpp>
#include <tinia/renderlist/SetInputs.hpp>
#include <tinia/renderlist/SetUniforms.hpp>
#include <tinia/renderlist/SetLight.hpp>
#include <tinia/renderlist/SetLocalCoordSys.hpp>
#include <tinia/renderlist/SetFramebuffer.hpp>
#include <tinia/renderlist/SetFramebufferState.hpp>
#include <tinia/renderlist/SetPixelState.hpp>
#include <tinia/renderlist/SetRasterState.hpp>
#include <tinia/renderlist/XMLWriter.hpp>

BOOST_AUTO_TEST_SUITE( renderlist )

struct Fixture
{
    Fixture()
    {
        BOOST_MESSAGE( "constructor" );
    }

    ~Fixture()
    {
        BOOST_MESSAGE( "destructor" );
    }

    tinia::renderlist::Revision
    dumpChanges( tinia::renderlist::Revision has_revision )
    {

        std::cerr << "----------------------------\n";
        std::cerr << "has_revision=" << has_revision << ", latest=" << m_db.latest() << "\n";
        std::cout << tinia::renderlist::getUpdateXML( &m_db, tinia::renderlist::ENCODING_JSON,  has_revision );
        std::cerr << "----------------------------\n";
        return m_db.latest();
    }


    tinia::renderlist::DataBase m_db;
};

BOOST_FIXTURE_TEST_CASE( foobar, Fixture )
{

    tinia::renderlist::Revision r = 0;

    float temporaryCubePosArray[] = { 0.f, 0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,
                                       0.f, 0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,
                                       0.f, 0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,
                                       1.f, 0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,
                                       1.f, 0.f,  0.f,  1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,
                                       -1.f, 0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,
                                       0.f, 1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,
                                       0.f, 1.f,  0.f,  0.f,  1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,
                                       0.f, -1.f, 0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f };
    m_db.createBuffer( "cube_pos" )
            ->set( temporaryCubePosArray, sizeof(temporaryCubePosArray) / sizeof(float));

    float temporaryCubeNormArray[] = { -1.f,  1.f, -1.f,  1.f,  1.f, -1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f,
                                       -1.f, -1.f, -1.f, -1.f,  1.f, -1.f, -1.f,  1.f,  1.f,  1.f, -1.f,  1.f,
                                       1.f,  1.f,  1.f,  1.f, -1.f,  1.f, -1.f,  1.f,  1.f, -1.f, -1.f,  1.f,
                                       1.f,  1.f, -1.f,  1.f,  1.f,  1.f,  1.f, -1.f,  1.f,  1.f, -1.f,  1.f,
                                       1.f, -1.f, -1.f,  1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f, -1.f,  1.f,
                                       -1.f,  1.f,  1.f, -1.f, -1.f,  1.f, -1.f,  1.f, -1.f, -1.f, -1.f, -1.f,
                                       -1.f,  1.f, -1.f, -1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,
                                       1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f, -1.f, -1.f,  1.f, -1.f,  1.f,
                                       -1.f, -1.f,  1.f,  1.f, -1.f,  1.f, -1.f, -1.f, -1.f,  1.f, -1.f, -1.f };

    m_db.createBuffer( "cube_nrm" )
            ->set( temporaryCubeNormArray, sizeof(temporaryCubeNormArray) / sizeof(float));


    float P[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1.0202f, -1, 0, 0, -0.121333f, 0 };
    float Pi[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -8.24176f, 0, 0, -1, 8.40826f };
    float cfw[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -3, 1};
    float ctw[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 3, 1 };
    float lfw[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    float ltw[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    float c0fw[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    float c0tw[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

    m_db.createShader( "phong" )
            ->setVertexStage( "uniform mat4 MVP;\n"
                              "uniform mat3 NM;\n"
                              "attribute vec3 position;\n"
                              "attribute vec3 normal;\n"
                              "varying vec3 es_normal;\n"
                              "void\n"
                              "main()\n"
                              "{\n"
                              "    es_normal = NM * normal;\n"
                              "    gl_Position = MVP * vec4( position, 1.0 );\n"
                              "}\n" )
            ->setFragmentStage( "precision highp float;\n"
                                "varying vec3 es_normal;\n"
                                "uniform vec3 color;\n"
                                "void\n"
                                "main()\n"
                                "{\n"
                                "    vec3 light_z = normalize( vec3(1,1,1) );\n"
                                "    gl_FragColor = vec4( color*max(0.1, dot( es_normal, light_z ) ), 1.0 );\n"
                                "}\n" );

    m_db.createAction<tinia::renderlist::Draw>( "cube_draw")
            ->setNonIndexed( tinia::renderlist::PRIMITIVE_TRIANGLES, 0, 36 );

    m_db.createAction<tinia::renderlist::SetViewCoordSys>( "cam" )
            ->setProjection( P, Pi )
            ->setOrientation( cfw, ctw );

    m_db.createAction<tinia::renderlist::SetLight>( "light" )
        ->setType( tinia::renderlist::LIGHT_POINT )
        ->setIndex( 0 )
        ->setColor( 0.8f, 1.f, 1.f )
        ->setAttenuation( 1.f, 0.f, 0.f )
        ->setFalloff( 3.14159f, 0.f )
        ->setOrientation( lfw, ltw );

    m_db.createAction<tinia::renderlist::SetInputs>( "phong_cube" )
        ->setShader( "phong" )
        ->setInput( "position", "cube_pos", 3 )
        ->setInput( "normal", "cube_nrm", 3 );

    m_db.createAction<tinia::renderlist::SetUniforms>( "phong_uniforms" )
        ->setShader( "phong" )
        ->setSemantic( "MVP", tinia::renderlist::SEMANTIC_MODELVIEW_PROJECTION_MATRIX )
        ->setSemantic( "NM", tinia::renderlist::SEMANTIC_NORMAL_MATRIX )
        ->setFloat3( "color", 0.8f, 0.9f, 1.f );

    m_db.createAction<tinia::renderlist::SetLocalCoordSys>( "cube0_pos" )
        ->setOrientation( c0fw, c0tw );

    m_db.createAction<tinia::renderlist::SetShader>( "use_phong" )
        ->setShader( "phong" );

    m_db.createAction<tinia::renderlist::SetFramebuffer>( "default_fbo" )
        ->setDefault();

    m_db.createAction<tinia::renderlist::SetPixelState>( "px_state" )
        ->enableDepthTest()
        ->disableBlending();

    m_db.createAction<tinia::renderlist::SetFramebufferState>( "fb_state" )
        ->setColorWritemask( true, true, true )
        ->setDepthWritemask( true );

    m_db.createAction<tinia::renderlist::SetRasterState>( "rs_state" );

    m_db.drawOrderClear()
        ->drawOrderAdd( "cam" )
        ->drawOrderAdd( "cube0_pos" )
        ->drawOrderAdd( "default_fbo" )
        ->drawOrderAdd( "rs_state" )
        ->drawOrderAdd( "px_state" )
        ->drawOrderAdd( "fb_state" )
        ->drawOrderAdd( "use_phong" )
        ->drawOrderAdd( "phong_cube" )
        ->drawOrderAdd( "phong_uniforms" )
        ->drawOrderAdd( "cube_draw" );

    std::cout << tinia::renderlist::getUpdateXML( &m_db, tinia::renderlist::ENCODING_JSON, r );
    r = m_db.latest();

    /*

    std::cerr << "** creating buf0\n";
    tinia::renderlist::Buffer* buf0 = m_db.createBuffer( "buf0" );
    r = dumpChanges( r );

    std::cerr << "** creating buf1,,\n";
    tinia::renderlist::Buffer* buf1 = m_db.createBuffer( "buf1" );
    r = dumpChanges( r );

    std::cerr << "** setting buf0 data\n";
    buf0->set( { 1.f, 2.f, 3.f, } );
    r = dumpChanges( r );

    std::vector<float> foo;
    foo.push_back( 6.f, );
    foo.push_back( 7.f, );
    foo.push_back( 8.f, );
    foo.push_back( 9.f, );
    buf1->set( foo.data(), foo.size() );


    tinia::renderlist::Shader* sh0 = m_db.createShader( "sh0" );
    sh0
    ->setVertexStage( "vertex" )
    ->setGeometryStage( "geometry" )
    ->setFragmentStage( "fragment" );

    tinia::renderlist::SetInputs* si = m_db.createSetInputs( "si" );
    si
    ->addInput( "position", buf1->id(), 3 )
    ->addInput( "normal", buf1->id(), 3 );

    std::cerr << "** deleting buf0\n";
    m_db.deleteBuffer( buf0->id() );
    r = dumpChanges( r );
*/

}


BOOST_AUTO_TEST_SUITE_END()
