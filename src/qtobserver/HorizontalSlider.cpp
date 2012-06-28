/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tinia/qtobserver/moc/HorizontalSlider.hpp"
#include <QHBoxLayout>
#include <boost/lexical_cast.hpp>

namespace tinia {
namespace qtobserver {
/**
  Used templated min max and removed boos lexical cast
  */
HorizontalSlider::HorizontalSlider(std::string key, bool withButtons,
                                   std::shared_ptr<model::ExposedModel> model,
                                   QWidget *parent) :
    QWidget(parent), m_key(key), m_model(model)
{
   m_slider = new QSlider(Qt::Horizontal, parent);
   // Get max and min
   model::StateSchemaElement element = m_model->getStateSchemaElement(m_key);
   int max = boost::lexical_cast<int>(element.getMaxConstraint());
   int min = boost::lexical_cast<int>(element.getMinConstraint());
   m_slider->setMaximum(max);
   m_slider->setMinimum(min);


   int value;
   m_model->getElementValue<int>(m_key, value);
   m_slider->setValue(value);


   m_slider->setTickPosition(QSlider::TicksBelow);
   m_slider->setTickInterval((max-min)/20.);


   m_model->addStateListener(m_key, this);
   m_model->addStateSchemaListener(m_key, this);

   setLayout(new QHBoxLayout(this));
   layout()->addWidget(m_slider);
   layout()->setContentsMargins( 0, 0, 0, 0 );


   if(withButtons)
   {
      addButtons();
   }

   // Do signals
   connect(this, SIGNAL(setValueFromExposedModel(int)), m_slider,
           SLOT(setValue(int)));

   connect(m_slider, SIGNAL(valueChanged(int)), this,
           SLOT(setValueFromQt(int)));

   // We want this large, but not too large
   setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
   setMinimumWidth(200);

}
HorizontalSlider::~HorizontalSlider()
{
   m_model->removeStateListener(m_key, this);
   m_model->removeStateSchemaListener(m_key, this);
}

}

void qtobserver::HorizontalSlider::addButtons()
{

}

void qtobserver::HorizontalSlider::stateElementModified(
      model::StateElement *stateElement)
{
   int value;
   stateElement->getValue(value);
   emit setValueFromExposedModel(value);
}

void qtobserver::HorizontalSlider::stateSchemaElementModified(model::StateSchemaElement *stateSchemaElement)
{
    int max, min;
    stateSchemaElement->getMinConstraint(min);
    stateSchemaElement->getMaxConstraint(max);
    m_slider->setMaximum(max);
    m_slider->setMinimum(min);
}

void qtobserver::HorizontalSlider::setValueFromQt(int value)
{
   m_model->updateElement<int>(m_key, value);
}


} // of namespace tinia
