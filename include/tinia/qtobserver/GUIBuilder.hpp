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

#ifndef QTOBSERVER_GUIBUILDER_HPP
#define QTOBSERVER_GUIBUILDER_HPP
#include "tinia/policy/GUILayout.hpp"
#include "tinia/policy/Policy.hpp"
#include "QTObserver.hpp"
#include "tinia/jobobserver/Job.hpp"
#include <memory>
#include <QWidget>
#include <QTabWidget>
#include <QBoxLayout>
#include <QLayoutItem>

class QGLWidget;

namespace tinia {
namespace qtobserver {

class GUIBuilder
{
public:
    GUIBuilder(std::shared_ptr<policy::Policy> policy,
               jobobserver::Job* job,
               QTObserver* observer,
               const bool perf_mode,
               QGLWidget* root_context );
    /**
      Builds the GUI specified by root.
      \param root the root element of the GUI
      \param parent the QWidget to put the GUI in. This assumes that parent
      has a layout.
      */
    QWidget* buildGUI(policy::gui::Element* root, QWidget* parent );

private:
    std::shared_ptr<policy::Policy>   m_policy;
    jobobserver::Job*                       m_job;
    QTObserver*                             m_observer;
    const bool                              m_perf_mode;
    QGLWidget*                              m_root_context;

    QWidget* addTabLayout(policy::gui::TabLayout* root, QWidget* parent);
    QWidget* addGrid(policy::gui::Grid* root, QWidget* parent );
    QWidget* addCanvas(policy::gui::Canvas* root, QWidget* parent );
    QWidget* addCombobox(policy::gui::ComboBox* root, QWidget* parent);
    QWidget* addRadiobuttons(policy::gui::RadioButtons* root, QWidget* parent);
    QWidget* addElementGroup(policy::gui::ElementGroup* root, QWidget* parent);
    QWidget* addSpinBox(policy::gui::SpinBox* root, QWidget* parent);
    QWidget* addCheckBox(policy::gui::CheckBox* root, QWidget* parent);
    QWidget* addButton(policy::gui::Button* root, QWidget* parent);
    QWidget* addHorizontalSlider(policy::gui::HorizontalSlider* root, QWidget* parent);
    QWidget* addHorizontalLayout(policy::gui::HorizontalLayout* root, QWidget* parent);
    QWidget* addVerticalLayout(policy::gui::VerticalLayout* root, QWidget* parent);
    QWidget* addDoubleSpinBox(policy::gui::DoubleSpinBox* root, QWidget* parent);
    QWidget* addHorizontalSpace(policy::gui::HorizontalSpace* root, QWidget* parent);
    QWidget* addVerticalSpace(policy::gui::VerticalSpace* root, QWidget* parent);
    QWidget* addHorizontalExpandingSpace(policy::gui::HorizontalExpandingSpace* root, QWidget* parent);
    QWidget* addVerticalExpandingSpace(policy::gui::VerticalExpandingSpace* root, QWidget* parent);
    QWidget* addPopupButton( policy::gui::PopupButton* root, QWidget* parent );
    QWidget* addFileDialogButton( policy::gui::FileDialogButton* root, QWidget* parent);

    void
    doGridLayout( policy::gui::Grid* root, QWidget* widget );

    void
    doHorizontalLayout( policy::gui::HorizontalLayout* root, QWidget* widget );

    void
    addChildren( policy::gui::Container0D<policy::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( policy::gui::Container1D<policy::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( policy::gui::Container2D<policy::gui::Element>* container,
                 QWidget* widget,
                 QGridLayout* layout );

    void
    doVerticalLayout( policy::gui::VerticalLayout* root, QWidget* widget );

    QLayoutItem*
    buildLayoutItem( policy::gui::Element* root );

};

}
}
#endif // QTOBSERVER_GUIBUILDER_HPP
