#define BOOST_AUTO_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <typeinfo>
#include <tinia/librenderlist/DataBase.hpp>
#include <tinia/librenderlist/Buffer.hpp>
#include <tinia/librenderlist/Draw.hpp>
#include <tinia/librenderlist/SetViewCoordSys.hpp>
#include <tinia/librenderlist/Shader.hpp>
#include <tinia/librenderlist/SetShader.hpp>
#include <tinia/librenderlist/SetInputs.hpp>
#include <tinia/librenderlist/SetUniforms.hpp>
#include <tinia/librenderlist/SetLight.hpp>
#include <tinia/librenderlist/SetLocalCoordSys.hpp>
#include <tinia/librenderlist/SetFramebuffer.hpp>
#include <tinia/librenderlist/SetFramebufferState.hpp>
#include <tinia/librenderlist/SetPixelState.hpp>
#include <tinia/librenderlist/SetRasterState.hpp>
#include <tinia/librenderlist/XMLWriter.hpp>

BOOST_AUTO_TEST_SUITE( librenderlist )

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

    librenderlist::Revision
    dumpChanges( librenderlist::Revision has_revision )
    {

        std::cerr << "----------------------------\n";
        std::cerr << "has_revision=" << has_revision << ", latest=" << m_db.latest() << "\n";
        std::cout << librenderlist::getUpdateXML( &m_db, librenderlist::ENCODING_JSON,  has_revision );
        std::cerr << "----------------------------\n";
        return m_db.latest();
    }


    librenderlist::DataBase m_db;
};

BOOST_FIXTURE_TEST_CASE( foobar, Fixture )
{
    Revision r = 0;


    m_db.createBuffer( "cube_pos" )
            ->set( { 0.f, 0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,
                   0.f, 0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,
                   0.f, 0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,
                   1.f, 0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,
                   1.f, 0.f,  0.f,  1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,
                   -1.f, 0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f,
                   0.f, 1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,  0.f,  1.f,  0.f,
                   0.f, 1.f,  0.f,  0.f,  1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,
                   0.f, -1.f, 0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f,  0.f, -1.f,  0.f } );

    m_db.createBuffer( "cube_nrm" )
            ->set( { -1.f,  1.f, -1.f,  1.f,  1.f, -1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f,
                   -1.f, -1.f, -1.f, -1.f,  1.f, -1.f, -1.f,  1.f,  1.f,  1.f, -1.f,  1.f,
                   1.f,  1.f,  1.f,  1.f, -1.f,  1.f, -1.f,  1.f,  1.f, -1.f, -1.f,  1.f,
                   1.f,  1.f, -1.f,  1.f,  1.f,  1.f,  1.f, -1.f,  1.f,  1.f, -1.f,  1.f,
                   1.f, -1.f, -1.f,  1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f, -1.f,  1.f,
                   -1.f,  1.f,  1.f, -1.f, -1.f,  1.f, -1.f,  1.f, -1.f, -1.f, -1.f, -1.f,
                   -1.f,  1.f, -1.f, -1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,  1.f,
                   1.f,  1.f, -1.f, -1.f,  1.f, -1.f, -1.f, -1.f, -1.f,  1.f, -1.f,  1.f,
                   -1.f, -1.f,  1.f,  1.f, -1.f,  1.f, -1.f, -1.f, -1.f,  1.f, -1.f, -1.f } );


    float P[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1.0202, -1, 0, 0, -0.121333, 0 };
    float Pi[16] = { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, -8.24176, 0, 0, -1, 8.40826 };
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

    m_db.createAction<librenderlist::Draw>( "cube_draw")
            ->setNonIndexed( librenderlist::PRIMITIVE_TRIANGLES, 0, 36 );

    m_db.createAction<librenderlist::SetViewCoordSys>( "cam" )
            ->setProjection( P, Pi )
            ->setOrientation( cfw, ctw );

    m_db.createAction<librenderlist::SetLight>( "light" )
        ->setType( librenderlist::LIGHT_POINT )
        ->setIndex( 0 )
        ->setColor( 0.8f, 1.f, 1.f )
        ->setAttenuation( 1.f, 0.f, 0.f )
        ->setFalloff( 3.14159f, 0.f )
        ->setOrientation( lfw, ltw );

    m_db.createAction<librenderlist::SetInputs>( "phong_cube" )
        ->setShader( "phong" )
        ->setInput( "position", "cube_pos", 3 )
        ->setInput( "normal", "cube_nrm", 3 );

    m_db.createAction<librenderlist::SetUniforms>( "phong_uniforms" )
        ->setShader( "phong" )
        ->setSemantic( "MVP", SEMANTIC_MODELVIEW_PROJECTION_MATRIX )
        ->setSemantic( "NM", SEMANTIC_NORMAL_MATRIX )
        ->setFloat3( "color", 0.8f, 0.9f, 1.f );

    m_db.createAction<librenderlist::SetLocalCoordSys>( "cube0_pos" )
        ->setOrientation( c0fw, c0tw );

    m_db.createAction<librenderlist::SetShader>( "use_phong" )
        ->setShader( "phong" );

    m_db.createAction<librenderlist::SetFramebuffer>( "default_fbo" )
        ->setDefault();

    m_db.createAction<librenderlist::SetPixelState>( "px_state" )
        ->enableDepthTest()
        ->disableBlending();

    m_db.createAction<librenderlist::SetFramebufferState>( "fb_state" )
        ->setColorWritemask( true, true, true )
        ->setDepthWritemask( true );

    m_db.createAction<librenderlist::SetRasterState>( "rs_state" );

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

    std::cout << librenderlist::getUpdateXML( &m_db, librenderlist::ENCODING_JSON, r );
    r = m_db.latest();

    /*

    std::cerr << "** creating buf0\n";
    librenderlist::Buffer* buf0 = m_db.createBuffer( "buf0" );
    r = dumpChanges( r );

    std::cerr << "** creating buf1,,\n";
    librenderlist::Buffer* buf1 = m_db.createBuffer( "buf1" );
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


    librenderlist::Shader* sh0 = m_db.createShader( "sh0" );
    sh0
    ->setVertexStage( "vertex" )
    ->setGeometryStage( "geometry" )
    ->setFragmentStage( "fragment" );

    librenderlist::SetInputs* si = m_db.createSetInputs( "si" );
    si
    ->addInput( "position", buf1->id(), 3 )
    ->addInput( "normal", buf1->id(), 3 );

    std::cerr << "** deleting buf0\n";
    m_db.deleteBuffer( buf0->id() );
    r = dumpChanges( r );
*/
}


BOOST_AUTO_TEST_SUITE_END()
