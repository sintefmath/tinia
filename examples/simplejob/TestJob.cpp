#include <GL/glew.h>
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

#include "TestJob.hpp"
#include "tinia/policy/GUILayout.hpp"
#include <iostream>
#include <siut2/gl_utils/GLSLtools.hpp>
#include "tinia/policy/File.hpp"

TestJob::TestJob()
{
}

bool TestJob::init()
{


   m_policy->addStateListener(this);
   tinia::policy::Viewer viewer;
   viewer.height = 500;
   viewer.width = 500;
   m_policy->addElement("viewer", viewer);
   m_policy->addElement<std::string>( "boundingbox", "-1.1 -1.1 -1.1 1.1 1.1 1.1" );
   m_policy->addElement<int>( "renderlist", 0 );

   m_policy->addElement<bool>("myTab", false);
   m_policy->addAnnotation("myTab", "My tab");
      m_policy->addElement<bool>("myBool", false);
   m_policy->addElement<std::string>("myVal", "THIS WORKS!");

   const char* restrictions[] = {"select1", "select2", "select3", "select4"};
   m_policy->addElementWithRestriction<std::string>("myVal2", "select1",
                                                       &restrictions[0], &restrictions[4]);

   m_policy->addAnnotation("myVal2", "My special value");

   m_policy->addConstrainedElement<int>("myIntBA", 5,0, 900);
   m_policy->addConstrainedElement<double>("myDouble", 10., 0., 11.);
   m_policy->addElement<bool>("myButton", false);

   m_policy->addElement<bool>( "details", true, "Popup" );

   m_policy->addAnnotation("myButton", "My pushButton");

   tinia::policy::File file;
   m_policy->addElement("SpecialFile", file);
   m_policy->addAnnotation("SpecialFile", "Open file");
   m_policy->addElement("SpecialFileName", file.name());

   using namespace tinia::policy::gui;
   TabLayout* root = new TabLayout();
   HorizontalLayout *superLayout = new HorizontalLayout();

   Grid *mainGrid = new Grid(16, 1);
   //superLayout->addChild(mainGrid);
   Tab *mainTab = new Tab("myTab");
   mainTab->setChild(superLayout);
   root->addChild(mainTab);


   TabLayout *subTablayout = new TabLayout();
   Tab* subTab = new Tab("myBool");

   subTablayout->addChild(subTab);

   subTab->setChild(mainGrid);

   superLayout->addChild(subTablayout);

   // Simple Label-input-layout
   Grid *input1Grid = new Grid(4,3);
   mainGrid->setChild(0,0, input1Grid);
   input1Grid->setChild(0, 0, new Label("myVal"));
   input1Grid->setChild(0, 1, (new TextInput("myVal"))->setEnabledKey( "myTab", true ) );

   input1Grid->setChild(1,0, new Label("myVal2"));
   input1Grid->setChild(1,1, new ComboBox("myVal2"));

   input1Grid->setChild(2,0, new Label("myVal2"));
   input1Grid->setChild(2,1, new RadioButtons("myVal2"));

   PopupButton* popup = new PopupButton("details", false);
   input1Grid->setChild(2, 2, popup );
   popup->setEnabledKey( "myTab" );

   PopupButton* popup2 = new PopupButton( "myVal", true );
   input1Grid->setChild(3, 2, popup2 );

   popup2->setChild( new TextInput("myVal" ) );


   ElementGroup* popup_group = new ElementGroup( "myTab", true );
   popup->setChild( popup_group );
   popup_group->setChild( new RadioButtons( "myVal2" ) );


   mainGrid->setChild(1, 0, new SpinBox("myIntBA"));
   mainGrid->setChild(2, 0, new TextInput("myIntBA"));
   mainGrid->setChild(3, 0, new CheckBox("myTab"));
   mainGrid->setChild(4, 0, new CheckBox("myTab"));
   mainGrid->setChild(5, 0, new Button("myButton"));
   mainGrid->setChild(6, 0, new HorizontalSlider("myIntBA"));
   mainGrid->setChild(8,0, new DoubleSpinBox("myDouble"));
   mainGrid->setChild(9, 0, new CheckBox("myBool"));
   mainGrid->setChild(10, 0, new VerticalExpandingSpace());
   mainGrid->setChild(11, 0, new Label("myDouble", true ) );
   mainGrid->setChild(12, 0, new FileDialogButton("SpecialFile"));
   mainGrid->setChild(13, 0, new Label("SpecialFileName", true));

   ElementGroup* elemGroup = new ElementGroup("myTab", false);
   elemGroup->setVisibilityKey( "myTab" );

   elemGroup->setChild( new TextInput("myVal") );
   mainGrid->setChild(7, 0, elemGroup);

   Grid* canvasGrid = new Grid(2,1);
   Canvas *canvas = new Canvas("viewer", "renderlist", "boundingbox" );

   canvasGrid->setChild(0,0, canvas);
   VerticalLayout* verticalLayout = new VerticalLayout;
   verticalLayout->setVisibilityKey( "myBool" );
   verticalLayout->addChild(elemGroup);
   verticalLayout->addChild(elemGroup);
   verticalLayout->addChild(elemGroup);
   canvasGrid->setChild(1,0, verticalLayout);
   superLayout->addChild(canvasGrid);
   m_policy->setGUILayout(root, DESKTOP);



   // ---- Set up renderlist data

   // Geometry for a simple line segment cube
   float wire_cube_pos[12*2*3] = {
       0.f, 0.f, 0.f,  1.f, 0.f, 0.f,
       0.f, 1.f, 0.f,  1.f, 1.f, 0.f,
       0.f, 0.f, 0.f,  0.f, 1.f, 0.f,
       1.f, 0.f, 0.f,  1.f, 1.f, 0.f,
       0.f, 0.f, 0.f,  0.f, 0.f, 1.f,
       0.f, 1.f, 0.f,  0.f, 1.f, 1.f,
       1.f, 0.f, 0.f,  1.f, 0.f, 1.f,
       1.f, 1.f, 0.f,  1.f, 1.f, 1.f,
       0.f, 0.f, 1.f,  1.f, 0.f, 1.f,
       0.f, 1.f, 1.f,  1.f, 1.f, 1.f,
       0.f, 0.f, 1.f,  0.f, 1.f, 1.f,
       1.f, 0.f, 1.f,  1.f, 1.f, 1.f,
   };
   m_renderlist_db.createBuffer( "wire_cube_pos" )->set( wire_cube_pos, 12*2*3 );
   m_renderlist_db.createAction<tinia::renderlist::Draw>( "wire_cube_draw" )
           ->setNonIndexed( tinia::renderlist::PRIMITIVE_LINES, 0, 12*2 );
   // Solid color shader
   std::string solid_vs =
           "uniform mat4 MVP;\n"
           "attribute vec3 position;\n"
           "void\n"
           "main()\n"
           "{\n"
           "    gl_Position = MVP * vec4( position, 1.0 );\n"
           "}\n";
   std::string solid_fs =
           "#ifdef GL_ES\n"
           "precision highp float;\n"
           "#endif\n"
           "uniform vec3 color;\n"
           "void\n"
           "main()\n"
           "{\n"
           "    gl_FragColor = vec4( color, 1.0 );\n"
           "}\n";
   m_renderlist_db.createShader( "solid" )
           ->setVertexStage( solid_vs )
           ->setFragmentStage( solid_fs );
   m_renderlist_db.createAction<tinia::renderlist::SetShader>( "solid_use" )
           ->setShader( "solid" );
   m_renderlist_db.createAction<tinia::renderlist::SetUniforms>( "solid_orient" )
           ->setShader( "solid" )
           ->setSemantic( "MVP", tinia::renderlist::SEMANTIC_MODELVIEW_PROJECTION_MATRIX );
   m_renderlist_db.createAction<tinia::renderlist::SetUniforms>( "solid_white" )
           ->setShader( "solid" )
           ->setFloat3( "color", 1.f, 1.f, 1.f );
   m_renderlist_db.createAction<tinia::renderlist::SetUniforms>( "solid_green" )
           ->setShader( "solid" )
           ->setFloat3( "color", 0.f, 1.f, 0.f );

   // Combine the wire cube geometry with the solid shader
   m_renderlist_db.createAction<tinia::renderlist::SetInputs>( "solid_wire_cube_input" )
           ->setShader( "solid" )
           ->setInput( "position", "wire_cube_pos", 3 );

   // Position and scale the two instances, encoded as a transform matric and its inverse
   float shape0_to_world[16] = {
       2.f, 0.f, 0.f, -1.0f,
       0.f, 2.f, 0.f, -1.0f,
       0.f, 0.f, 2.f, -1.0f,
       0.f, 0.f, 0.f,  1.f,
   };
   float shape0_from_world[16] = {
       0.5f,  0.f,  0.f,  1.0f,
       0.f,  0.5f,  0.f,  1.0f,
       0.f,  0.f,  0.5f,  1.0f,
       0.f,  0.f,  0.f,   1.f,
   };
   m_renderlist_db.createAction<tinia::renderlist::SetLocalCoordSys>( "shape0_pos" )
           ->setOrientation( shape0_from_world, shape0_to_world );

   float shape1_to_world[16] = {
       1.f, 0.f, 0.f, -0.5f,
       0.f, 1.f, 0.f, -0.5f,
       0.f, 0.f, 1.f, -0.5f,
       0.f, 0.f, 0.f,  1.f,
   };
   float shape1_from_world[16] = {
       1.f,  0.f,  0.f,  0.5f,
       0.f,  1.f,  0.f,  0.5f,
       0.f,  0.f,  1.f,  0.5f,
       0.f,  0.f,  0.f,   1.f,
   };
   m_renderlist_db.createAction<tinia::renderlist::SetLocalCoordSys>( "shape1_pos" )
           ->setOrientation( shape1_from_world, shape1_to_world );

   // Set up draw order
   m_renderlist_db.drawOrderClear()
           ->drawOrderAdd( "solid_use")
           ->drawOrderAdd( "solid_wire_cube_input" )

           ->drawOrderAdd( "shape0_pos" )      // sets transform matrices
           ->drawOrderAdd( "solid_orient" )    // updates uniforms
           ->drawOrderAdd( "solid_white" )     // set color uniform
           ->drawOrderAdd( "wire_cube_draw" )  // invoke draw call

           ->drawOrderAdd( "shape1_pos" )
           ->drawOrderAdd( "solid_orient" )
           ->drawOrderAdd( "solid_green" )
           ->drawOrderAdd( "wire_cube_draw" );

   m_renderlist_db.process();
   m_policy->updateElement<int>( "renderlist", m_renderlist_db.latest() );

   glewInit();

   static const GLfloat quad[ 4*4 ] = {
        1.f, -1.f, 0.f, 1.f,
        1.f,  1.f, 0.f, 1.f,
       -1.f, -1.f, 0.f, 1.f,
       -1.f,  1.f, 0.f, 1.f
   };
   glGenVertexArrays( 1, &m_gpgpu_quad_vertex_array );
   glBindVertexArray( m_gpgpu_quad_vertex_array );
   glGenBuffers( 1, &m_gpgpu_quad_buffer );
   glBindBuffer( GL_ARRAY_BUFFER, m_gpgpu_quad_buffer );
   glBufferData( GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW );
   glVertexPointer( 4, GL_FLOAT, 0, NULL );
   glEnableClientState( GL_VERTEX_ARRAY );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );
   glBindVertexArray( 0 );
   CHECK_GL;

   glBindVertexArray( m_gpgpu_quad_vertex_array );
   glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
   glBindVertexArray( 0 );
   CHECK_GL;



    return true;
}

TestJob::~TestJob()
{
   m_policy->removeStateListener(this);
}

void TestJob::stateElementModified(tinia::policy::StateElement *stateElement)
{
   if(stateElement->getKey() == "myButton")
   {
      bool value;
      stateElement->getValue<bool>(value);
      if(value)
      {
         m_policy->updateElement<bool>("myButton", false);
         std::cout<<"button clicked"<<std::endl;
      }
   }
   if(stateElement->getKey() == "SpecialFile")
   {
      tinia::policy::File file;
      stateElement->getValue(file);
      m_policy->updateElement("SpecialFileName", file.name());
   }

   if(stateElement->getKey() == "myIntBA") {
       int max;
       stateElement->getValue(max);
       m_policy->updateConstraints("myDouble", max/2., 0., double(max));
   }

}

bool TestJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
{
    glClearColor(0, 0, 0, 1);
    glClear( GL_COLOR_BUFFER_BIT );

	glViewport(0, 0, width, height);
    tinia::policy::Viewer viewer;
    m_policy->getElementValue( key, viewer);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( viewer.projectionMatrix.data() );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( viewer.modelviewMatrix.data() );

    glBegin(GL_TRIANGLES);
    glColor3f( 1.f, 0.f, 0.f );
    glVertex2f(0,0);
    glColor3f( 0.f, 1.f, 0.f );
    glVertex2f(0,1);
    glColor3f( 0.f, 0.f, 1.f );
    glVertex2f(1,1);
    glEnd();
    CHECK_GL;

    // This works
    if(1) {
        glBindBuffer( GL_ARRAY_BUFFER, m_gpgpu_quad_buffer );
        glVertexPointer( 4, GL_FLOAT, 0, NULL );
        glBindBuffer( GL_ARRAY_BUFFER, 0 );
        glEnableClientState( GL_VERTEX_ARRAY );
        glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
        glDisableClientState( GL_VERTEX_ARRAY );
    }

    // glBindVertexArray fails.
    if(0) {
        glBindVertexArray( m_gpgpu_quad_vertex_array );
        CHECK_GL;
        glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );
        glBindVertexArray( 0 );
    }
    CHECK_GL;

    return true;
}

const tinia::renderlist::DataBase*
TestJob::getRenderList( const std::string& session, const std::string& key )
{
    return &m_renderlist_db;
}
