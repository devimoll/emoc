#ifndef TOUCH_BUTTON_HPP
#define TOUCH_BUTTON_HPP

#include <QPushButton>
#include <QEvent>

class touch_button : public QPushButton
{
    Q_OBJECT
public:
    touch_button();
protected:
    bool event(QEvent *e);
};

#endif // TOUCH_BUTTON_HPP
