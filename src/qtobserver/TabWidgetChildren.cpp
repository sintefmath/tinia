#include "tinia/qtobserver/moc/TabWidgetChildren.hpp"

namespace tinia {
namespace qtobserver {

/**
  \todo Make this change when the annotation changes
  */
TabWidgetChildren::TabWidgetChildren(std::string key,
                                     std::shared_ptr<policy::Policy> policy,
                                     QTabWidget *parent) :
    QWidget(parent)
{

}



void TabWidgetChildren::stateSchemaElementModified(policy::StateSchemaElement *stateSchemaElement)
{
}

} // namespace qtobserver
} // namespace tinia
