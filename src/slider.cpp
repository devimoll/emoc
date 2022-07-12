#include "slider.hpp"

slider::slider(int height, int page_count, bool is_reversed)
    : is_reversed{is_reversed},
      slider_width{4},
      page_count{page_count},
      bar_length{},
      bar_thickness{},
      bar_pos_x{},
      bar_pos_y{}
{
    setFixedHeight(height);
}

void slider::set_curr_page_num(int n)
{
   curr_pos_x_rate = page_num_to_x_rate(n);
   update();
}

// seek barのsliderを触ったときに呼ばれる
void slider::set_curr_pos_x(int x)
{
    emit_curr_page_num_changed(x_to_page_num(x));
}

void slider::paintEvent(QPaintEvent *e)
{
    QWidget::paintEvent(e);

    QPainter painter;
    painter.begin(this);
    // bar
    painter.setBrush(Qt::darkGray);
    // QPinter::drawRect(int x, int y, int width, int height);
    painter.drawRect(bar_pos_x, bar_pos_y,
                    bar_length, bar_thickness);

    // slider shadow
    painter.setBrush(QColor::fromRgb(100, 100, 100));
    // + bar_pos_x - slider_width/2 が省略されてゐる(第一項の定義はまさに第二項である)
    int slider_pos_x = bar_length * curr_pos_x_rate;
    painter.setPen(Qt::SolidLine);
    painter.drawRect(slider_pos_x, 0,
         slider_width, height() - 1);
    // slider
    painter.setBrush(QColor::fromRgb(200, 200, 200));
    // sliderの輪郭
    painter.setPen(Qt::NoPen);
    painter.drawRect(slider_pos_x + 1, 1,
                     slider_width - 2, height() - 3);
}

void slider::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    // sliderの幅ががwidth()よりも大きいと描画範囲からはみ出てしまい
    // 映らなくなるからbarの長さはwidth()よりも少し短くす。
    bar_length = width() - 1 - slider_width;
    bar_thickness = .6 * height();
    bar_pos_x = slider_width / 2;
    bar_pos_y = .2 * height();
}

void slider::mousePressEvent(QMouseEvent *e)
{
    QWidget::mousePressEvent(e);
    if (e->button() == Qt::LeftButton) {
        set_curr_pos_x(e->pos().x());
    }
}

void slider::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
    set_curr_pos_x(e->pos().x());
}

// page_numをx座標に変換
int slider::page_num_to_x(int n)
{
    return bar_length * page_num_to_x_rate(n) + bar_pos_x;
}

double slider::page_num_to_x_rate(int n)
{
    if (is_reversed) {
        return (1 - static_cast<double>(n) / (page_count - 1));
    }
    else {
        return static_cast<double>(n) / (page_count - 1);
    }
}

int slider::x_to_page_num(int x)
{
    x -= bar_pos_x;
    if (x < 0) {
        x = 0;
    }
    else if (x > bar_length) {
        x = bar_length;
    }
    if (is_reversed) {
        return ((page_count - 1) - static_cast<double>(page_count - 1) * x / bar_length);
    }
    else {
        return static_cast<double>(page_count - 1) * x / bar_length;
    }
}

// signalのemitを経由してcurr_page_numをセットすることで似た関数を１つにまとめた
void slider::emit_curr_page_num_changed(int n)
{
    emit curr_page_num_changed(n);
}
