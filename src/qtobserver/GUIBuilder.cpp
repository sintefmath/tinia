#include "tinia/qtobserver/GUIBuilder.hpp"
#include "tinia/qtobserver/moc/TabWidgetChildren.hpp"
#include "tinia/qtobserver/moc/ComboBox.hpp"
#include "tinia/qtobserver/moc/RadioButtonGroup.hpp"
#include "tinia/qtobserver/moc/SpinBox.hpp"
#include "tinia/qtobserver/moc/CheckBox.hpp"
#include "tinia/qtobserver/moc/Button.hpp"
#include "tinia/qtobserver/moc/HorizontalSlider.hpp"
#include "tinia/qtobserver/moc/ElementGroup.hpp"
#include "tinia/qtobserver/moc/Canvas.hpp"
#include "tinia/qtobserver/moc/DoubleSpinBox.hpp"
#include "tinia/qtobserver/moc/VisibilityController.hpp"
#include "tinia/qtobserver/moc/EnabledController.hpp"
#include "tinia/qtobserver/moc/StringController.hpp"
#include "tinia/qtobserver/moc/PopupEventFilter.hpp"
#include "tinia/qtobserver/moc/FileDialogButton.hpp"
#include "tinia/qtobserver/utils.hpp"
#include <QLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSpacerItem>
#include <QSizePolicy>
#include <QLabel>
#include <QObject>
#include <QWidget>
#include <QPushButton>

namespace tinia {
namespace qtobserver {

namespace pg = policylib::gui;

GUIBuilder::GUIBuilder( std::shared_ptr<policylib::PolicyLib>   policyLib,
                        jobobserver::Job*                       job,
                        QTObserver*                             observer,
                        const bool                              perf_mode,
                        QGLWidget*                              root_context )
   : m_policyLib(policyLib),
     m_job(job),
     m_observer( observer ),
     m_perf_mode( perf_mode ),
     m_root_context( root_context )
{
}


QWidget* GUIBuilder::buildGUI(policylib::gui::Element* root, QWidget *parent )
{
    QWidget* widget = NULL;


    if( widget == NULL ) {
        pg::TabLayout* tab_layout = dynamic_cast<pg::TabLayout*>( root );
        if( tab_layout != NULL ) {
            widget = addTabLayout( tab_layout, parent );
        }
    }
    if( widget == NULL ) {
        pg::Grid* grid = dynamic_cast<pg::Grid*>( root );
        if( grid != NULL ) {
            widget = addGrid( grid, parent );
        }
    }
    if( widget == NULL ) {
        pg::TextInput* text_input = dynamic_cast<pg::TextInput*>( root );
        if( text_input != NULL ) {
            widget = new QLineEdit( parent );
            new StringController( widget,
                                  m_policyLib,
                                  text_input->key(),
                                  true );
        }
    }
    if( widget == NULL ) {
        pg::Canvas* canvas = dynamic_cast<pg::Canvas*>( root );
        if( canvas != NULL ) {
            widget = addCanvas( canvas, parent );
        }
    }
    if( widget == NULL ) {
        pg::Label* label = dynamic_cast<pg::Label*>( root );
        if( label != NULL ) {
            widget = new QLabel( parent );
            new StringController( widget,
                                  m_policyLib,
                                  label->key(),
                                  label->showValue() );
        }
    }
    if( widget == NULL ) {
        pg::ComboBox* combobox = dynamic_cast<pg::ComboBox*>( root );
        if( combobox != NULL ) {
            widget = addCombobox( combobox, parent);
        }
    }
    if( widget == NULL ) {
        pg::ElementGroup* element_group = dynamic_cast<pg::ElementGroup*>( root );
        if( element_group != NULL ) {
            widget = addElementGroup( element_group, parent);
        }
    }
    if( widget == NULL ) {
        pg::RadioButtons* radio_buttons = dynamic_cast<pg::RadioButtons*>( root );
        if( radio_buttons != NULL ) {
            widget = addRadiobuttons( radio_buttons, parent);
        }
    }
    if( widget == NULL ) {
        pg::SpinBox* spin_box = dynamic_cast<pg::SpinBox*>( root );
        if( spin_box != NULL ) {
            widget = addSpinBox( spin_box, parent);
        }
    }
    if( widget == NULL ) {
        pg::CheckBox* check_box = dynamic_cast<pg::CheckBox*>( root );
        if( check_box != NULL ) {
            widget = addCheckBox( check_box, parent);
        }
    }
    if( widget == NULL ) {
        pg::Button* button = dynamic_cast<pg::Button*>( root );
        if( button != NULL ) {
            widget = addButton( button, parent);
        }
    }
    if( widget == NULL ) {
        pg::HorizontalSlider* horizontal_slider = dynamic_cast<pg::HorizontalSlider*>( root );
        if( horizontal_slider != NULL ) {
            widget = addHorizontalSlider( horizontal_slider, parent);
        }
    }
    if( widget == NULL ) {
        pg::HorizontalLayout* horizontal_layout = dynamic_cast<pg::HorizontalLayout*>( root );
        if( horizontal_layout != NULL ) {
            widget = addHorizontalLayout( horizontal_layout, parent);
        }
    }
    if( widget == NULL ) {
        pg::VerticalLayout* vertical_layout = dynamic_cast<pg::VerticalLayout*>( root );
        if( vertical_layout != NULL ) {
            widget = addVerticalLayout( vertical_layout, parent);
        }
    }
    if( widget == NULL ) {
        pg::DoubleSpinBox* double_spinbox = dynamic_cast<pg::DoubleSpinBox*>( root );
        if( double_spinbox != NULL ) {
            widget = addDoubleSpinBox( double_spinbox, parent);
        }
    }
    if( widget == NULL ) {
        pg::PopupButton* popup_button = dynamic_cast<pg::PopupButton*>( root );
        if( popup_button != NULL ) {
            widget = addPopupButton( popup_button, parent );
        }
    }
    if(widget == NULL ) {
       pg::FileDialogButton* file_dialog_button = dynamic_cast<pg::FileDialogButton*>( root );
       if(file_dialog_button != NULL)
       {
          widget = addFileDialogButton(file_dialog_button, parent);
       }
    }

    if( widget != NULL ) {

        if( !root->visibilityKey().empty() ) {
            new VisibilityController( widget,
                                      m_policyLib,
                                      root->visibilityKey(),
                                      root->visibilityInverted() );
        }
        if( !root->enabledKey().empty() ) {
            new EnabledController( widget,
                                   m_policyLib,
                                   root->enabledKey(),
                                   root->enabledInverted() );
        }
    }

    return widget;
}

QWidget*
GUIBuilder::addPopupButton( policylib::gui::PopupButton* root, QWidget* parent )
{
    QString suffix( " " );
    suffix.append( QChar( 0x02ec ) );
    QPushButton* widget = new QPushButton( parent );
    widget->setCheckable( true );
    new StringController( widget, m_policyLib, root->key(),root->showValue(), suffix );

    QWidget* foo = buildGUI( root->child(), widget );
    if( foo != NULL ) {
        foo->setObjectName( "popup" );
        foo->setStyleSheet( "QWidget#popup { border: 1px solid black }");
        new PopupEventFilter( foo, widget );
    }
    return widget;
}



QWidget*
GUIBuilder::addTabLayout(policylib::gui::TabLayout *root, QWidget *parent)
{
    QTabWidget* tabWidget = new QTabWidget(parent);

    for( size_t i=0; i<root->children(); i++ ) {
        policylib::gui::Tab* tab = root->child( i );
        if( tab != NULL ) {
            QWidget* child = buildGUI( tab->child(), tabWidget );
            if( child != NULL ) {
                tabWidget->addTab( child,
                                   QString(prettyName(tab->key(), m_policyLib).c_str()) );
            }
            else {
                std::cerr << __FILE__ << '@' << __LINE__<< ": ERROR: got nullptr.\n";
            }
        }
        else {
            std::cerr << __FILE__ << '@' << __LINE__<< ": ERROR: child is not tab.\n";
        }
    }
   return tabWidget;
}


QWidget*
GUIBuilder::addHorizontalLayout(policylib::gui::HorizontalLayout *root, QWidget *parent)
{
    QWidget* widget = new QWidget(parent);
    doHorizontalLayout( root, widget );
    return widget;
}

QWidget*
GUIBuilder::addVerticalLayout(policylib::gui::VerticalLayout *root, QWidget *parent)
{
    QWidget* widget = new QWidget(parent);
    doVerticalLayout( root, widget );
    return widget;
}

void
GUIBuilder::doHorizontalLayout(policylib::gui::HorizontalLayout *root, QWidget *widget)
{
    QHBoxLayout* layout = new QHBoxLayout( widget );
    widget->setLayout( layout );
    widget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    addChildren( static_cast<policylib::gui::Container1D<policylib::gui::Element>*>( root ),
                 widget,
                 layout );
}

void
GUIBuilder::doVerticalLayout(policylib::gui::VerticalLayout *root, QWidget *widget)
{
    QVBoxLayout* layout = new QVBoxLayout( widget );
    widget->setLayout( layout );
    widget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    addChildren( static_cast<policylib::gui::Container1D<policylib::gui::Element>*>( root ),
                 widget,
                 layout );
}






QLayoutItem*
GUIBuilder::buildLayoutItem( policylib::gui::Element* root )
{
    policylib::gui::VerticalSpace* vspace = dynamic_cast<policylib::gui::VerticalSpace*>( root );
    if( vspace != NULL ) {
        return new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
    policylib::gui::VerticalExpandingSpace* vexspace = dynamic_cast<policylib::gui::VerticalExpandingSpace*>( root );
    if( vexspace != NULL ) {
        return new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    }

    policylib::gui::HorizontalSpace* hspace = dynamic_cast<policylib::gui::HorizontalSpace*>( root );
    if( hspace != NULL ) {
        return new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
    policylib::gui::HorizontalExpandingSpace* hexspace = dynamic_cast<policylib::gui::HorizontalExpandingSpace*>( root );
    if( hexspace != NULL ) {
        return new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    }
    return NULL;
}


QWidget*
GUIBuilder::addGrid(policylib::gui::Grid *root, QWidget *parent)
{
    QWidget* grid_widget = new QWidget(parent);
    doGridLayout( root, grid_widget );
    return grid_widget;
}

void
GUIBuilder::doGridLayout( policylib::gui::Grid* root, QWidget* widget )
{
    QGridLayout* grid_layout = new QGridLayout;
    //grid_layout->setHorizontalSpacing(0);
    //grid_layout->setVerticalSpacing(0);
    //grid_layout->setContentsMargins( 0, 0, 0, 0 );
    widget->setLayout( grid_layout );
    addChildren( static_cast<policylib::gui::Container2D<policylib::gui::Element>*>( root ),
                 widget,
                 grid_layout );

}



QWidget*
GUIBuilder::addElementGroup(policylib::gui::ElementGroup *root, QWidget *parent)
{
    QGroupBox* widget = new QGroupBox( parent );
    widget->setFlat( true );

    const std::string key = root->key();
    if( root->showLabel() && !root->key().empty() ) {
        widget->setTitle( (prettyName( root->key(), m_policyLib ) ).c_str() );
    }

    policylib::gui::Element* child = root->child();
    if( child != NULL ) {
        if( child->type() == policylib::gui::GRID ) {
            doGridLayout( static_cast<policylib::gui::Grid*>( child ), widget );
        }
        else if( child->type() == policylib::gui::HORIZONTAL_LAYOUT ) {
            doHorizontalLayout( static_cast<policylib::gui::HorizontalLayout*>( child ), widget );
        }
        else if( child->type() == policylib::gui::VERTICAL_LAYOUT ) {
            doVerticalLayout( static_cast<policylib::gui::VerticalLayout*>( child ), widget );
        }
        else {
            QHBoxLayout* layout = new QHBoxLayout( widget );
            widget->setLayout( layout );
            addChildren( static_cast<policylib::gui::Container0D<policylib::gui::Element>*>( root ),
                         widget,
                         layout );
        }
    }
    return widget;
}


QWidget*
GUIBuilder::addCanvas(policylib::gui::Canvas *root, QWidget *parent )
{
    QWidget* wrapper = NULL;

    if( m_root_context == NULL ) {
        throw std::runtime_error( "Only GL jobs can create canvases." );
    }

    if( m_observer->perfMode() || m_observer->renderListMode() ) {
        wrapper = new QWidget( parent );

        QWidget* row = new QWidget( wrapper );
        QHBoxLayout* row_layout = new QHBoxLayout( row );
        row->setLayout( row_layout );
        row->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        row_layout->setContentsMargins( 0, 0, 0, 0 );

        Canvas * canvas = new Canvas( static_cast<jobobserver::OpenGLJob*>(m_job),
                                      root->key(),
                                      root->boundingBoxKey(),
                                      m_policyLib,
                                      wrapper,
                                      m_root_context,
                                      true );
        canvas->setMinimumSize(640, 360);
        canvas->setMaximumSize(2000, 2000);
        canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        if( m_observer->renderListMode() ) {
            QComboBox* mode = new QComboBox( row );
            mode->addItem( "Native Rendering" );
            mode->addItem( "Render List" );
            row->layout()->addWidget( mode );
            QObject::connect( mode, SIGNAL(currentIndexChanged(int)),
                              canvas, SLOT(setRenderMode(int)) );
        }
        if( m_observer->perfMode() ) {
            QLabel* fps_label = new QLabel( row );
            fps_label->setText( "fps" );
            QObject::connect( canvas, SIGNAL(updateFPS(QString)), fps_label, SLOT(setText(QString)));
            row->layout()->addWidget(fps_label);
        }
        row_layout->addStretch( 1 );

        QVBoxLayout* wrapper_layout = new QVBoxLayout(wrapper);
        wrapper->setLayout( wrapper_layout );
        wrapper_layout->setSpacing( 0 );
        wrapper->setContentsMargins( 0, 0, 0, 0 );

        wrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        wrapper->layout()->addWidget(canvas );
        wrapper->layout()->addWidget( row );
    }
    else {
        Canvas * canvas = new Canvas( static_cast<jobobserver::OpenGLJob*>(m_job),
                                      root->key(),
                                      root->boundingBoxKey(),
                                      m_policyLib,
                                      parent,
                                      m_root_context,
                                      false );
        canvas->setMinimumSize(640, 360);
        canvas->setMaximumSize(2000, 2000);
        canvas->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        wrapper = canvas;
    }
    return wrapper;
}


QWidget*
GUIBuilder::addCombobox(policylib::gui::ComboBox *root, QWidget *parent)
{
    QWidget* w = new ComboBox(root->key(), m_policyLib, parent);
    w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    return w;
}

QWidget*
GUIBuilder::addRadiobuttons(policylib::gui::RadioButtons *root, QWidget *parent)
{
    QWidget* w = new RadioButtonGroup(root->key(), m_policyLib,  root->horizontal(), parent);
    w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    return w;
}


QWidget*
GUIBuilder::addSpinBox(policylib::gui::SpinBox *root, QWidget *parent)
{
   QWidget* w = new SpinBox(root->key(), m_policyLib, parent);
   w->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
   return w;
}

QWidget*
GUIBuilder::addCheckBox(policylib::gui::CheckBox *root, QWidget *parent)
{
   return new CheckBox(root->key(), m_policyLib, parent);
}

QWidget*
GUIBuilder::addButton(policylib::gui::Button *root, QWidget *parent)
{
    return new Button(root->key(), m_policyLib, parent);
}

QWidget*
GUIBuilder::addHorizontalSlider(policylib::gui::HorizontalSlider *root, QWidget *parent)
{
   return new HorizontalSlider(root->key(), root->withButtons(),
                                          m_policyLib, parent);
}



QWidget*
GUIBuilder::addDoubleSpinBox(policylib::gui::DoubleSpinBox *root, QWidget *parent)
{
    return new DoubleSpinBox(root->key(), m_policyLib, parent);
}

void
GUIBuilder::addChildren( policylib::gui::Container0D<policylib::gui::Element>*  container,
                         QWidget*                                               widget,
                         QBoxLayout*                                            layout )
{
    policylib::gui::Element* child = container->child();
    QWidget* child_widget = buildGUI( child, widget );
    if( child_widget != NULL ) {
        if( child->type() == policylib::gui::CANVAS ) {
            layout->addWidget( child_widget, 1 );
        }
        else {
            layout->addWidget( child_widget );
        }
    }
    else {
        std::cerr << __FILE__ << '@' << __LINE__<< ": ERROR: got nullptr.\n";
    }
}

void
GUIBuilder::addChildren( policylib::gui::Container1D<policylib::gui::Element>*  container,
                         QWidget*                                               widget,
                         QBoxLayout*                                            layout )
{
    for( size_t i=0; i<container->children(); i++ ) {
        policylib::gui::Element* child = container->child( i );
        if( child == NULL ) {
            continue;
        }
        QLayoutItem* layout_item = buildLayoutItem( child );
        if( layout_item != NULL ) {
            layout->addItem( layout_item );
            continue;
        }
        QWidget* child_widget = buildGUI( child, widget );
        if( child_widget != NULL ) {
            if( child->type() == policylib::gui::CANVAS ) {
                layout->addWidget( child_widget, 1 );
            }
            else {
                layout->addWidget( child_widget );
            }
        }
        else {
            std::cerr << __FILE__ << '@' << __LINE__<< ": ERROR: got nullptr.\n";
        }
    }
}

void
GUIBuilder::addChildren( policylib::gui::Container2D<policylib::gui::Element>* container,
                         QWidget* widget,
                         QGridLayout* layout )
{
    for(size_t row = 0; row < container->height(); row++) {
       for(size_t col = 0; col  < container->width(); col++) {

           policylib::gui::Element* child = container->child(row, col);
           if( child == NULL ) {
               continue;
           }
           QLayoutItem* layout_item = buildLayoutItem( child );
           if( layout_item != NULL ) {
               layout->addItem( layout_item, row, col );
               continue;
           }

           QWidget* child_widget = buildGUI( child, widget );
           if( child_widget != NULL ) {
               if( child->type() == policylib::gui::CANVAS ) {
                   layout->setColumnStretch( col, 1 );
                   layout->setRowStretch( row, 1 );
               }
               else if( child->type() == policylib::gui::GRID ) {
                   child_widget->layout()->setContentsMargins(0, 0, 0, 0);
               }
               else if( child->type() == policylib::gui::VERTICAL_LAYOUT ) {
                   child_widget->layout()->setContentsMargins(0, 0, 0, 0);
               }
               else if( child->type() == policylib::gui::HORIZONTAL_LAYOUT ) {
                   child_widget->layout()->setContentsMargins(0, 0, 0, 0);
               }
               layout->addWidget( child_widget, row, col );
           }
           else {
               std::cerr << __FILE__ << '@' << __LINE__<< ": ERROR: got nullptr.\n";
           }
       }
    }
}


}

QWidget * qtobserver::GUIBuilder::addFileDialogButton(policylib::gui::FileDialogButton *root, QWidget *parent)
{

   FileDialogButton *widget =  new FileDialogButton(root->key(), root->showValue(),
                                                    m_policyLib, parent);
   return widget;
}

} // of namespace tinia
