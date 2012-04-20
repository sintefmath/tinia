#include "tinia/qtobserver/moc/DoubleSpinBox.hpp"

namespace tinia {
namespace qtobserver {
DoubleSpinBox::DoubleSpinBox(std::string key, std::shared_ptr<policy::Policy> policy,
                 QWidget *parent) :
   QDoubleSpinBox(parent), m_key(key.c_str()), m_policy(policy)
{
   // Get max and min
   policy::StateSchemaElement element = m_policy->getStateSchemaElement(m_key);
   try
   {
      double max = boost::lexical_cast<double>(element.getMaxConstraint());
      double min = boost::lexical_cast<double>(element.getMinConstraint());
      setMaximum(max);
      setMinimum(min);
   }
   catch(boost::bad_lexical_cast* exception)
   {
      // Do nothing
   }

   m_policy->addStateListener(m_key, this);
   double value;
   m_policy->getElementValue<double>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(double)), this,
           SLOT(setValue(double)));
   connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueSetFromQt(double)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
DoubleSpinBox::~DoubleSpinBox()
{
   m_policy->removeStateListener(m_key, this);
}
}

void qtobserver::DoubleSpinBox::stateElementModified(policy::StateElement *stateElement)
{
   double value;
   m_policy->getElementValue<double>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::DoubleSpinBox::valueSetFromQt(double val)
{
   m_policy->updateElement(m_key, val);
}
}

