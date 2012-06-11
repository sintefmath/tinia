#ifndef QTOBSERVER_HORIZONTALSLIDER_HPP
#define QTOBSERVER_HORIZONTALSLIDER_HPP

#include <QWidget>
#include <QSlider>
#include <tinia/policy/Policy.hpp>
#include <tinia/policy/StateListener.hpp>
#include <tinia/policy/StateSchemaElement.hpp>
#include <tinia/policy/StateSchemaListener.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {

class HorizontalSlider : public QWidget, public policy::StateListener, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit HorizontalSlider(std::string key, bool withButtons,
                              std::shared_ptr<policy::Policy> policy,
                              QWidget *parent = 0);

   ~HorizontalSlider();
   void stateElementModified(policy::StateElement *stateElement);

   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
signals:
   void setValueFromPolicy(int value);
public slots:
   void setValueFromQt(int value);

private:
   void addButtons();
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;
   QSlider* m_slider;

};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_HORIZONTALSLIDER_HPP
