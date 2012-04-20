#include "tinia/qtobserver/moc/ElementGroup.hpp"
#include "tinia/qtobserver/utils.hpp"
#include <QHBoxLayout>

namespace tinia {
namespace qtobserver {

ElementGroup::ElementGroup(std::string key, bool showLabel,
                           std::shared_ptr<policy::Policy> policy,
                           QWidget *parent) :
    QGroupBox(parent), m_key(key), m_policy(policy)
{
/*

    //   setLayout(new QHBoxLayout(this));
   m_policy->addStateListener(m_key, this);
   connect(this, SIGNAL(setVisibleFromPolicy(bool)), this,
           SLOT(setVisible(bool)));
   if(showLabel)
   {
      setTitle(prettyName(m_key, m_policy).c_str());
   }

   // Should we show ourselves?
   bool show;
   m_policy->getElementValue(m_key, show);
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
//   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::ElementGroup::stateElementModified(policy::StateElement *stateElement)
{
/*
    bool visible;
   stateElement->getValue(visible);
   emit setVisibleFromPolicy(visible);
*/
}

} // of namespace tinia

