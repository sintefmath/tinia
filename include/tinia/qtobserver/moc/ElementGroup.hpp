#ifndef QTOBSERVER_ELEMENTGROUP_HPP
#define QTOBSERVER_ELEMENTGROUP_HPP

#include <QGroupBox>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include <memory>

namespace tinia {
namespace qtobserver {

class ElementGroup : public QGroupBox, public policy::StateListener
{
    Q_OBJECT
public:
    explicit ElementGroup(std::string key, bool showLabel,
                          std::shared_ptr<policy::Policy> policy,
                          QWidget *parent = 0);
   ~ElementGroup();
   void stateElementModified(policy::StateElement *stateElement);

signals:
   void setVisibleFromPolicy(bool visible);
public slots:

private:
   std::string m_key;
   std::shared_ptr<policy::Policy> m_policy;
};

} // namespace qtobserver
} // namespace tinia
#endif // QTOBSERVER_ELEMENTGROUP_HPP
