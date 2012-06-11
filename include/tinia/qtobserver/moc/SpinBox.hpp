#ifndef QTOBSERVER_SPINBOX_HPP
#define QTOBSERVER_SPINBOX_HPP

#include <QSpinBox>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include <tinia/policy/StateSchemaListener.hpp>
#include <memory>

namespace tinia {
namespace qtobserver {
/**
  \todo Use max and min if available.
  */
class SpinBox : public QSpinBox, public policy::StateListener, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit SpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                     QWidget *parent = 0);
    ~SpinBox();
    void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
    void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
    void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
    void stateElementModified(policy::StateElement *stateElement);

signals:
    void setValueFromPolicy(int val);
public slots:
    void valueSetFromQt(int val);

private:
    std::string m_key;
    std::shared_ptr<policy::Policy> m_policy;
};
}
}
#endif // QTOBSERVER_SPINBOX_HPP
