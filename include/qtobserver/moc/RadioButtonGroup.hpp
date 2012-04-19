#ifndef QTOBSERVER_RADIOBUTTONS_HPP
#define QTOBSERVER_RADIOBUTTONS_HPP
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <string>
#include "policylib/PolicyLib.hpp"
#include "policylib/StateListener.hpp"
#include "policylib/StateSchemaListener.hpp"
#include <memory>
#include "qtobserver/moc/RadioButton.hpp"

namespace qtobserver {
/**
  \todo Add support for changing restriction set
*/
class RadioButtonGroup : public QGroupBox,
         public policylib::StateSchemaListener
{
    Q_OBJECT
public:
    explicit RadioButtonGroup(std::string key,
                          std::shared_ptr<policylib::PolicyLib> policyLib,
                              bool horizontal,
                          QWidget *parent = 0);

   ~RadioButtonGroup();

   void stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement);

   void toggleRadioButtonFromPolicy();
public slots:

private:
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   std::string m_key;
};
}
#endif // QTOBSERVER_RADIOBUTTONS_HPP
