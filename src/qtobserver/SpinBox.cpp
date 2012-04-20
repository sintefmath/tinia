#include "tinia/qtobserver/moc/SpinBox.hpp"

namespace tinia {
namespace qtobserver {
SpinBox::SpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                 QWidget *parent) :
   QSpinBox(parent), m_key(key.c_str()), m_policy(policy)
{
   // Get max and min
   policy::StateSchemaElement element = m_policy->getStateSchemaElement(m_key);
   try {
	int maxValue = boost::lexical_cast<int>(element.getMaxConstraint());
	int minValue = boost::lexical_cast<int>(element.getMinConstraint());
	setMaximum(maxValue);
	setMinimum(minValue);
   } catch(...) {
	   std::cerr<< "WARNING: Using SpinBox without max/min"<<std::endl;

   }


   m_policy->addStateListener(m_key, this);
   int value;
   m_policy->getElementValue<int>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(int)), this,
           SLOT(setValue(int)));
   connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueSetFromQt(int)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
SpinBox::~SpinBox()
{
   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::SpinBox::stateElementModified(policy::StateElement *stateElement)
{
   int value;
   m_policy->getElementValue<int>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::SpinBox::valueSetFromQt(int val)
{
   m_policy->updateElement(m_key, val);
}

} // of namespace tinia
