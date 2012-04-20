#ifndef QTOBSERVER_HORIZONTALSLIDER_HPP
#define QTOBSERVER_HORIZONTALSLIDER_HPP

#include <QWidget>
#include <QSlider>
#include <tinia/policylib/PolicyLib.hpp>
#include <tinia/policylib/StateListener.hpp>
#include <tinia/policylib/StateSchemaElement.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {

class HorizontalSlider : public QWidget, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit HorizontalSlider(std::string key, bool withButtons,
                              std::shared_ptr<policylib::PolicyLib> policyLib,
                              QWidget *parent = 0);

   ~HorizontalSlider();
   void stateElementModified(policylib::StateElement *stateElement);
signals:
   void setValueFromPolicylib(int value);
public slots:
   void setValueFromQt(int value);

private:
   void addButtons();
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   QSlider* m_slider;

};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_HORIZONTALSLIDER_HPP
