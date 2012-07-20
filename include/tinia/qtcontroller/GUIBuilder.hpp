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
#include "tinia/model/GUILayout.hpp"
#include "tinia/model/ExposedModel.hpp"
#include "QTController.hpp"
#include "tinia/jobcontroller/Job.hpp"
#include <memory>
#include <QWidget>
#include <QTabWidget>
#include <QBoxLayout>
#include <QLayoutItem>

class QGLWidget;

namespace tinia {
namespace qtcontroller {

class GUIBuilder
{
public:
    GUIBuilder(std::shared_ptr<model::ExposedModel> model,
               jobcontroller::Job* job,
               QTController* controller,
               const bool perf_mode,
               QGLWidget* root_context );
    /**
      Builds the GUI specified by root.
      \param root the root element of the GUI
      \param parent the QWidget to put the GUI in. This assumes that parent
      has a layout.
      */
    QWidget* buildGUI(model::gui::Element* root, QWidget* parent );

    /** Adds the script defined in script to the javascript engine.
     * @note Must be called after QApplication is constructed.
     */
    void addScript(const std::string& script);

private:
    std::shared_ptr<model::ExposedModel>   m_model;
    jobcontroller::Job*                       m_job;
    QTController*                             m_controller;
    const bool                              m_perf_mode;
    QGLWidget*                              m_root_context;

    QWidget* addTabLayout(model::gui::TabLayout* root, QWidget* parent);
    QWidget* addGrid(model::gui::Grid* root, QWidget* parent );
    QWidget* addCanvas(model::gui::Canvas* root, QWidget* parent );
    QWidget* addCombobox(model::gui::ComboBox* root, QWidget* parent);
    QWidget* addRadiobuttons(model::gui::RadioButtons* root, QWidget* parent);
    QWidget* addElementGroup(model::gui::ElementGroup* root, QWidget* parent);
    QWidget* addSpinBox(model::gui::SpinBox* root, QWidget* parent);
    QWidget* addCheckBox(model::gui::CheckBox* root, QWidget* parent);
    QWidget* addButton(model::gui::Button* root, QWidget* parent);
    QWidget* addHorizontalSlider(model::gui::HorizontalSlider* root, QWidget* parent);
    QWidget* addHorizontalLayout(model::gui::HorizontalLayout* root, QWidget* parent);
    QWidget* addVerticalLayout(model::gui::VerticalLayout* root, QWidget* parent);
    QWidget* addDoubleSpinBox(model::gui::DoubleSpinBox* root, QWidget* parent);
    QWidget* addHorizontalSpace(model::gui::HorizontalSpace* root, QWidget* parent);
    QWidget* addVerticalSpace(model::gui::VerticalSpace* root, QWidget* parent);
    QWidget* addHorizontalExpandingSpace(model::gui::HorizontalExpandingSpace* root, QWidget* parent);
    QWidget* addVerticalExpandingSpace(model::gui::VerticalExpandingSpace* root, QWidget* parent);
    QWidget* addPopupButton( model::gui::PopupButton* root, QWidget* parent );
    QWidget* addFileDialogButton( model::gui::FileDialogButton* root, QWidget* parent);

    void
    doGridLayout( model::gui::Grid* root, QWidget* widget );

    void
    doHorizontalLayout( model::gui::HorizontalLayout* root, QWidget* widget );

    void
    addChildren( model::gui::Container0D<model::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( model::gui::Container1D<model::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( model::gui::Container2D<model::gui::Element>* container,
                 QWidget* widget,
                 QGridLayout* layout );

    void
    doVerticalLayout( model::gui::VerticalLayout* root, QWidget* widget );

    QLayoutItem*
    buildLayoutItem( model::gui::Element* root );

};

}
}

