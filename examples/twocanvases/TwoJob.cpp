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
#include "utils.hpp"
#include "TwoJob.hpp"
#include "tinia/model/GUILayout.hpp"
#include <iostream>
#include "tinia/model/File.hpp"


////// Code to reproduce error
#include "tinia/model/impl/xml/XMLHandler.hpp"
#include "tinia/model/StateElement.hpp"
#include "tinia/model/StateSchemaElement.hpp"
#include "tinia/model/impl/xml/XMLBuilder.hpp"
////// End of Code to reproduce error





namespace tinia {
namespace example {
TwoJob::TwoJob()
{
}

bool TwoJob::init()
{
    tinia::model::Viewer viewer;
    m_model->addElement("viewer", viewer);
    m_model->addElement("viewer2", tinia::model::Viewer());
    
    m_model->addElement<std::string>( "boundingbox", "-2.0 -2.0 -2.0 2.0 2.0 2.0" );

#define USE_AUTO_P // Enable this to turn this into a "dual canvas autoProxy example" and not only a "dual canvas" example!

#ifdef USE_AUTO_P
    m_model->addElement<bool>( "ap_useAutoProxy", true );
    m_model->addAnnotation("ap_useAutoProxy", "Automatically generated proxy geometry");

    // The rest of these lines are not needed, default values will be used if they are not explicitly set
    // Some of these (those setting uniforms and in other ways affecting the shaders) will need the debugging to be turned on for having any effect.
    // Note that turning debugging on may also have other side effects.

    m_model->addElement<bool>( "ap_autoProxyDebugging", true ); // Turning this on for demonstration of the shader modifications "ap_debugSplatCol" enabled below.
    m_model->addAnnotation("ap_autoProxyDebugging", "Debug mode");
//    const char *allowed_auto_proxy_algos[] = { "0) AngleCoverage-5",
//                                               "1) AngleCoverage-2",
//                                               "2) OnlyMostRecent",
//                                               "3) ReplOldestWhnDiff-5",
//                                               "4) ReplaceOldest-5",
//                                               NULL }; // NB! This list must match the one in ProxyRenderer.js, exactly! (I think...)
//    int algos=0;
//    while ( allowed_auto_proxy_algos[algos] != NULL ) {
//        algos++;
//    }
//    m_model->addElementWithRestriction<std::string>( "ap_autoProxyAlgo", allowed_auto_proxy_algos[2], &allowed_auto_proxy_algos[0], &allowed_auto_proxy_algos[0]+algos );
//    m_model->addAnnotation("ap_autoProxyAlgo", "Proxy model replacement algo");
    m_model->addElement<bool>( "ap_debugSplatCol", true );
    m_model->addAnnotation("ap_debugSplatCol", "Index coloring (r, g, b, y, c, m)");
//    m_model->addElement<bool>( "ap_screenSpaceSized", true );
//    m_model->addAnnotation("ap_screenSpaceSized", "Screen-space-sized splats");
//    m_model->addConstrainedElement<int>("ap_overlap", 200, 1, 300);
//    m_model->addAnnotation("ap_overlap", "Overlap factor)");
//    m_model->addElement<bool>( "ap_alwaysShowMostRecent", true );
//    m_model->addAnnotation("ap_alwaysShowMostRecent", "Most recent model in front");
    m_model->addConstrainedElement<int>("ap_splats", 15, 2, 512);
    m_model->addAnnotation("ap_splats", "Number of splats)");
//    m_model->addElement<bool>( "ap_splatOutline", true );
//    m_model->addAnnotation("ap_splatOutline", "Square splat outline");
//    m_model->addElement<bool>( "ap_useFragExt", true );
//    m_model->addAnnotation("ap_useFragExt", "Use FragDepthExt if available");

#endif

    tinia::model::gui::HorizontalLayout *rootLayout = new tinia::model::gui::HorizontalLayout();
    rootLayout->addChild( new tinia::model::gui::Canvas("viewer", "renderlist", "boundingbox") );
    rootLayout->addChild( new tinia::model::gui::Canvas("viewer2", "renderlist", "boundingbox") );

#ifdef USE_AUTO_P
    tinia::model::gui::Grid *mainGrid = new tinia::model::gui::Grid(2, 3);
    mainGrid->setChild(0, 0, new tinia::model::gui::CheckBox("ap_useAutoProxy"));
    mainGrid->setChild(1, 0, new tinia::model::gui::HorizontalSlider("ap_splats"));
    mainGrid->setChild(1, 1, new tinia::model::gui::Label("ap_splats", false));
    mainGrid->setChild(1, 2, new tinia::model::gui::Label("ap_splats", true));
    rootLayout->addChild(mainGrid);
#endif

    m_model->setGUILayout(rootLayout, tinia::model::gui::DESKTOP);

    return true;
}

TwoJob::~TwoJob()
{
}

void TwoJob::stateElementModified(tinia::model::StateElement *stateElement)
{
}

bool TwoJob::renderFrame(const std::string &session, const std::string &key, unsigned int fbo, const size_t width, const size_t height)
{
    ////// Code to reproduce error
    {
        std::string failingxml = "<State> \
                            <viewer> \
                            <height>512</height> \
                            <modelview>1 0 0 0 0 1 0 0 0 0 1 0 -1315.632568359375 310.7430114746094 -1648.2550048828125 1</modelview> \
                            <projection>1 0 0 0 0 1 0 0 0 0 -2.9648404121398926 -1 0 0 -4516.4921875 0</projection> \
                            <sceneView>0</sceneView> \
                            <timestamp>0</timestamp> \
                            <width>512</width> \
                            </viewer> \
                            <viewer2> \
                            <height>512</height> \
                            <modelview>1 0 0 0 0 1 0 0 0 0 1 0 -1315.632568359375 310.7430114746094 -1648.2550048828125 1</modelview> \
                            <projection>1 0 0 0 0 1 0 0 0 0 -2.9648404121398926 -1 0 0 -4516.4921875 0</projection> \
                            <sceneView>0</sceneView> \
                            <timestamp>0</timestamp> \
                            <width>512</width> \
                            </viewer2> \
                            </State> \
                            ";

        std::string workingxml = "<State> \
                             <viewer> \
                             <height>512</height> \
                             <modelview>1 0 0 0 0 1 0 0 0 0 1 0 -1315.632568359375 310.7430114746094 -1648.2550048828125 1</modelview> \
                             <projection>1 0 0 0 0 1 0 0 0 0 -2.9648404121398926 -1 0 0 -4516.4921875 0</projection> \
                             <sceneView>0</sceneView> \
                             <timestamp>0</timestamp> \
                             <width>512</width> \
                             </viewer> \
                             </State> \
                             ";
        xmlDocPtr doc = NULL;
        try {
            tinia::model::impl::xml::XMLTransporter xmlTransporter;
            doc = xmlTransporter.readXMLfromBuffer(failingxml.c_str(), failingxml.length());
//            doc = xmlTransporter.readXMLfromBuffer(workingxml.c_str(), workingxml.length());
            tinia::model::impl::xml::XMLReader xmlReader;
            tinia::model::impl::xml::ElementHandler elementHandler(m_model);
            xmlReader.parseDocument(doc, elementHandler);
        } catch(const std::exception& e) {
            std::cerr<<"XML ERROR: \n";
            std::cerr<<"MESSAGE: " << e.what() << std::endl;

        } catch(...) {
            std::cerr<<"UNKNOWN ERROR: \n";
        }
    }
    ////// End of Code to reproduce error


    glEnable(GL_DEPTH_TEST);

    if  (key =="viewer") {
        glClearColor(0.2, 0, 0, 1);
    } else {
        glClearColor(0, 0.2, 0, 1);
    }
    
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glViewport(0, 0, width, height);
    tinia::model::Viewer viewer;
    m_model->getElementValue( key, viewer);
    glLoadIdentity();
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf( viewer.projectionMatrix.data() );
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf( viewer.modelviewMatrix.data() );

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0, 0.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0, 0.0 );
    glVertex3f(  0.5, -0.5, 0.5 );
    glVertex3f(  0.5,  0.5, 0.5 );
    glVertex3f( -0.5,  0.5, 0.5 );
    glVertex3f( -0.5, -0.5, 0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(  .0,  0.0,  1.0 );
    glVertex3f( 0.5, -0.5, -0.5 );
    glVertex3f( 0.5,  0.5, -0.5 );
    glVertex3f( 0.5,  0.5,  0.5 );
    glVertex3f( 0.5, -0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  1.0,  0.0 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   1.0,  0.0,  1.0 );
    glVertex3f(  0.5,  0.5,  0.5 );
    glVertex3f(  0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5, -0.5 );
    glVertex3f( -0.5,  0.5,  0.5 );
    glEnd();

    glBegin(GL_POLYGON);
    glColor3f(   0.0,  1.0,  1.0 );
    glVertex3f(  0.5, -0.5, -0.5 );
    glVertex3f(  0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5,  0.5 );
    glVertex3f( -0.5, -0.5, -0.5 );
    glEnd();
    // CHECK_GL;

    return true;
}

const tinia::renderlist::DataBase*
TwoJob::getRenderList( const std::string& session, const std::string& key )
{
    return &m_renderlist_db;
}
}
}
