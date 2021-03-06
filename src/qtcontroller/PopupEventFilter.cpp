/* Copyright STIFTELSEN SINTEF 2012
 * 
 * This file is part of the Tinia Framework.
 * 
 * The Tinia Framework is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * The Tinia Framework is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public License
 * along with the Tinia Framework.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "tinia/qtcontroller/moc/PopupEventFilter.hpp"
#include <iostream>
#include <algorithm>
#include <QObject>
#include <QEvent>
#include <QPoint>
#include <QFocusEvent>

namespace tinia {
namespace qtcontroller {
namespace impl {


PopupEventFilter::PopupEventFilter( QWidget* watch, QAbstractButton* toggle )
    : QObject( watch ),
      m_watch( watch ),
      m_toggle( toggle )
{
    watch->setWindowFlags( Qt::Popup );
    watch->setWindowModality( Qt::NonModal );
    watch->hide();
    watch->installEventFilter( this );
    connect( m_toggle, SIGNAL(toggled(bool)), m_watch, SLOT(setVisible(bool)) );
}

bool
PopupEventFilter::eventFilter( QObject* obj, QEvent* event )
{
    if( event->type() == QEvent::Show ) {
        QWidget* w = m_toggle->window();
        QPoint ul = QPoint( w->geometry().x(),
                            w->geometry().y() );
        QPoint lr = ul + QPoint( w->width(),
                                 w->height() );
        QPoint p = m_toggle->parentWidget()->mapToGlobal( m_toggle->pos() +
                                                          QPoint( m_toggle->width(),
                                                                  m_toggle->height() ) );
        p = QPoint( std::min( lr.x() - m_watch->width(), p.x() ),
                    std::min( lr.y() - m_watch->height(), p.y() ) );
        p = QPoint( std::max( ul.x(), p.x() ),
                    std::max( ul.y(), p.y() ) );
        m_watch->move( p );
    }
    else if( event->type() == QEvent::Resize ) {
        QWidget* w = m_toggle->window();
        QPoint ul = QPoint( w->geometry().x(),
                            w->geometry().y() );
        QPoint lr = ul + QPoint( w->width(),
                                 w->height() );
        QPoint p = m_toggle->parentWidget()->mapToGlobal( m_toggle->pos() +
                                                          QPoint( m_toggle->width(),
                                                                  m_toggle->height() ) );
        p = QPoint( std::min( lr.x() - m_watch->width(), p.x() ),
                    std::min( lr.y() - m_watch->height(), p.y() ) );
        p = QPoint( std::max( ul.x(), p.x() ),
                    std::max( ul.y(), p.y() ) );
        m_watch->move( p );
    }
    else if( event->type() == QEvent::KeyPress ) {
        QKeyEvent* e = static_cast<QKeyEvent*>( event );
        if( e->key() == Qt::Key_Escape ) {
            m_watch->hide();
        }
    }
    else if( event->type() == QEvent::MouseButtonPress ) {
        if( !m_watch->underMouse() ) {
            m_watch->hide();
        }
    }
    else if( event->type() == QEvent::Hide ) {
        m_toggle->setChecked( false );
    }
    return QObject::eventFilter( obj, event );
}


}
} // of namespace qtcontroller
} // of namespace tinia

