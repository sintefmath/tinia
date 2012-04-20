#ifndef QTOBSERVER_COMBOBOX_HPP
#define QTOBSERVER_COMBOBOX_HPP

#include <QComboBox>
#include <QStringList>
#include <memory>
#include <tinia/policylib/PolicyLib.hpp>
#include <tinia/policylib/StateListener.hpp>
#include <tinia/policylib/StateSchemaListener.hpp>


namespace qtobserver {

class ComboBox : public QComboBox, public policylib::StateListener,
         public policylib::StateSchemaListener
{
    Q_OBJECT
public:
    explicit ComboBox(std::string key, std::shared_ptr<policylib::PolicyLib> policyLib,
                      QWidget *parent = 0);
   ~ComboBox();

   void stateElementModified(policylib::StateElement *stateElement);
   void stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement);
   void stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement);

signals:
   void setStateFromPolicy(int);
   void clearFromPolicy();
   void addItemsFromPolicy(const QStringList& list);

public slots:

private slots:
   void activatedChanged(QString value);
private:
   QStringList m_options;
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   std::string m_key;
};

}

 // namespace qtobserver

#endif // QTOBSERVER_COMBOBOX_HPP
