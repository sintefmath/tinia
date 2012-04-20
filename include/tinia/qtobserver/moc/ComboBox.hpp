#ifndef QTOBSERVER_COMBOBOX_HPP
#define QTOBSERVER_COMBOBOX_HPP

#include <QComboBox>
#include <QStringList>
#include <memory>
#include <tinia/policy/Policy.hpp>
#include <tinia/policy/StateListener.hpp>
#include <tinia/policy/StateSchemaListener.hpp>

namespace tinia {
namespace qtobserver {

class ComboBox : public QComboBox, public policy::StateListener,
         public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit ComboBox(std::string key, std::shared_ptr<policy::Policy> policy,
                      QWidget *parent = 0);
   ~ComboBox();

   void stateElementModified(policy::StateElement *stateElement);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement);

signals:
   void setStateFromPolicy(int);
   void clearFromPolicy();
   void addItemsFromPolicy(const QStringList& list);

public slots:

private slots:
   void activatedChanged(QString value);
private:
   QStringList m_options;
   std::shared_ptr<policy::Policy> m_policy;
   std::string m_key;
};

} // namespace qtobserver
} // namespace tinia

#endif // QTOBSERVER_COMBOBOX_HPP
