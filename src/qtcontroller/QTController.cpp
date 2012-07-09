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

#include "tinia/jobcontroller/Job.hpp"
#include "tinia/jobcontroller/OpenGLJob.hpp"
#include "tinia/qtcontroller/QTController.hpp"
#include "tinia/qtcontroller/GUIBuilder.hpp"
#include <tinia/qtcontroller/scripting/utils.hpp>
#include <QApplication>
#include <QLayout>
#include <QLabel>
#include <QGLWidget>
#include <QMainWindow>
#include <exception>

#include <QDomDocument>
#include <QString>

// This needs to be outside namespaces because QT says so...
static void tiniaInitResources() {
    Q_INIT_RESOURCE(resources);
}

namespace tinia {
namespace qtcontroller {

QTController::QTController()
    : m_main_window(NULL),
      m_root_context(NULL),
      m_job(NULL),
      m_builder(NULL),
      m_perf_mode( false ),
      m_renderlist_mode( false )
{

}

QTController::~QTController()
{
    if( m_main_window != NULL ){
        delete m_main_window;
    }

    if( m_job != NULL ) {
        delete m_job;
    }

    if( m_builder != NULL ) {
        delete m_builder;
    }
}




void QTController::setJob(jobcontroller::Job *job)
{
    if( m_job != NULL ) {
        throw std::runtime_error( "Job already set" );
    }
    else {
       m_job = job;
    }
}

void QTController::notify()
{
}

void QTController::fail()
{
}

void QTController::finish()
{
}

int QTController::run(int argc, char **argv)
{
    if( m_job == NULL ) {
        throw std::runtime_error( "No job set" );
    }
    m_model = m_job->getExposedModel();



    QApplication app(argc, argv);
    m_main_window = new QMainWindow();

    // Now we may init the script.
    tiniaInitResources();
    initScript();

    if( dynamic_cast<jobcontroller::OpenGLJob*>( m_job ) ) {
        for( int i=1; i<argc; i++) {
            if( strcmp(argv[i], "--perf" ) == 0 ) {
                m_perf_mode = true;
            }
            else if( strcmp( argv[i], "--renderlist" ) == 0 ) {
                m_renderlist_mode = true;
            }
        }

        // Create an off-screen context that is subsequently shared with all
        // GL widgets that will be subsequently created. This avoids requiring
        // that the view is defined before doing GL init as well as allowing
        // GL resources to be shared between widgets.
        m_root_context = new QGLWidget(QGL::DepthBuffer| QGL::DoubleBuffer | QGL::AlphaChannel);
        m_root_context->makeCurrent();
    }
    if( !m_job->init() ) {
       throw new std::runtime_error("Job did not start up properly");
    }

    m_builder = new GUIBuilder( m_model,
                                m_job,
                                this,
                                m_perf_mode,
                                m_root_context );

    m_main_window->setCentralWidget( m_builder->buildGUI( m_model->getGUILayout(model::gui::DESKTOP),
                                                          NULL ) );

	if( dynamic_cast<jobcontroller::OpenGLJob*>( m_job ) ) {
	if( !(dynamic_cast<jobcontroller::OpenGLJob*>(m_job))->initGL()) {
		throw new std::runtime_error("Job did not initialize GL correctly");
	}
	}
    m_main_window->show();

/*
   QString style =
           "QGroupBox::title { color: black; } "
           "QGroupBox { margin: 0; border: 0; padding: 0; }";

   app.setStyleSheet( style );
*/
   /*
   model::gui::Element* tree = m_model->getGUILayout( model::gui::DESKTOP );
   QDomDocument* dom = new QDomDocument;
   QDomElement gui_layout = dom->createElement( "layout" );
   dom->appendChild( gui_layout );
   addNode( dom, gui_layout, tree ); 
   QString foo = dom->toString();
   
   std::cerr << foo.toLocal8Bit() << "\n";
   */
    return app.exec();
}

void QTController::initScript()
{
    m_engine = scripting::ScriptEngine::getInstance();
    scripting::addDefaultScripts(m_engine->engine());
}

namespace  {
static
QDomElement
addNode( QDomDocument* dom, QDomElement& parent, model::gui::Element* branch );

template<typename ChildType>
static
void
addChildren( QDomDocument* dom, QDomElement& parent, model::gui::Container1D<ChildType>* branch )
{
    for(size_t i=0; i<branch->children(); i++ ) {
        addNode( dom, parent, branch->child(i) );
    }
}

static
void
addChildren( QDomDocument* dom, QDomElement& parent, model::gui::Container2D<model::gui::Element>* branch )
{
    parent.setAttribute( "width", (uint)branch->width() );
    parent.setAttribute( "height", (uint)branch->height() );
    for(size_t j=0; j<branch->height(); j++ ) {
        for(size_t i=0; i<branch->width(); i++ ) {
            auto child = branch->child( j, i );
            if( child != NULL ) {
                QDomElement n = addNode( dom, parent, child );
                n.setAttribute( "row", (uint)j );
                n.setAttribute( "col", (uint)i );
            }
        }
    }
}

static
void
addElementAttributes( QDomDocument* dom, QDomElement& node, model::gui::Element* element )
{
    if( !element->visibilityKey().empty() ) {
        QDomElement n = dom->createElement( "Visibility" );
        QDomText t = dom->createTextNode( element->visibilityKey().c_str() );
        n.appendChild( t );
        if( element->visibilityInverted() ) {
            n.setAttribute( "inverted", 1 );
        }
        node.appendChild( n );
    }
    if( !element->enabledKey().empty() ) {
        QDomElement n = dom->createElement( "Enabled" );
        QDomText t = dom->createTextNode( element->enabledKey().c_str() );
        n.appendChild( t );
        if( element->enabledInverted() ) {
            n.setAttribute( "inverted", 1 );
        }
        node.appendChild( n );
    }
}

static
QDomElement
addNode( QDomDocument* dom, QDomElement& parent, model::gui::Element* branch )
{
    auto TabLayout = dynamic_cast<model::gui::TabLayout*>( branch );
    if( TabLayout ) {
        QDomElement node = dom->createElement( "TabLayout" );
        addElementAttributes( dom, node, branch );
        addChildren<model::gui::Tab>( dom, node, static_cast<model::gui::Container1D<model::gui::Tab>*>( TabLayout ) );
        parent.appendChild( node );
        return node;
    }
    auto Tab = dynamic_cast<model::gui::Tab*>( branch );
    if( Tab ) {
        QDomElement node = dom->createElement( "Tab" );
        addElementAttributes( dom, node, branch );
        addNode( dom, node, Tab->child() );
        parent.appendChild( node );
        return node;
    }
    auto Grid = dynamic_cast<model::gui::Grid*>( branch );
    if( Grid ) {
        QDomElement node = dom->createElement( "Grid" );
        addElementAttributes( dom, node, branch );
        addChildren( dom, node, static_cast<model::gui::Container2D<model::gui::Element>*>( Grid ) );
        parent.appendChild( node );
        return node;
    }
    auto TextInput = dynamic_cast<model::gui::TextInput*>( branch );
    if( TextInput ) {
        QDomElement node = dom->createElement( "TextInput" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto Canvas = dynamic_cast<model::gui::Canvas*>( branch );
    if( Canvas ) {
        QDomElement node = dom->createElement( "Canvas" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto Label = dynamic_cast<model::gui::Label*>( branch );
    if( Label ) {
        QDomElement node = dom->createElement( "Label" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto ComboBox = dynamic_cast<model::gui::ComboBox*>( branch );
    if( ComboBox ) {
        QDomElement node = dom->createElement( "ComboBox" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto ElementGroup = dynamic_cast<model::gui::ElementGroup*>( branch );
    if( ElementGroup ) {
        QDomElement node = dom->createElement( "ElementGroup" );
        addElementAttributes( dom, node, branch );
        addNode( dom, node, ElementGroup->child() );
        parent.appendChild( node );
        return node;
    }
    auto RadioButtons = dynamic_cast<model::gui::RadioButtons*>( branch );
    if( RadioButtons ) {
        QDomElement node = dom->createElement( "RadioButtons" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto SpinBox = dynamic_cast<model::gui::SpinBox*>( branch );
    if( SpinBox ) {
        QDomElement node = dom->createElement( "SpinBox" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto CheckBox = dynamic_cast<model::gui::CheckBox*>( branch );
    if( CheckBox ) {
        QDomElement node = dom->createElement( "CheckBox" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto Button = dynamic_cast<model::gui::Button*>( branch );
    if( Button ) {
        QDomElement node = dom->createElement( "Button" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto HorizontalSlider = dynamic_cast<model::gui::HorizontalSlider*>( branch );
    if( HorizontalSlider ) {
        QDomElement node = dom->createElement( "HorizontalSlider" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto HorizontalLayout = dynamic_cast<model::gui::HorizontalLayout*>( branch );
    if( HorizontalLayout ) {
        QDomElement node = dom->createElement( "HorizontalLayout" );
        addElementAttributes( dom, node, branch );
        addChildren<model::gui::Element>( dom, node, static_cast<model::gui::Container1D<model::gui::Element>*>( HorizontalLayout ) );
        parent.appendChild( node );
        return node;
    }
    auto VerticalLayout = dynamic_cast<model::gui::VerticalLayout*>( branch );
    if( VerticalLayout ) {
        QDomElement node = dom->createElement( "VerticalLayout" );
        addElementAttributes( dom, node, branch );
        addChildren<model::gui::Element>( dom, node, static_cast<model::gui::Container1D<model::gui::Element>*>( VerticalLayout ) );
        parent.appendChild( node );
        return node;
    }
    auto DoubleSpinBox = dynamic_cast<model::gui::DoubleSpinBox*>( branch );
    if( DoubleSpinBox ) {
        QDomElement node = dom->createElement( "DoubleSpinBox" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto VerticalSpace = dynamic_cast<model::gui::VerticalSpace*>( branch );
    if( VerticalSpace ) {
        QDomElement node = dom->createElement( "VerticalSpace" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto VerticalExpandingSpace = dynamic_cast<model::gui::VerticalExpandingSpace*>( branch );
    if( VerticalExpandingSpace ) {
        QDomElement node = dom->createElement( "VerticalExpandingSpace" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto HorizontalSpace = dynamic_cast<model::gui::HorizontalSpace*>( branch );
    if( HorizontalSpace ) {
        QDomElement node = dom->createElement( "HorizontalSpace" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    auto HorizontalExpandingSpace = dynamic_cast<model::gui::HorizontalExpandingSpace*>( branch );
    if( HorizontalExpandingSpace ) {
        QDomElement node = dom->createElement( "HorizontalExpandingSpace" );
        addElementAttributes( dom, node, branch );
        parent.appendChild( node );
        return node;
    }
    QDomElement node = dom->createElement( "IllegalShouldThrow" );
    parent.appendChild( node );
    return node;
}



}

}// namespace qtcontroller
} // of namespace tinia
