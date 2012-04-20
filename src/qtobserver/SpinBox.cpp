#include "tinia/qtobserver/moc/SpinBox.hpp"

namespace tinia {
namespace qtobserver {
SpinBox::SpinBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                 QWidget *parent) :
   QSpinBox(parent), m_key(key.c_str()), m_policyLib(policyLib)
{
   // Get max and min
   policylib::StateSchemaElement element = m_policyLib->getStateSchemaElement(m_key);
   try {
	int maxValue = boost::lexical_cast<int>(element.getMaxConstraint());
	int minValue = boost::lexical_cast<int>(element.getMinConstraint());
	setMaximum(maxValue);
	setMinimum(minValue);
   } catch(...) {
	   std::cerr<< "WARNING: Using SpinBox without max/min"<<std::endl;

   }


   m_policyLib->addStateListener(m_key, this);
   int value;
   m_policyLib->getElementValue<int>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(int)), this,
           SLOT(setValue(int)));
   connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueSetFromQt(int)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
SpinBox::~SpinBox()
{
   m_policyLib->removeStateListener(m_key, this);
}
}

void qtobserver::SpinBox::stateElementModified(policylib::StateElement *stateElement)
{
   int value;
   m_policyLib->getElementValue<int>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::SpinBox::valueSetFromQt(int val)
{
   m_policyLib->updateElement(m_key, val);
}

} // of namespace tinia
