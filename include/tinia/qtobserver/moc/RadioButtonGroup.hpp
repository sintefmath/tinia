#ifndef QTOBSERVER_RADIOBUTTONS_HPP
#define QTOBSERVER_RADIOBUTTONS_HPP
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <string>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"
#include "tinia/policy/StateSchemaListener.hpp"
#include <memory>
#include "tinia/qtobserver/moc/RadioButton.hpp"

namespace tinia {
namespace qtobserver {
/**
  \todo Add support for changing restriction set
*/
class RadioButtonGroup : public QGroupBox,
         public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit RadioButtonGroup(std::string key,
                          std::shared_ptr<policy::Policy> policy,
                              bool horizontal,
                          QWidget *parent = 0);

   ~RadioButtonGroup();

   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);

   void toggleRadioButtonFromPolicy();
public slots:

private:
   std::shared_ptr<policy::Policy> m_policy;
   std::string m_key;
};
}
}
#endif // QTOBSERVER_RADIOBUTTONS_HPP
