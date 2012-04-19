#ifndef QTOBSERVER_TABWIDGETCHILDREN_HPP
#define QTOBSERVER_TABWIDGETCHILDREN_HPP

#include <QWidget>
#include <QTableWidget>
#include "policylib/PolicyLib.hpp"
#include "policylib/StateSchemaListener.hpp"
namespace qtobserver {

class TabWidgetChildren : public QWidget, public policylib::StateSchemaListener
{
    Q_OBJECT
public:
    explicit TabWidgetChildren(std::string key,
                               std::shared_ptr<policylib::PolicyLib> policyLib,
                               QTabWidget *parent = 0);
   void stateSchemaElementAdded(policylib::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policylib::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement);

signals:

public slots:
private:
      std::shared_ptr<policylib::PolicyLib> m_policyLib;
      std::string m_key;

};

} // namespace qtobserver

#endif // QTOBSERVER_TABWIDGETCHILDREN_HPP
