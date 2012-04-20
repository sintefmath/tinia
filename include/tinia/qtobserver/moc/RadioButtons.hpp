#ifndef QTOBSERVER_RADIOBUTTONS_HPP
#define QTOBSERVER_RADIOBUTTONS_HPP
#include <QGroupBox>
#include <QRadioButton>
#include <QList>
#include <string>
#include "tinia/policylib/PolicyLib.hpp"
#include <memory>

namespace qtobserver {

class RadioButtons : public QGroupBox
{
    Q_OBJECT
public:
    explicit RadioButtons(std::string key,
                          std::shared_ptr<policylib::PolicyLib> policyLib,
                          QWidget *parent = 0);

signals:

public slots:

private slots:
   void buttonChecked(bool);

private:
   std::shared_ptr<policylib::PolicyLib> m_policyLib;
   std::string m_key;
   QList<QRadioButton*> m_buttons;
};
}
#endif // QTOBSERVER_RADIOBUTTONS_HPP
