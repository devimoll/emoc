#ifndef SLIDER_H
#define SLIDER_H

#include <QWidget>
#include <QDebug>
#include <QPainter>
#include <QMouseEvent>

// trim display and seek bar
class slider : public QWidget
{
    Q_OBJECT
public:
    slider(int, int, bool);

    //fns
    void set_curr_page_num(int);
    void set_curr_pos_x(int);
    void update_external();

protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

private:
    bool is_reversed;
    int slider_width;
    int page_count;
    // seekのx軸方向の位置の、width()を基準としたときの割合
    double curr_pos_x_rate;
    int bar_length;
    int bar_thickness;
    int bar_pos_x;
    int bar_pos_y;

    //fns
    int page_num_to_x(int);
    double page_num_to_x_rate(int);
    int x_to_page_num(int);

private slots:
    void emit_curr_page_num_changed(int);

signals:
    void curr_page_num_changed(int);
};

#endif // SLIDER_H
