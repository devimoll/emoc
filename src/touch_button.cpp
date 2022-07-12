#include "touch_button.hpp"

touch_button::touch_button()
{
    setAttribute(Qt::WA_AcceptTouchEvents);
}

bool touch_button::event(QEvent *e)
{/*
    switch (e->type()) {
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        if (e->type() == QEvent::TouchEnd) {
            emit clicked();
        }
        return true;
    }
    default: break;
    }
*/
    return QPushButton::event(e);
}

