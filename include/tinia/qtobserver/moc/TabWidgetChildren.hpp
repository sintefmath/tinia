#ifndef QTOBSERVER_TABWIDGETCHILDREN_HPP
#define QTOBSERVER_TABWIDGETCHILDREN_HPP

#include <QWidget>
#include <QTableWidget>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateSchemaListener.hpp"

namespace tinia {
namespace qtobserver {

class TabWidgetChildren : public QWidget, public policy::StateSchemaListener
{
    Q_OBJECT
public:
    explicit TabWidgetChildren(std::string key,
                               std::shared_ptr<policy::Policy> policy,
                               QTabWidget *parent = 0);
   void stateSchemaElementAdded(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementRemoved(policy::StateSchemaElement *stateSchemaElement) {}
   void stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement);

signals:

public slots:
private:
      std::shared_ptr<policy::Policy> m_policy;
      std::string m_key;

};

} // namespace qtobserver
} // of namespace tinia
#endif // QTOBSERVER_TABWIDGETCHILDREN_HPP
