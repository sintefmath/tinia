#include "qtobserver/moc/TabWidgetChildren.hpp"

namespace qtobserver {

/**
  \todo Make this change when the annotation changes
  */
TabWidgetChildren::TabWidgetChildren(std::string key,
                                     std::shared_ptr<policylib::PolicyLib> policyLib,
                                     QTabWidget *parent) :
    QWidget(parent)
{

}

}

void qtobserver::TabWidgetChildren::stateSchemaElementModified(policylib::StateSchemaElement *stateSchemaElement)
{
} // namespace qtobserver
