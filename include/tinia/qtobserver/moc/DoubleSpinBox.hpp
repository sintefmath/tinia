#ifndef QTOBSERVER_DOUBLESPINBOX_HPP
#define QTOBSERVER_DOUBLESPINBOX_HPP

#include <QDoubleSpinBox>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include "tinia/policy/StateSchemaListener.hpp"
#include <memory>

namespace tinia {
namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class DoubleSpinBox : public QDoubleSpinBox, public policy::StateListener, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit DoubleSpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                     QWidget *parent = 0);
   ~DoubleSpinBox();

   void stateElementModified(policy::StateElement *stateElement);
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
signals:
   void setValueFromPolicy(double val);
public slots:
   void valueSetFromQt(double val);

private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;
};
}
}
#endif // QTOBSERVER_DOUBLESPINBOX_HPP
