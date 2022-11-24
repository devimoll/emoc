#include "emoc.hpp"
#include <QDebug>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStandardPaths>
#include <QThread>
#include <QtConcurrent/QtConcurrent>

emoc::emoc(QWidget *parent)
    : QWidget(parent),
      v{nullptr},
      s{nullptr},
      tmp_dir_ap{"/home/decimoll/.cache/emoc"},
      file_ap{""},
      curr_page_num{0},
      slider_height{20},
      does_show_ui{false},
      prtsc_out_dir_ap{"~/pictures/emoc/"},
      layout{new QGridLayout()},
      buttons_layout{new QGridLayout()},
      has_pressed_g{false},
      thread_num{4},
      pd{nullptr},
      prtsc_btn{new touch_button()},
      interpolation_btn{new touch_button()},
      rotate_left_btn{new touch_button()},
      rotate_normal_btn{new touch_button()},
      img_name{new QLabel()},
      fullscreen_btn{new touch_button()},
      quit_btn{new touch_button()},
      about_emoc_btn{new touch_button()},
      mihiraki_btn{new touch_button()}
{
    setGeometry(400, 300, 400, 300);
    // 起動直後のみ 引数確認
    QStringList argv = QCoreApplication::arguments();
    argv.pop_front();
    if (argv.size() == 1) {
        init(argv[0]);
    }
    else {
        if (!argv.isEmpty()) {
            qDebug() << "Too many arguments.";
            QMessageBox mb(this);
            mb.warning(this, tr("Warning"), tr("Too many arguments."), QMessageBox::Abort);
        }
        init("");
    }
}

emoc::~emoc()
{
    delete v;
    delete s;
    delete pd;
    delete prtsc_btn;
    delete interpolation_btn;
    delete rotate_left_btn;
    delete rotate_normal_btn;
    delete img_name;
    delete fullscreen_btn;
    delete quit_btn;
    delete about_emoc_btn;
    delete mihiraki_btn;
    // deleteの順番は重要 layoutの中身をdeleteしてからlayoutをdeleteしないとsegmentation fault
    delete buttons_layout;
    delete layout;

}

void emoc::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_A:
        show_about_emoc();
        break;
    case Qt::Key_I:
        v->switch_interpolation();
        break;
    case Qt::Key_H:
        goto_left();
        break;
    case Qt::Key_L:
        goto_right();
        break;
    case Qt::Key_G:
        if (e->modifiers() == Qt::ShiftModifier) {
            goto_last();
        }
        else {
            if (has_pressed_g) {
                goto_first();
                has_pressed_g = false;
            }
            else {
                has_pressed_g = true;
                return;
            }
        }
        break;
    case Qt::Key_F:
        toggle_fullscreen();
        break;
    case Qt::Key_M:
        v->toggle_mihiraki();
        break;
    case Qt::Key_P:
        prtsc();
        break;
    case Qt::Key_U:
        if (does_show_ui) {
            hide_ui();
        }
        else {
            show_ui();
        }
        break;
    case Qt::Key_Q:
        quit();
        break;
    }
    has_pressed_g = false;
}

bool emoc::set_curr_page_num(int ind)
{
    if (!is_page_num_valid(ind)) {
        qDebug() << "invalid page_num in emoc::set_curr_page_num" << ind;
        return 1;
    }
    curr_page_num = ind;
    mutex.lock();
    if (v->is_image_empty(curr_page_num)) {
        extract_load_page(curr_page_num);
    }
    // 見開きのため 次のページも読まれてるか確認 最終ページは範囲外アクセスになるから除外
    if (curr_page_num + 1 < page_count && v->is_image_empty(curr_page_num + 1)) {
        extract_load_page(curr_page_num + 1);
    }
    mutex.unlock();
    v->set_curr_page_num(curr_page_num);
    s->set_curr_page_num(curr_page_num);

    img_name->setText(page_names[curr_page_num]);

    return 0;
}

int emoc::get_curr_page_num()
{
    return curr_page_num;
}

bool emoc::is_page_num_valid(int ind)
{
    return 0 <= ind && ind < page_count;
}

// 引数: ファイルパス opendialogを出したいときは""
void emoc::init(QString input_archive_path)
{
    if (input_archive_path == "") {
        input_archive_path = open_file_dialog();
        if (input_archive_path == "") {
            qDebug() << "file not selected.";
            exit(0);
        }
    }
    switch (open(input_archive_path)) {
    case 0:
        break;
    case 1:
        qDebug() << "case 1";
        exit(1);
    case 2:
        qDebug() << "case 2";
        exit(2);
    case 3:
        qDebug() << "case 3";
        exit(3);
    case 4:
        qDebug() << "case 4";
        exit(4);
    default:
        qDebug() << "default";
        exit(5);
    }

    QDir dir(tmp_dir_ap);
    dir.removeRecursively();

    dir = QDir("");
    dir.mkpath(tmp_dir_ap);

    page_names = get_page_names_from_archive(file_ap);
    page_count = page_names.size();

    v = new viewer(page_count);

    // 左綴か
    QFileInfo checker(file_ap);
    if (checker.completeSuffix() == "cbtl") {
        v->set_hidaritoji(true);
    }
    else {
        v->set_hidaritoji(false);
    }

    s = new slider(slider_height, page_count, !v->get_is_hidaritoji());

    v->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    s->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    prtsc_btn->setText("prtsc");
    interpolation_btn->setText("switch interpolation");
    rotate_left_btn->setText("rotate left");
    rotate_normal_btn->setText("rotate normal");
    fullscreen_btn->setText("toggle fullscreen");
    quit_btn->setText("quit");
    about_emoc_btn->setText("about emoc");
    mihiraki_btn->setText("toggle mihiraki");

    s->hide();
    prtsc_btn->hide()        ;
    interpolation_btn->hide();
    rotate_left_btn->hide()  ;
    rotate_normal_btn->hide();
    img_name->hide();
    fullscreen_btn->hide()   ;
    quit_btn->hide()         ;
    about_emoc_btn->hide()   ;
    mihiraki_btn->hide()     ;

    buttons_layout->addWidget(prtsc_btn,         0, 0, 1, 1);
    buttons_layout->addWidget(interpolation_btn, 0, 1, 1, 1);
    buttons_layout->addWidget(rotate_left_btn,   0, 2, 1, 1);
    buttons_layout->addWidget(rotate_normal_btn, 0, 3, 1, 1);
    buttons_layout->addWidget(img_name, 0, 4, 1, 1);
    buttons_layout->addWidget(fullscreen_btn,    0, 5, 1, 1);
    buttons_layout->addWidget(quit_btn,          0, 6, 1, 1);
    buttons_layout->addWidget(about_emoc_btn,    0, 7, 1, 1);
    buttons_layout->addWidget(mihiraki_btn,      0, 8, 1, 1);

    layout->addWidget(v, 0, 0, 1, 1);
    // Ignoredだと画像のサイズ以下までウィンドウを小さくできる
    // Expandingだと最小サイズが画像サイズになる
    layout->addWidget(s, 1, 0, 1, 1);
    layout->addLayout(buttons_layout, 2, 0, 1, 1);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    configure_connections();

    setWindowTitle("emoc " + file_ap);
    setWindowState(Qt::WindowFullScreen);

// 全部メモリに読み込むモード、全部HDD上に解凍するモード、逐一解凍するモード
    extract_load_page(0);

    pd = new QProgressDialog(
        tr("extracting..."),
        nullptr, 0, page_count, this);
    pd->setWindowModality(Qt::WindowModal);
    pd->setWindowFlags(Qt::Dialog |
                       Qt::CustomizeWindowHint);
    pd->setCancelButton(0);
    pd->setWindowTitle(tr("extracting"));
    pd->open();
    QFuture<void> th = QtConcurrent::run(&emoc::extract_load_all_pages, this);

    v->set_curr_page_num(0);
    s->set_curr_page_num(0);

    img_name->setText(page_names[0]);
}

QString emoc::open_file_dialog()
{
    QString filter = "(*.cbt *.cbtl)";
    QString tmp = QFileDialog::getOpenFileName(this, tr("open comic book tar..."), QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation), filter);
    qDebug() << tmp;
    return tmp;
}

int emoc::open(QString path)
{
    // 現在開いているのと同じファイルを開いた場合
    if (file_ap == path) {
        qDebug() << "already opened.";
        return 1;
    }

    // 存在確認
    QFileInfo checker(path);
    if (!checker.exists()) {
        qDebug() << path << " not exist.";
        return 2;
    }

    // 対応した拡張子であるか
    if (checker.completeSuffix() != "cbt" && checker.completeSuffix() != "cbtl") {
        qDebug() << " do not have a suffix of cbt or cbtl file.";
        return 3;
    }

    // アーカイブの中身は直下にpngまたはjpgという形式になっているか
    QStringList tmp_page_names = get_page_names_from_archive(path);
    for (QString p : tmp_page_names) {
        if (QFileInfo(p).completeSuffix() != "png" && QFileInfo(p).completeSuffix() != "jpg") {
            qDebug() << "アーカイブの中身は直下にpngまたはjpgという形式にしてくだされ。";
            QMessageBox::information(this, tr("Information"), tr("アーカイブの中身は直下にpngまたはjpgという形式にしてくだされ。"));
            return 4;
        }
    }

    file_ap = path;

    return 0;
}

QStringList emoc::get_page_names_from_archive(QString archive_path)
{
    //QString cmd = "7z l \"" + path + "\" -slt -ba | grep Path | cut -c8-";
    QString cmd = "tar tf \"" + archive_path + "\"";
    return get_stdout(cmd);
}

void emoc::extract_load_page(int n)
{
    if (!is_page_num_valid(n)) {
        qDebug() << "invalid page_num in emoc::extract_load_page";
        return;
    }
    extract_page(n);
    load_page(n);
}

void emoc::extract_page(int n) {
    QString cmd = "cd " + tmp_dir_ap + " && tar xvf \"" + file_ap + "\" \"" + page_names[n] + "\"";
    system(cmd.toUtf8().constData());
}

void emoc::load_page(int page_num)
{
    QString ap = tmp_dir_ap + "/" + page_names[page_num];
    QFileInfo checker(ap);
    if (!checker.exists()) {
        qDebug() << ap << "not found.";
        return;
    }
    v->set_image(page_num, ap);
    QFile::remove(ap);
}

void emoc::extract_load_all_pages()
{
#include <QElapsedTimer>
    QElapsedTimer et1;
    et1.start();

    QFuture<void> th = QtConcurrent::run(&emoc::extract_all_pages, this);

    for (int i = 0; i < page_count; i++) {
        QString ap = tmp_dir_ap + "/" + page_names[i];
        QFileInfo checker(ap);
        if (!checker.exists()) {
            i--;
            continue;
        }
        emit pd_value_changed(i);
        emit pd_text_changed("extracting " + QString::number(i) + "/" + QString::number(page_count));
    }
    emit pd_value_changed(page_count);
    th.waitForFinished();

    emit pd_value_changed(0);
    emit pd_text_changed("loading...");
    emit pd_title_changed("loading");

    QElapsedTimer et2;
    et2.start();

    QVector<QFuture<void>> load_ths(thread_num);
    int *loaded_pages_num = new int(0); // 進捗表示用
    for (int i = 0; i < thread_num; i++) {
        load_ths[i] = QtConcurrent::run(&emoc::load_all_pages, this, loaded_pages_num, thread_num, i);
    }

    for (int i = 0; i < thread_num; i++) {
        load_ths[i].waitForFinished();
    }
    delete loaded_pages_num;
    auto t1 = et1.elapsed();
    auto t2 = et2.elapsed();
    qDebug() << t1 << t2 << t1 - t2;

    emit close_pd_signal();
}

void emoc::extract_all_pages()
{
    QString cmd = "cd " + tmp_dir_ap + " && tar xvf \"" + file_ap + "\" -C \"" + tmp_dir_ap + "\"";
    qDebug() << cmd;
    system(cmd.toUtf8().constData());
}

void emoc::load_all_pages(int *loaded_pages_num, int thread_num, int n)
{
    for (int i = n; i < page_count; i+=thread_num) {
        QString ap = tmp_dir_ap + "/" + page_names[i];
        QFileInfo checker(ap);
        if (!checker.exists()) {
            i -= thread_num;
            continue;
        }
        v->set_image(i, ap);
        mutex.lock();
        (*loaded_pages_num)++;
        //qDebug() << "loaded " << ap << *loaded_pages_num;
        mutex.unlock();
        emit pd_value_changed(*loaded_pages_num);
        emit pd_text_changed("loading " + QString::number(*loaded_pages_num) + "/" + QString::number(page_count));
        QFile::remove(ap);
    }
}

void emoc::configure_connections()
{
    connect(s, &slider::curr_page_num_changed,
            this, &emoc::set_curr_page_num);

    connect(this, &emoc::pd_value_changed, this, &emoc::set_value_pd);
    connect(this, &emoc::pd_text_changed, this, &emoc::change_pd_text);
    connect(this, &emoc::pd_title_changed, this, &emoc::change_pd_title);
    connect(this, &emoc::close_pd_signal, this, &emoc::close_pd);

    connect(v, &viewer::goto_left_signal, this, &emoc::goto_left);
    connect(v, &viewer::goto_right_signal, this, &emoc::goto_right);

    connect(prtsc_btn, &touch_button::clicked, this, &emoc::prtsc);
    connect(interpolation_btn, &touch_button::clicked, v, &viewer::switch_interpolation);
    connect(rotate_left_btn, &touch_button::clicked, this, &emoc::rotate_left);
    connect(rotate_normal_btn, &touch_button::clicked, this, &emoc::rotate_normal);
    connect(fullscreen_btn, &touch_button::clicked, this, &emoc::toggle_fullscreen);
    connect(quit_btn, &touch_button::clicked, this, &emoc::quit);
    connect(about_emoc_btn, &touch_button::clicked, this, &emoc::show_about_emoc);
    connect(mihiraki_btn, &touch_button::clicked, v, &viewer::toggle_mihiraki);

}

void emoc::set_value_pd(int v)
{
    pd->setValue(v);
}

void emoc::close_pd()
{
    pd->close();
}

void emoc::change_pd_text(QString t)
{
    pd->setLabelText(t);
}

void emoc::change_pd_title(QString t)
{
    pd->setWindowTitle(t);
}

void emoc::toggle_fullscreen()
{
    if (windowState() == Qt::WindowFullScreen)
        setWindowState(Qt::WindowNoState);
    else
        setWindowState(Qt::WindowFullScreen);
}

void emoc::prtsc()
{
    qDebug() << 1234;
    QString cmd = "cd " + prtsc_out_dir_ap + " && tar xvf \"" + file_ap + "\" \"" + page_names[curr_page_num] + "\"";
    system(cmd.toUtf8().constData());
}

void emoc::rotate_left()
{
    qDebug() << "未実装";
}

void emoc::rotate_normal()
{
    qDebug() << "未実装";
}

void emoc::hide_ui()
{
    s->hide();
    prtsc_btn->hide()        ;
    interpolation_btn->hide();
    rotate_left_btn->hide()  ;
    rotate_normal_btn->hide();
    img_name->hide();
    fullscreen_btn->hide()   ;
    quit_btn->hide()         ;
    about_emoc_btn->hide()   ;
    mihiraki_btn->hide()     ;
    does_show_ui= false;
}

void emoc::show_ui()
{
    s->show();
    prtsc_btn->show()        ;
    interpolation_btn->show();
    rotate_left_btn->show()  ;
    rotate_normal_btn->show();
    img_name->show();
    fullscreen_btn->show()   ;
    quit_btn->show()         ;
    about_emoc_btn->show()   ;
    mihiraki_btn->show()     ;
    does_show_ui = true;
}

void emoc::show_about_emoc()
{
    QMessageBox msgbox(QMessageBox::Information, tr("About emoc"), tr("eat my own cooking"), QMessageBox::Close, this);

    QPushButton *about_qt = msgbox.addButton(tr("About Qt"), QMessageBox::ActionRole);

    msgbox.setWindowFlags(Qt::Dialog);
    msgbox.exec();
    if (msgbox.clickedButton() == about_qt) {
        msgbox.aboutQt(this, tr("About Qt"));
    }
}

void emoc::quit()
{
    int button = QMessageBox::question(this, tr("quit"), tr("quit?"));
    if (button == QMessageBox::Yes) {
        QCoreApplication::exit(0);
    }
}

// 左のページ
void emoc::goto_left()
{
    if (v->get_is_hidaritoji()) {
        goto_prev();
    }
    else {
        goto_next();
    }
}

// 右のページ
void emoc::goto_right()
{
    if (v->get_is_hidaritoji()) {
        goto_next();
    }
    else {
        goto_prev();
    }
}

// 次のページ
void emoc::goto_next()
{
    if (v->get_is_mihiraki()) {
        this->set_curr_page_num(this->get_curr_page_num() + 2);
    }
    else {
        this->set_curr_page_num(this->get_curr_page_num() + 1);
    }
    s->set_curr_page_num(this->get_curr_page_num());
}

// 前のページ
void emoc::goto_prev()
{
    if (v->get_is_mihiraki()) {
        this->set_curr_page_num(this->get_curr_page_num() - 2);
    }
    else {
        this->set_curr_page_num(this->get_curr_page_num() - 1);
    }
    s->set_curr_page_num(this->get_curr_page_num());
}

void emoc::goto_first()
{
    this->set_curr_page_num(0);
    s->set_curr_page_num(0);
}

void emoc::goto_last()
{
    this->set_curr_page_num(page_count - 1);
    s->set_curr_page_num(page_count - 1);
}

QStringList emoc::get_stdout(QString cmd)
{
    // unix系ではpopen(), pclose()を使う
    FILE* fp;
    char buf[256];
    // コマンドを指定してパイプをオープン
    if ((fp = popen(cmd.toUtf8().constData(), "r")) == NULL) {
        qDebug() << "abort";
        exit(1);
    }
    QStringList list;
    // パイプからのコマンド実行結果を受け取り表示
    while (fgets(buf, sizeof(buf), fp) != NULL) {
        list.append(QString(buf).trimmed());
    }
    // パイプをクローズ
    pclose(fp);

    return list;
}
