#pragma once

#include <QObject>
#include <QWidget>
#include <QString>
#include <QAbstractButton>
#include <string>
#include "policylib/PolicyLib.hpp"
#include "policylib/StateListener.hpp"

namespace qtobserver {

class StringController
        : public QObject,
          public policylib::StateListener
{
    Q_OBJECT;
public:
    explicit
    StringController( QWidget*                               widget,
                      std::shared_ptr<policylib::PolicyLib>  policylib,
                      const std::string&                     key,
                      const bool                             show_value,
                      const QString&                         suffix = "" );

    ~StringController();

    void
    stateElementModified(policylib::StateElement *stateElement);

signals:
    void
    textChangeFromPolicyLib( const QString& text );

public slots:

    void
    textChangeFromQt( const QString& text );

private:
    std::shared_ptr<policylib::PolicyLib>   m_policylib;
    const std::string                       m_key;
    const bool                              m_show_value;
    std::string                             m_current_value;
    QString                                 m_suffix;
    QAbstractButton*                        m_button;
};






}
