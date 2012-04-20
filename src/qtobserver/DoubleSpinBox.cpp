#include "tinia/qtobserver/moc/DoubleSpinBox.hpp"

namespace tinia {
namespace qtobserver {
DoubleSpinBox::DoubleSpinBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                 QWidget *parent) :
   QDoubleSpinBox(parent), m_key(key.c_str()), m_policyLib(policyLib)
{
   // Get max and min
   policylib::StateSchemaElement element = m_policyLib->getStateSchemaElement(m_key);
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

   m_policyLib->addStateListener(m_key, this);
   double value;
   m_policyLib->getElementValue<double>(m_key, value);
   setValue(value);
   connect(this, SIGNAL(setValueFromPolicy(double)), this,
           SLOT(setValue(double)));
   connect(this, SIGNAL(valueChanged(double)), this, SLOT(valueSetFromQt(double)));
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

}
DoubleSpinBox::~DoubleSpinBox()
{
   m_policyLib->removeStateListener(m_key, this);
}
}

void qtobserver::DoubleSpinBox::stateElementModified(policylib::StateElement *stateElement)
{
   double value;
   m_policyLib->getElementValue<double>(m_key, value);
   emit setValueFromPolicy(value);
}

void qtobserver::DoubleSpinBox::valueSetFromQt(double val)
{
   m_policyLib->updateElement(m_key, val);
}
}

