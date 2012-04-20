#ifndef QTOBSERVER_ELEMENTGROUP_HPP
#define QTOBSERVER_ELEMENTGROUP_HPP

#include <QGroupBox>
#include "tinia/policylib/PolicyLib.hpp"
#include "tinia/policylib/StateListener.hpp"
#include <memory>

namespace qtobserver {

class ElementGroup : public QGroupBox, public policylib::StateListener
{
    Q_OBJECT
public:
    explicit ElementGroup(std::string key, bool showLabel,
                          std::shared_ptr<policylib::PolicyLib> policyLib,
                          QWidget *parent = 0);
   ~ElementGroup();
   void stateElementModified(policylib::StateElement *stateElement);

signals:
   void setVisibleFromPolicyLib(bool visible);
public slots:

private:
   std::string m_key;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
};

} // namespace qtobserver

#endif // QTOBSERVER_ELEMENTGROUP_HPP
