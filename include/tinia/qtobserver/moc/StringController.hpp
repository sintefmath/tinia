#pragma once

#include <QObject>
#include <QWidget>
#include <QString>
#include <QAbstractButton>
#include <string>
#include "tinia/policy/Policy.hpp"
#include "tinia/policy/StateListener.hpp"

namespace tinia {
namespace qtobserver {

class StringController
        : public QObject,
          public policy::StateListener
{
    Q_OBJECT;
public:
    explicit
    StringController( QWidget*                               widget,
                      std::shared_ptr<policy::Policy>  policy,
                      const std::string&                     key,
                      const bool                             show_value,
                      const QString&                         suffix = "" );

    ~StringController();

    void
    stateElementModified(policy::StateElement *stateElement);

signals:
    void
    textChangeFromPolicy( const QString& text );

public slots:

    void
    textChangeFromQt( const QString& text );

private:
    std::shared_ptr<policy::Policy>   m_policy;
    const std::string                       m_key;
    const bool                              m_show_value;
    std::string                             m_current_value;
    QString                                 m_suffix;
    QAbstractButton*                        m_button;
};






}
}
