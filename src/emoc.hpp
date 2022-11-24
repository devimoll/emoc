#ifndef EMOC_HPP
#define EMOC_HPP

#include <QWidget>
#include "viewer.hpp"
#include "slider.hpp"
#include "touch_button.hpp"
#include <QLayout>
#include <QMutex>
#include <QProgressDialog>

class emoc : public QWidget
{
    Q_OBJECT

public:
    emoc(QWidget *parent = nullptr);
    ~emoc();

protected:
    void keyPressEvent(QKeyEvent*);

private:
    // functions
    bool set_curr_page_num(int);
    int get_curr_page_num();
    bool is_page_num_valid(int);
    void init(QString);
    QString open_file_dialog();
    int open(QString);
    QStringList get_page_names_from_archive(QString);
    void extract_load_page(int);
    void extract_page(int);
    void load_page(int);
    void extract_all_pages();
    void configure_connections();
    void set_value_pd(int);
    void close_pd();
    void change_pd_text(QString);
    void change_pd_title(QString);
    void toggle_fullscreen();
    void prtsc();
    void rotate_left();
    void rotate_normal();
    void hide_ui();
    void show_ui();
    void show_about_emoc();
    void quit();

    void goto_left();
    void goto_right();
    void goto_next();
    void goto_prev();
    void goto_first();
    void goto_last();


    QStringList get_stdout(QString);

    // variables
    viewer *v;
    slider *s;
    QString tmp_dir_ap;
    QString file_ap;
    QStringList page_names;
    int page_count;
    int curr_page_num;
    int slider_height;
    bool does_use_buffer;
    bool does_show_ui;
    QString prtsc_out_dir_ap;
    QGridLayout *layout;
    QGridLayout *buttons_layout;
    bool has_pressed_g;
    int thread_num;
    QProgressDialog *pd;
    QMutex mutex;

    // buttons
    touch_button *prtsc_btn;
    touch_button *interpolation_btn;
    touch_button *rotate_left_btn;
    touch_button *rotate_normal_btn;
    QLabel *img_name;
    touch_button *fullscreen_btn;
    touch_button *quit_btn;
    touch_button *about_emoc_btn;
    touch_button *mihiraki_btn;

private slots:
    void extract_load_all_pages();
    void load_all_pages(int*, int, int);

signals:
    void pd_value_changed(int);
    void pd_text_changed(QString);
    void pd_title_changed(QString);
    void close_pd_signal();
};
#endif // EMOC_HPP
