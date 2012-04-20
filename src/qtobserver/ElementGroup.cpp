#include "tinia/qtobserver/moc/ElementGroup.hpp"
#include "tinia/qtobserver/utils.hpp"
#include <QHBoxLayout>

namespace tinia {
namespace qtobserver {

ElementGroup::ElementGroup(std::string key, bool showLabel,
                           std::shared_ptr<policylib::PolicyLib> policyLib,
                           QWidget *parent) :
    QGroupBox(parent), m_key(key), m_policyLib(policyLib)
{
/*

    //   setLayout(new QHBoxLayout(this));
   m_policyLib->addStateListener(m_key, this);
   connect(this, SIGNAL(setVisibleFromPolicyLib(bool)), this,
           SLOT(setVisible(bool)));
   if(showLabel)
   {
      setTitle(prettyName(m_key, m_policyLib).c_str());
   }

   // Should we show ourselves?
   bool show;
   m_policyLib->getElementValue(m_key, show);
   if(!show)
   {
      setVisible(false);
   }
*/
   // We don't want frames
   setFlat(true);

}

ElementGroup::~ElementGroup()
{
//   m_policyLib->removeStateListener(m_key, this);
}
}

void qtobserver::ElementGroup::stateElementModified(policylib::StateElement *stateElement)
{
/*
    bool visible;
   stateElement->getValue(visible);
   emit setVisibleFromPolicyLib(visible);
*/
}

} // of namespace tinia

