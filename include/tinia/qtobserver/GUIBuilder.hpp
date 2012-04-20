#ifndef QTOBSERVER_GUIBUILDER_HPP
#define QTOBSERVER_GUIBUILDER_HPP
#include "tinia/policylib/GUILayout.hpp"
#include "tinia/policylib/PolicyLib.hpp"
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
    GUIBuilder(std::shared_ptr<policylib::PolicyLib> policyLib,
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
    QWidget* buildGUI(policylib::gui::Element* root, QWidget* parent );

private:
    std::shared_ptr<policylib::PolicyLib>   m_policyLib;
    jobobserver::Job*                       m_job;
    QTObserver*                             m_observer;
    const bool                              m_perf_mode;
    QGLWidget*                              m_root_context;

    QWidget* addTabLayout(policylib::gui::TabLayout* root, QWidget* parent);
    QWidget* addGrid(policylib::gui::Grid* root, QWidget* parent );
    QWidget* addCanvas(policylib::gui::Canvas* root, QWidget* parent );
    QWidget* addCombobox(policylib::gui::ComboBox* root, QWidget* parent);
    QWidget* addRadiobuttons(policylib::gui::RadioButtons* root, QWidget* parent);
    QWidget* addElementGroup(policylib::gui::ElementGroup* root, QWidget* parent);
    QWidget* addSpinBox(policylib::gui::SpinBox* root, QWidget* parent);
    QWidget* addCheckBox(policylib::gui::CheckBox* root, QWidget* parent);
    QWidget* addButton(policylib::gui::Button* root, QWidget* parent);
    QWidget* addHorizontalSlider(policylib::gui::HorizontalSlider* root, QWidget* parent);
    QWidget* addHorizontalLayout(policylib::gui::HorizontalLayout* root, QWidget* parent);
    QWidget* addVerticalLayout(policylib::gui::VerticalLayout* root, QWidget* parent);
    QWidget* addDoubleSpinBox(policylib::gui::DoubleSpinBox* root, QWidget* parent);
    QWidget* addHorizontalSpace(policylib::gui::HorizontalSpace* root, QWidget* parent);
    QWidget* addVerticalSpace(policylib::gui::VerticalSpace* root, QWidget* parent);
    QWidget* addHorizontalExpandingSpace(policylib::gui::HorizontalExpandingSpace* root, QWidget* parent);
    QWidget* addVerticalExpandingSpace(policylib::gui::VerticalExpandingSpace* root, QWidget* parent);
    QWidget* addPopupButton( policylib::gui::PopupButton* root, QWidget* parent );
    QWidget* addFileDialogButton( policylib::gui::FileDialogButton* root, QWidget* parent);

    void
    doGridLayout( policylib::gui::Grid* root, QWidget* widget );

    void
    doHorizontalLayout( policylib::gui::HorizontalLayout* root, QWidget* widget );

    void
    addChildren( policylib::gui::Container0D<policylib::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( policylib::gui::Container1D<policylib::gui::Element>* container,
                 QWidget* widget,
                 QBoxLayout* layout );

    void
    addChildren( policylib::gui::Container2D<policylib::gui::Element>* container,
                 QWidget* widget,
                 QGridLayout* layout );

    void
    doVerticalLayout( policylib::gui::VerticalLayout* root, QWidget* widget );

    QLayoutItem*
    buildLayoutItem( policylib::gui::Element* root );

};

}
}
#endif // QTOBSERVER_GUIBUILDER_HPP
