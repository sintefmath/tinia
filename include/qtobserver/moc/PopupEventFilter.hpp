#pragma once

#include <QObject>
#include <QWidget>
#include <QAbstractButton>

namespace qtobserver {

class PopupEventFilter
        : public QObject
{
    Q_OBJECT;
public:

    PopupEventFilter( QWidget* watch, QAbstractButton* toggle );

protected:
    bool
    eventFilter( QObject* obj, QEvent* event );

private:
    QWidget*            m_watch;
    QAbstractButton*    m_toggle;
};



} // of namespace qtobserver
