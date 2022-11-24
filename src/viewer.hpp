#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QLabel>
#include <QMessageBox>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <opencv2/core/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/imgcodecs/imgcodecs.hpp>

class viewer : public QLabel
{
     Q_OBJECT
public:
    viewer(int);
    ~viewer();
    void set_image(int, QString);
    void set_curr_page_num(int);
    void set_hidaritoji(bool);
    bool get_is_hidaritoji();
    bool get_is_mihiraki();
    bool toggle_mihiraki();
    bool is_image_empty(int);
    void switch_interpolation();

protected:
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);

private:
    bool is_page_num_valid(int);
    void draw();

    int page_count;
    QVector<cv::Mat> images;
    int curr_page_num;
    bool is_hidaritoji;
    bool is_mihiraki;
    int interpolation;

signals:
    void goto_left_signal();
    void goto_right_signal();
};

#endif // VIEWER_HPP
