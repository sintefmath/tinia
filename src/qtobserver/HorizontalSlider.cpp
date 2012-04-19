#include "qtobserver/moc/HorizontalSlider.hpp"
#include <QHBoxLayout>
#include <boost/lexical_cast.hpp>
namespace qtobserver {
/**
  Used templated min max and removed boos lexical cast
  */
HorizontalSlider::HorizontalSlider(std::string key, bool withButtons,
                                   std::shared_ptr<policylib::PolicyLib> policyLib,
                                   QWidget *parent) :
    QWidget(parent), m_key(key), m_policyLib(policyLib)
{
   m_slider = new QSlider(Qt::Horizontal, parent);
   // Get max and min
   policylib::StateSchemaElement element = m_policyLib->getStateSchemaElement(m_key);
   int max = boost::lexical_cast<int>(element.getMaxConstraint());
   int min = boost::lexical_cast<int>(element.getMinConstraint());
   m_slider->setMaximum(max);
   m_slider->setMinimum(min);


   int value;
   m_policyLib->getElementValue<int>(m_key, value);
   m_slider->setValue(value);


   m_slider->setTickPosition(QSlider::TicksBelow);
   m_slider->setTickInterval((max-min)/20.);


   m_policyLib->addStateListener(m_key, this);

   setLayout(new QHBoxLayout(this));
   layout()->addWidget(m_slider);
   layout()->setContentsMargins( 0, 0, 0, 0 );


   if(withButtons)
   {
      addButtons();
   }

   // Do signals
   connect(this, SIGNAL(setValueFromPolicylib(int)), m_slider,
           SLOT(setValue(int)));

   connect(m_slider, SIGNAL(valueChanged(int)), this,
           SLOT(setValueFromQt(int)));

   // We want this large, but not too large
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   setMinimumWidth(200);

}
HorizontalSlider::~HorizontalSlider()
{
   m_policyLib->removeStateListener(m_key, this);
}

}

void qtobserver::HorizontalSlider::addButtons()
{

}

void qtobserver::HorizontalSlider::stateElementModified(
      policylib::StateElement *stateElement)
{
   int value;
   stateElement->getValue(value);
   emit setValueFromPolicylib(value);
}

void qtobserver::HorizontalSlider::setValueFromQt(int value)
{
   m_policyLib->updateElement<int>(m_key, value);
}
