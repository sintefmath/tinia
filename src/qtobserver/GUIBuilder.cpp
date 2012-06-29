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
#include "tinia/qtobserver/impl/utils.hpp"
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
using namespace tinia::qtobserver::impl;
namespace tinia {
namespace qtobserver {

GUIBuilder::GUIBuilder( std::shared_ptr<model::ExposedModel>   model,
                        jobobserver::Job*                       job,
                        QTObserver*                             observer,
                        const bool                              perf_mode,
                        QGLWidget*                              root_context )
   : m_model(model),
     m_job(job),
     m_observer( observer ),
     m_perf_mode( perf_mode ),
     m_root_context( root_context )
{
}


QWidget* GUIBuilder::buildGUI(model::gui::Element* root, QWidget *parent )
{
    QWidget* widget = NULL;

    if( widget == NULL ) {
        model::gui::TabLayout* tab_layout = dynamic_cast<model::gui::TabLayout*>( root );
        if( tab_layout != NULL ) {
            widget = addTabLayout( tab_layout, parent );
        }
    }
    if( widget == NULL ) {
        model::gui::Grid* grid = dynamic_cast<model::gui::Grid*>( root );
        if( grid != NULL ) {
            widget = addGrid( grid, parent );
        }
    }
    if( widget == NULL ) {
        model::gui::TextInput* text_input = dynamic_cast<model::gui::TextInput*>( root );
        if( text_input != NULL ) {
            widget = new QLineEdit( parent );
            new StringController( widget,
                                  m_model,
                                  text_input->key(),
                                  true );
        }
    }
    if( widget == NULL ) {
        model::gui::Canvas* canvas = dynamic_cast<model::gui::Canvas*>( root );
        if( canvas != NULL ) {
            widget = addCanvas( canvas, parent );
        }
    }
    if( widget == NULL ) {
        model::gui::Label* label = dynamic_cast<model::gui::Label*>( root );
        if( label != NULL ) {
            widget = new QLabel( parent );
            new StringController( widget,
                                  m_model,
                                  label->key(),
                                  label->showValue() );
        }
    }
    if( widget == NULL ) {
        model::gui::ComboBox* combobox = dynamic_cast<model::gui::ComboBox*>( root );
        if( combobox != NULL ) {
            widget = addCombobox( combobox, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::ElementGroup* element_group = dynamic_cast<model::gui::ElementGroup*>( root );
        if( element_group != NULL ) {
            widget = addElementGroup( element_group, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::RadioButtons* radio_buttons = dynamic_cast<model::gui::RadioButtons*>( root );
        if( radio_buttons != NULL ) {
            widget = addRadiobuttons( radio_buttons, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::SpinBox* spin_box = dynamic_cast<model::gui::SpinBox*>( root );
        if( spin_box != NULL ) {
            widget = addSpinBox( spin_box, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::CheckBox* check_box = dynamic_cast<model::gui::CheckBox*>( root );
        if( check_box != NULL ) {
            widget = addCheckBox( check_box, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::Button* button = dynamic_cast<model::gui::Button*>( root );
        if( button != NULL ) {
            widget = addButton( button, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::HorizontalSlider* horizontal_slider = dynamic_cast<model::gui::HorizontalSlider*>( root );
        if( horizontal_slider != NULL ) {
            widget = addHorizontalSlider( horizontal_slider, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::HorizontalLayout* horizontal_layout = dynamic_cast<model::gui::HorizontalLayout*>( root );
        if( horizontal_layout != NULL ) {
            widget = addHorizontalLayout( horizontal_layout, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::VerticalLayout* vertical_layout = dynamic_cast<model::gui::VerticalLayout*>( root );
        if( vertical_layout != NULL ) {
            widget = addVerticalLayout( vertical_layout, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::DoubleSpinBox* double_spinbox = dynamic_cast<model::gui::DoubleSpinBox*>( root );
        if( double_spinbox != NULL ) {
            widget = addDoubleSpinBox( double_spinbox, parent);
        }
    }
    if( widget == NULL ) {
        model::gui::PopupButton* popup_button = dynamic_cast<model::gui::PopupButton*>( root );
        if( popup_button != NULL ) {
            widget = addPopupButton( popup_button, parent );
        }
    }
    if(widget == NULL ) {
       model::gui::FileDialogButton* file_dialog_button = dynamic_cast<model::gui::FileDialogButton*>( root );
       if(file_dialog_button != NULL)
       {
          widget = addFileDialogButton(file_dialog_button, parent);
       }
    }

    if( widget != NULL ) {

        if( !root->visibilityKey().empty() ) {
            new VisibilityController( widget,
                                      m_model,
                                      root->visibilityKey(),
                                      root->visibilityInverted() );
        }
        if( !root->enabledKey().empty() ) {
            new EnabledController( widget,
                                   m_model,
                                   root->enabledKey(),
                                   root->enabledInverted() );
        }
    }

    return widget;
}

QWidget*
GUIBuilder::addPopupButton( model::gui::PopupButton* root, QWidget* parent )
{
    QString suffix( " " );
    suffix.append( QChar( 0x02ec ) );
    QPushButton* widget = new QPushButton( parent );
    widget->setCheckable( true );
    new impl::StringController( widget, m_model, root->key(),root->showValue(), suffix );

    QWidget* foo = buildGUI( root->child(), widget );
    if( foo != NULL ) {
        foo->setObjectName( "popup" );
        foo->setStyleSheet( "QWidget#popup { border: 1px solid black }");
        new tinia::qtobserver::impl::PopupEventFilter( foo, widget );
    }
    return widget;
}



QWidget*
GUIBuilder::addTabLayout(model::gui::TabLayout *root, QWidget *parent)
{
    QTabWidget* tabWidget = new QTabWidget(parent);

    for( size_t i=0; i<root->children(); i++ ) {
        model::gui::Tab* tab = root->child( i );
        if( tab != NULL ) {
            QWidget* child = buildGUI( tab->child(), tabWidget );
            if( child != NULL ) {
                tabWidget->addTab( child,
                                   QString(prettyName(tab->key(), m_model).c_str()) );
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
GUIBuilder::addHorizontalLayout(model::gui::HorizontalLayout *root, QWidget *parent)
{
    QWidget* widget = new QWidget(parent);
    doHorizontalLayout( root, widget );
    return widget;
}

QWidget*
GUIBuilder::addVerticalLayout(model::gui::VerticalLayout *root, QWidget *parent)
{
    QWidget* widget = new QWidget(parent);
    doVerticalLayout( root, widget );
    return widget;
}

void
GUIBuilder::doHorizontalLayout(model::gui::HorizontalLayout *root, QWidget *widget)
{
    QHBoxLayout* layout = new QHBoxLayout( widget );
    widget->setLayout( layout );
    widget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    addChildren( static_cast<model::gui::Container1D<model::gui::Element>*>( root ),
                 widget,
                 layout );
}

void
GUIBuilder::doVerticalLayout(model::gui::VerticalLayout *root, QWidget *widget)
{
    QVBoxLayout* layout = new QVBoxLayout( widget );
    widget->setLayout( layout );
    widget->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    addChildren( static_cast<model::gui::Container1D<model::gui::Element>*>( root ),
                 widget,
                 layout );
}






QLayoutItem*
GUIBuilder::buildLayoutItem( model::gui::Element* root )
{
    model::gui::VerticalSpace* vspace = dynamic_cast<model::gui::VerticalSpace*>( root );
    if( vspace != NULL ) {
        return new QSpacerItem(0, 10, QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
    model::gui::VerticalExpandingSpace* vexspace = dynamic_cast<model::gui::VerticalExpandingSpace*>( root );
    if( vexspace != NULL ) {
        return new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
    }

    model::gui::HorizontalSpace* hspace = dynamic_cast<model::gui::HorizontalSpace*>( root );
    if( hspace != NULL ) {
        return new QSpacerItem(10, 0, QSizePolicy::Minimum, QSizePolicy::Minimum);
    }
    model::gui::HorizontalExpandingSpace* hexspace = dynamic_cast<model::gui::HorizontalExpandingSpace*>( root );
    if( hexspace != NULL ) {
        return new QSpacerItem(0, 0, QSizePolicy::MinimumExpanding, QSizePolicy::Minimum );
    }
    return NULL;
}


QWidget*
GUIBuilder::addGrid(model::gui::Grid *root, QWidget *parent)
{
    QWidget* grid_widget = new QWidget(parent);
    doGridLayout( root, grid_widget );
    return grid_widget;
}

void
GUIBuilder::doGridLayout( model::gui::Grid* root, QWidget* widget )
{
    QGridLayout* grid_layout = new QGridLayout;
    //grid_layout->setHorizontalSpacing(0);
    //grid_layout->setVerticalSpacing(0);
    //grid_layout->setContentsMargins( 0, 0, 0, 0 );
    widget->setLayout( grid_layout );
    addChildren( static_cast<model::gui::Container2D<model::gui::Element>*>( root ),
                 widget,
                 grid_layout );

}



QWidget*
GUIBuilder::addElementGroup(model::gui::ElementGroup *root, QWidget *parent)
{
    QGroupBox* widget = new QGroupBox( parent );
    widget->setFlat( true );

    const std::string key = root->key();
    if( root->showLabel() && !root->key().empty() ) {
        widget->setTitle( (prettyName( root->key(), m_model ) ).c_str() );
    }

    model::gui::Element* child = root->child();
    if( child != NULL ) {
        if( child->type() == model::gui::GRID ) {
            doGridLayout( static_cast<model::gui::Grid*>( child ), widget );
        }
        else if( child->type() == model::gui::HORIZONTAL_LAYOUT ) {
            doHorizontalLayout( static_cast<model::gui::HorizontalLayout*>( child ), widget );
        }
        else if( child->type() == model::gui::VERTICAL_LAYOUT ) {
            doVerticalLayout( static_cast<model::gui::VerticalLayout*>( child ), widget );
        }
        else {
            QHBoxLayout* layout = new QHBoxLayout( widget );
            widget->setLayout( layout );
            addChildren( static_cast<model::gui::Container0D<model::gui::Element>*>( root ),
                         widget,
                         layout );
        }
    }
    return widget;
}


QWidget*
GUIBuilder::addCanvas(model::gui::Canvas *root, QWidget *parent )
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

        tinia::qtobserver::impl::Canvas * canvas = new tinia::qtobserver::impl::Canvas( static_cast<jobobserver::OpenGLJob*>(m_job),
                                      root->key(),
                                      root->boundingBoxKey(),
                                      root->resetViewKey(),
                                      m_model,
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
        tinia::qtobserver::impl::Canvas * canvas = new tinia::qtobserver::impl::Canvas( static_cast<jobobserver::OpenGLJob*>(m_job),
                                      root->key(),
                                      root->boundingBoxKey(),
                                      root->resetViewKey(),
                                      m_model,
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
GUIBuilder::addCombobox(model::gui::ComboBox *root, QWidget *parent)
{
    QWidget* w = new ComboBox(root->key(), m_model, parent);
    w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    return w;
}

QWidget*
GUIBuilder::addRadiobuttons(model::gui::RadioButtons *root, QWidget *parent)
{
    QWidget* w = new RadioButtonGroup(root->key(), m_model,  root->horizontal(), parent);
    w->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
    return w;
}


QWidget*
GUIBuilder::addSpinBox(model::gui::SpinBox *root, QWidget *parent)
{
   QWidget* w = new SpinBox(root->key(), m_model, parent);
   w->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Minimum );
   return w;
}

QWidget*
GUIBuilder::addCheckBox(model::gui::CheckBox *root, QWidget *parent)
{
   return new CheckBox(root->key(), m_model, parent);
}

QWidget*
GUIBuilder::addButton(model::gui::Button *root, QWidget *parent)
{
    return new Button(root->key(), m_model, parent);
}

QWidget*
GUIBuilder::addHorizontalSlider(model::gui::HorizontalSlider *root, QWidget *parent)
{
   return new HorizontalSlider(root->key(), root->withButtons(),
                                          m_model, parent);
}



QWidget*
GUIBuilder::addDoubleSpinBox(model::gui::DoubleSpinBox *root, QWidget *parent)
{
    return new DoubleSpinBox(root->key(), m_model, parent);
}

void
GUIBuilder::addChildren( model::gui::Container0D<model::gui::Element>*  container,
                         QWidget*                                               widget,
                         QBoxLayout*                                            layout )
{
    model::gui::Element* child = container->child();
    QWidget* child_widget = buildGUI( child, widget );
    if( child_widget != NULL ) {
        if( child->type() == model::gui::CANVAS ) {
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
GUIBuilder::addChildren( model::gui::Container1D<model::gui::Element>*  container,
                         QWidget*                                               widget,
                         QBoxLayout*                                            layout )
{
    for( size_t i=0; i<container->children(); i++ ) {
        model::gui::Element* child = container->child( i );
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
            if( child->type() == model::gui::CANVAS ) {
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
GUIBuilder::addChildren( model::gui::Container2D<model::gui::Element>* container,
                         QWidget* widget,
                         QGridLayout* layout )
{
    for(size_t row = 0; row < container->height(); row++) {
       for(size_t col = 0; col  < container->width(); col++) {

           model::gui::Element* child = container->child(row, col);
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
               if( child->type() == model::gui::CANVAS ) {
                   layout->setColumnStretch( col, 1 );
                   layout->setRowStretch( row, 1 );
               }
               else if( child->type() == model::gui::GRID ) {
                   child_widget->layout()->setContentsMargins(0, 0, 0, 0);
               }
               else if( child->type() == model::gui::VERTICAL_LAYOUT ) {
                   child_widget->layout()->setContentsMargins(0, 0, 0, 0);
               }
               else if( child->type() == model::gui::HORIZONTAL_LAYOUT ) {
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

QWidget * GUIBuilder::addFileDialogButton(model::gui::FileDialogButton *root, QWidget *parent)
{

   FileDialogButton *widget =  new FileDialogButton(root->key(), root->showValue(),
                                                    m_model, parent);
   return widget;
}


}
} // of namespace tinia
