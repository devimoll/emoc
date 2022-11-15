#include "viewer.hpp"

viewer::viewer(int page_count)
    : page_count{page_count},
      curr_page_num{0},
      is_hidaritoji{false},
      is_mihiraki{false},
      interpolation{cv::INTER_AREA}
{
    images.resize(page_count);
    setAlignment(Qt::AlignHCenter);
}

viewer::~viewer()
{
    //delete image;
}

void viewer::set_image(int n, QString ap) {
    images[n] = cv::imread((ap).toUtf8().constData());
}
// viewerは印刷屋か 俺がフィルム（画像データ）を持って行ってL版に引き伸ばしてプリントしてくれと頼む いやフィルムだとextractもしろ感あるか
// 俺が単行本を切り開いて１ページずつの状態にし（実際には裏表になるが）、それを写真屋に持っていって拡大コピーしてくれと頼むイメージか 俺はemocで写真屋はviewer そう考えるとsliderは別だ

void viewer::set_curr_page_num(int n)
{
    //is_mihiraki = false;
    if (is_image_empty(n)) {
        qDebug() << "IMAGE IS EMPTY." << n;
        exit(1);
    }
    this->curr_page_num = n;
    draw();
}

void viewer::set_hidaritoji(bool b)
{
    is_hidaritoji = b;
}

bool viewer::get_is_hidaritoji()
{
    return this->is_hidaritoji;
}

bool viewer::get_is_mihiraki()
{
    return this->is_mihiraki;
}

bool viewer::toggle_mihiraki()
{
    is_mihiraki = !is_mihiraki;
    draw();
    return is_mihiraki;
}

bool viewer::is_image_empty(int n)
{
    if (is_page_num_valid(n)) {
        return images[n].empty();
    }
    else {
        qDebug() << "page num invalid at viewer::is_image_empty: " << n;
        exit(1);
    }
}

void viewer::switch_interpolation()
{
    QString text;
    switch(interpolation){
        case cv::INTER_LANCZOS4:
            interpolation = cv::INTER_NEAREST;
            text = "INTER_NEAREST";
            break;
        case cv::INTER_NEAREST:
            interpolation = cv::INTER_LINEAR;
            text = "INTER_LINEAR";
            break;
        case cv::INTER_LINEAR:
            interpolation = cv::INTER_AREA;
            text = "INTER_AREA";
            break;
        case cv::INTER_AREA:
            interpolation = cv::INTER_CUBIC;
            text = "INTER_CUBIC";
            break;
        case cv::INTER_CUBIC:
            interpolation = cv::INTER_LANCZOS4;
            text = "INTER_LANCZOS4";
            break;
    }

    draw();
}

void viewer::resizeEvent(QResizeEvent *e)
{
    QLabel::resizeEvent(e);
    draw();
}

void viewer::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        if (e->position().x() < width() * 0.33) {
            emit goto_left_signal();
        }
        else if (e->position().x() < width() * 0.66) {
        }
        else {
            emit goto_right_signal();
        }
    }
}


bool viewer::is_page_num_valid(int n)
{
    return 0 <= n && n < page_count;
}

void viewer::draw()
{
    qreal dpr = devicePixelRatioF();
    cv::Mat resized;
    // とりあえず
    int image_scale = 1;

    cv::Mat drawing;

    if (is_mihiraki) {
        // 見開きオンのとき端のページで不具合が起こる
        if (curr_page_num + 1 >= page_count) {
            qDebug() << "This is last page.";
            drawing = images[curr_page_num];
        }
        else {
            if (is_image_empty(curr_page_num + 1)) {
                qDebug() << "NEXT IMAGE IS EMPTY.";
                exit(1);
            }
            if (is_hidaritoji) {
                cv::hconcat(images[curr_page_num], images[curr_page_num + 1], drawing);
            }
            else {
                cv::hconcat(images[curr_page_num + 1], images[curr_page_num], drawing);
            }
        }
    }
    else {
        drawing = images[curr_page_num];
    }

    if ((double)width() / height() > (double)drawing.size().width / drawing.size().height) {
        // 描画範囲の高さに合わせる
        int new_width = drawing.size().width * height() * dpr * image_scale / drawing.size().height;
        int new_height = height() * dpr * image_scale;
        // resize関数のsize引数に0が入るとエラーで落ちる
        if (new_width <= 0) {
            new_width = 1;
        }
        if (new_height <= 0) {
            new_height = 1;
        }
        cv::resize(drawing, resized, cv::Size(new_width, new_height), 0, 0, interpolation);
    }
    else {
        // 描画範囲の幅に合わせる
        int new_width = width() * dpr * image_scale;
        int new_height = drawing.size().height * width() * dpr * image_scale / drawing.size().width;
        if (new_width <= 0) {
            new_width = 1;
        }
        if (new_height <= 0) {
            new_height = 1;
        }
        cv::resize(drawing, resized, cv::Size(new_width, new_height), 0, 0, interpolation);
    }
    // 色変換
    cv::cvtColor(resized, resized, cv::COLOR_RGB2BGR);
    // QImageを経由してQPixmapにして描画 setPixmapするとpaintEvent()が呼ばれる
    setPixmap(QPixmap::fromImage(QImage(resized.data, resized.cols, resized.rows, resized.step, QImage::Format_RGB888)));

}
