#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDesktopWidget>
#include <QVideoWidget>
#include <QShortcut>
#include <QMessageBox>
#include <QKeyEvent>
#include <QTime>
#include <QTimer>
#include <QInputDialog>
#include <QApplication>
#include <QDesktopWidget>
#include <QStandardPaths>
#include <QScreen>
#include <QPainter>
#include <QMediaMetaData>
#include <QTextBrowser>
#include <QMimeData>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QClipboard>
#include <QDomDocument>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->splitter->setStretchFactor(0,2);
    ui->splitter->setStretchFactor(1,1);
    ui->widgetZY->hide();
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
    QAction *action_search = new QAction;
    action_search->setIcon(QIcon::fromTheme("search"));
    ui->lineEdit_search->addAction(action_search, QLineEdit::TrailingPosition);
    connect(action_search, SIGNAL(triggered()), this, SLOT(search()));
    connect(ui->lineEdit_search, SIGNAL(returnPressed()), this, SLOT(search()));
    connect(ui->treeWidget, SIGNAL(itemDoubleClicked(QTreeWidgetItem*, int)), this, SLOT(treeWidgetItemDoubleClicked(QTreeWidgetItem*, int)));

    QActionGroup *actionGroup_scale = new QActionGroup(this);
    actionGroup_scale->addAction(ui->action_scale0_5);
    actionGroup_scale->addAction(ui->action_scale1);
    actionGroup_scale->addAction(ui->action_scale1_5);
    actionGroup_scale->addAction(ui->action_scale2);
    actionGroup_scale->addAction(ui->action_scale0);

    scene = new QGraphicsScene;
    scene->setBackgroundBrush(Qt::black);
    GVI = new QGraphicsVideoItem;
    GVI->setAspectRatioMode(Qt::IgnoreAspectRatio);
    scene->addItem(GVI);
    GTI = new QGraphicsTextItem;
    GTI->setDefaultTextColor(Qt::white);
    QFont font = this->font();
    font.setPixelSize(20);
    GTI->setFont(font);
    scene->addItem(GTI);
    ui->graphicsView->setScene(scene);

    timer_information = new QTimer;
    timer_information->setSingleShot(true);
    connect(timer_information, &QTimer::timeout, [=]{
        GTI->hide();
    });

    timer_controlPanel = new QTimer;
    timer_controlPanel->setSingleShot(true);
    connect(timer_controlPanel, &QTimer::timeout, [=]{
        if(isFullScreen()){
            ui->sliderProgress->hide();
            ui->controlPanel->hide();
            setCursor(Qt::BlankCursor);
        }
    });

    ui->graphicsView->installEventFilter(this);
    ui->controlPanel->installEventFilter(this);


    player = new QMediaPlayer;
    player->setVolume(100);
    player->setVideoOutput(GVI);
    connect(player, SIGNAL(durationChanged(qint64)), this, SLOT(durationChange(qint64)));
    connect(player, SIGNAL(positionChanged(qint64)), this, SLOT(positionChange(qint64)));
    connect(player, SIGNAL(volumeChanged(int)), this, SLOT(volumeChange(int)));
    connect(player, SIGNAL(mutedChanged(bool)), this, SLOT(mutedChange(bool)));
    connect(player, SIGNAL(metaDataChanged()), this, SLOT(metaDataChange()));
    connect(player, SIGNAL(error(QMediaPlayer::Error)), this, SLOT(errorHandle(QMediaPlayer::Error)));
    connect(player, SIGNAL(stateChanged(QMediaPlayer::State)), SLOT(stateChange(QMediaPlayer::State)));
    connect(player, SIGNAL(mediaStatusChanged(QMediaPlayer::MediaStatus)), SLOT(mediaStatusChange(QMediaPlayer::MediaStatus)));
    connect(player, &QMediaPlayer::bufferStatusChanged, [=](int i){
        if (i < 100) {
            ui->statusBar->showMessage("正在缓冲 " + QString::number(i)+ "%");
        } else {
            ui->statusBar->showMessage("");
        }
    });

    playlist = new QMediaPlaylist;
    player->setPlaylist(playlist);

    connect(new QShortcut(QKeySequence(Qt::Key_O),this), SIGNAL(activated()), this, SLOT(on_action_open_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_U),this), SIGNAL(activated()), this, SLOT(on_action_openURL_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_T),this), SIGNAL(activated()), this, SLOT(on_action_liveList_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_I),this), SIGNAL(activated()), this, SLOT(on_action_liveImport_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()), this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Return),this), SIGNAL(activated()), this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::ALT + Qt::Key_Enter),this), SIGNAL(activated()), this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Space),this), SIGNAL(activated()), this, SLOT(playPause()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()), this, SLOT(on_pushButton_seek_backward_clicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()), this, SLOT(on_pushButton_seek_forward_clicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_P),this), SIGNAL(activated()), this, SLOT(on_action_capture_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Up),this), SIGNAL(activated()), this, SLOT(on_action_volumeUp_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Down),this), SIGNAL(activated()), this, SLOT(on_action_volumeDown_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_M),this), SIGNAL(activated()), this, SLOT(on_action_volumeMute_triggered()));
    connect(new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I),this), SIGNAL(activated()), this, SLOT(on_action_info_triggered()));
     connect(new QShortcut(QKeySequence(Qt::Key_0),this), SIGNAL(activated()), this, SLOT(on_action_scale0_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_1),this), SIGNAL(activated()), this, SLOT(on_action_scale1_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_2),this), SIGNAL(activated()), this, SLOT(on_action_scale2_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_3),this), SIGNAL(activated()), this, SLOT(on_action_scale1_5_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_4),this), SIGNAL(activated()), this, SLOT(on_action_scale0_5_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_5),this), SIGNAL(activated()), this, SLOT(fitDesktop()));

    connect(ui->action_checkVersion, SIGNAL(triggered(bool)), this, SLOT(manualCheckVersion(bool)));
    connect(ui->sliderProgress, SIGNAL(sliderReleased()), this, SLOT(sliderProgressReleased()));
    //connect(ui->sliderProgress,SIGNAL(valueChanged(int)), this, SLOT(sliderProgressValueChanged(int)));
    connect(ui->sliderProgress, SIGNAL(sliderMoved(int)), this, SLOT(sliderProgressMoved(int)));
    connect(ui->sliderVolume, SIGNAL(sliderMoved(int)), this, SLOT(sliderVolumeMoved(int)));

    createPopmenu();
    genTreeLive();
    TWI_search = new QTreeWidgetItem(ui->treeWidget);
    TWI_search->setText(0, "搜索");
    TWI_search->setToolTip(0, "");

    QStringList SL_args = QApplication::arguments();
    qDebug() << SL_args;
    QString log = "\n";
    for(int i=0; i< SL_args.length(); i++){
        log += SL_args.at(i) + " ";
    }
    if(SL_args.length() > 1){
        filename = SL_args.at(1);
        if(!filename.contains("chrome-extension://")){
            if(filename.startsWith("file://")){
                QUrl url(filename);
                filename = url.toLocalFile();
            }
            if(QFileInfo(filename).suffix().toLower() == "xspf"){
                //importXSPF(filename);
            }else{
                open(filename);
            }
        }else{
            // 下面这段放在外面会导致调试时窗口出不来和从外部程序打开文件中断
            // 接收Chrome扩展传来的数据
            int length = 0;
            //read the first four bytes (=> Length)
            //getwchar: receive char from stdin
            //putwchar: write char to stdout
            for (int i = 0; i < 4; i++) {
                length += getwchar();
            }
            //read the json-message
            QString url = "";
            for (int i = 0; i < length; i++) {
                url += getwchar();
            }

            log += "\n" + url;
            appandText("log.txt", log);

            //浏览器端传来的数据会有一个双引号引在两端
            url = url.mid(1, url.length()-2);
            qDebug() << url;
            if (url != "") {
                ui->widgetZY->hide();
                player->setMedia(QUrl(url));
                player->play();
                addHistory(url);
                setWindowTitle(url);
            }
        }
    }

    dialogUrl = new DialogURL(this);
    connect(dialogUrl->ui->pushButtonAnalyze,SIGNAL(pressed()),this,SLOT(analyze()));
    connect(dialogUrl->ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(playURL(int,int)));
    MPLurl = new QMediaPlaylist;
    MPLurl->setPlaybackMode(QMediaPlaylist::Sequential);
    connect(MPLurl, SIGNAL(currentIndexChanged(int)), this, SLOT(MPLCIChange(int)));

    QTimer::singleShot(5000, this, SLOT(autoCheckVersion()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if(filename=="") filename = ".";
    filename = QFileDialog::getOpenFileName(this, "打开媒体文件", filename);
    if(!filename.isEmpty()){
        open(filename);
    }
}

void MainWindow::open(QString path)
{
    if(QFileInfo(path).suffix().toLower() == "xspf"){
        //importXSPF(path);
        ui->statusBar->showMessage("导入 " + path);
    }else{
        player->setMedia(QUrl::fromLocalFile(path));
        player->play();
        setWindowTitle(QFileInfo(path).fileName());
        ui->statusBar->showMessage("打开 " + path);
        addHistory(path);
        ui->widgetZY->hide();
        ui->action_scale1->setChecked(true);
        playlist->addMedia(QUrl::fromLocalFile(path));
    }
}

void MainWindow::on_action_openURL_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray BA = QByteArray::fromPercentEncoding(clipboard->text().toUtf8());
    QString s = BA;
    dialogUrl->ui->lineEdit->setText(s);
    dialogUrl->show();
}

void MainWindow::on_pushButton_list_clicked()
{
    showHideList();
}

void MainWindow::on_action_liveList_triggered()
{
    showHideList();
}

void MainWindow::showHideList()
{
    if (ui->widgetZY->isVisible()) {
        ui->widgetZY->setVisible(false);
        //resize(QSize(width()-ui->tableWidget->width(),height()));
    } else {
        ui->widgetZY->setVisible(true);
        //resize(QSize(width()+ui->tableWidget->width(),height()));
    }
}

void MainWindow::on_action_liveImport_triggered()
{
    if(filename=="") filename = "tv.txt";
    filename = QFileDialog::getOpenFileName(this, "打开文本", filename, "文本文件(*.txt)");
    if(!filename.isEmpty()){
        //fillTable(filename);
    }
}

void MainWindow::on_action_quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_capture_triggered()
{
    QTime t(0,0,0);
    t = t.addMSecs(player->position());
    QString STime = t.toString("HHmmss");
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/" + QFileInfo(windowTitle()).baseName() + STime + ".jpg";
    QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
    QPainter painter(&image);
    scene->render(&painter);
    image.save(path);
    ui->statusBar->showMessage("截图 " + path);
}

QString MainWindow::SB(qint64 b)
{
    QString s = "";
    if(b>999999999){
        s = QString::number(b/(1024*1024*1024.0),'f',2) + " GB";
    }else{
        if(b>999999){
            s = QString::number(b/(1024*1024.0),'f',2) + " MB";
        }else{
            if(b>999){
                s = QString::number(b/1024.0,'f',2) + " KB";
            }else{
                s = QString::number(b) + " B";
            }
        }
    }
    return s;
}

void MainWindow::on_action_capture16_triggered()
{
    QPixmap pixc[16];
    player->pause();
    player->setMuted(true);
    QPixmap pixDS(1280 + 50, 960 + 80 + 40);
    pixDS.fill(Qt::white);
    int k = 0;
    QPainter painter(&pixDS);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.drawPixmap(20, 10, QPixmap(":/icon.png").scaled(60,60));
    painter.drawText(300, 30, "文件名称：" + QFileInfo(filename).fileName());
    painter.drawText(300, 60, "文件大小：" + SB(QFileInfo(filename).size()));
    painter.drawText(500, 60, "视频尺寸：" + QString::number(widthV) + " X " + QString::number(heightV));
    painter.drawText(700, 60, "视频时长：" + STimeDuration);
    for(qint64 i=player->duration()/16; i<player->duration(); i+=player->duration()/16){
        player->setPosition(i);
        QPixmap pixmap(scene->sceneRect().size().width(), scene->sceneRect().size().height());
        QPainter painter1(&pixmap);
        scene->render(&painter1);
        pixc[k] = pixmap.scaled(320, 240, Qt::KeepAspectRatio);
        QTime t(0,0,0);
        t = t.addMSecs(player->position());
        QString STimeElapse = t.toString("hh:mm:ss");
        QPainter painter2(&pixc[k]);
        painter2.setPen(QPen(Qt::white));
        //painter2.drawText(270, 235, STimeElapse);
        painter2.drawText(pixc[k].width()-60, pixc[k].height()-10, STimeElapse);
        QEventLoop eventloop;
        QTimer::singleShot(500, &eventloop, SLOT(quit()));
        eventloop.exec();
        k++;
    }
    k = 0;
    for(int y=0; y<4; y++){
        for(int x=0; x<4; x++){
            painter.drawPixmap(x*(320+10)+10, y*(240+10)+80, pixc[k]);
            k++;
        }
    }
    QFont font = this->font();
    font.setPointSize(20);
    painter.setFont(font);
    painter.drawText(100, 50, "海天鹰播放器");
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation) + "/Summary_" + QFileInfo(filename).baseName() + ".jpg";
    pixDS.save(path,0,100);
    player->play();
    player->setMuted(false);
}

void MainWindow::on_action_info_triggered()
{
    QString path = player->currentMedia().canonicalUrl().toString();
    if(path.startsWith("file://"))
        path = path.mid(7);
    QString s = "媒体地址：" + path + "\n";
    QStringList SLMD = player->availableMetaData();
    //qDebug() << SLMD;
    for(int i=0; i<SLMD.size(); i++){
        if(SLMD.at(i)=="PixelAspectRatio" || SLMD.at(i)=="Resolution"){
            s += SLMD.at(i) + ": " + QString::number(player->metaData(SLMD.at(i)).toSize().width()) + " X " + QString::number(player->metaData(SLMD.at(i)).toSize().height()) + "\n";
        }else{
            s += SLMD.at(i) + ": " + player->metaData(SLMD.at(i)).toString() + "\n";
        }
    }

    s += "文件大小：" + SB(QFileInfo(path).size());
    QMessageBox MBInfo(QMessageBox::NoIcon, "信息", s);
    QImage imageCover = player->metaData(QMediaMetaData::ThumbnailImage).value<QImage>();
    MBInfo.setIconPixmap(QPixmap::fromImage(imageCover));
    MBInfo.setWindowIcon(QIcon(":/icon.png"));
    MBInfo.exec();
}

void MainWindow::on_action_volumeUp_triggered()
{
    player->setMuted(false);
    if (player->volume() == 100) {
        GTI->setPlainText("音量：100");
        GTI->show();
        timer_information->start(3000);
    } else {
        player->setVolume(player->volume() + 1);
    }
}

void MainWindow::on_action_volumeDown_triggered()
{
    player->setMuted(false);
    if (player->volume() == 0) {
        GTI->setPlainText("音量：100");
        GTI->show();
        timer_information->start(3000);
    } else {
        player->setVolume(player->volume()-1);
    }
}

void MainWindow::on_action_volumeMute_triggered()
{
    if (player->isMuted()) {
        player->setMuted(false);
    } else {
        player->setMuted(true);
    }
}

void MainWindow::on_action_help_triggered()
{
    QMessageBox MB(QMessageBox::NoIcon, "帮助", "快捷键：\nO\t打开文件\nU\t打开网址\nT\t直播列表\nI\t导入直播文件\n空格\t播放、暂停\nAlt+回车\t全屏、退出全屏\n鼠标左键双击\nEsc\t退出全屏\n上\t增加音量\n下\t减小音量\n左 \t快退\n右\t快进\nM\t静音\nP\t截图\n1\t原始视频大小\n2\t2倍视频大小\n3\t1.5倍视频大小\n4\t0.5倍视频大小\n5\t视频铺满全屏");
    MB.setWindowIcon(QIcon(":/icon.png"));
    MB.exec();
}

void MainWindow::on_action_changelog_triggered()
{
    QString s = "3.0\n(2020-08)\n移除直播列表，增加OK资源网列表。\n\n2.0\n(2020-02)\n使用 QGraphicsVideoItem 代替 QVideoWidget，以便实现视频旋转和镜像。\n参考：https://github.com/mgardner99/490/blob/master/mainwindow.cpp。\n\n1.16\n(2020-02)\n右键菜单增加快捷键，右键菜单增加视频信息。\n\n1.15\n(2019-01)\n截图文件名改为：媒体文件名+当前进度时间。\n\n1.14\n(2018-12)\n支持打开xspf列表。\n\n1.13\n(2018-09)\n增加：写日志。\n修复Qt5.10按钮黑色背景。\n\n1.12\n(2018-06)\n增加：自动更新和手动更新。\n改动：历史记录限制到20条。\n\n1.11\n(2018-05)\n修复：从右键打开方式无法打开文件。\n(2018-04)\n修复：视频分辨率为0X0时，缩放出错。\n\n1.10\n(2018-03)\n修复：拖动进度条时，进度条被拉回的问题。\n修复：从历史记录打开视频没有修改文件名，导致剧情连拍文件名错误。\n修复：增加从Chrome扩展打开后，调试时窗口无法启动和从外部程序启动打开文件中断。\n\n1.9\n(2018-01)\n打开URL时自动粘贴剪贴板文字。\n\n1.8\n(2017-12)\n文件信息增加文件大小。\n增加视频缩放。\n\n1.7\n(2017-11)\n增加对直播API的解析，换鼠标拖动代码更平滑，增加windows版编译图标。\n(2017-10)\n增加历史记录。\n增加接收Chrome扩展传来的直播网址。\n修复m_bPressed没有初始化引起鼠标移动界面移动的Bug，音频封面Windows调试，取消会引起窗口宽度变化的关闭直播列表缩小窗宽，感谢Snail1991。\n如果播放的音乐有封面则显示。\n(2017-09)\n启动、暂停、停止显示广告。\n解决网络视频媒体信息频繁变更引起界面多余的缩放动作。\n\n1.6\n(2017-09)\n增加解析分号分隔的网络媒体字符串到播放列表。\n遍历媒体信息。\n\n1.5\n(2017-09)\n增加显示错误信息。\n(2017-08-20)\n增加拖放打开文件。\n\n1.4\n(2017-06)\n更新日志太长，消息框改成带滚动条的文本框。\n打开本地文件，自动隐藏直播列表。\n(2017-05)\n系统升级后出现有声音无视频，根据 https://bugreports.qt.io/browse/QTBUG-23761，卸载 sudo apt-get remove gstreamer1.0-vaapi 修复。\n\n1.3 (2017-04)\n记忆全屏前直播列表是否显示，以便退出全屏后恢复。\n直播列表做进主窗体内并支持显隐。\n\n1.2 (2017-03)\n增加打开方式打开文件。\n右键增加截图菜单。\n增加剧情连拍。\n增加截图。\n\n1.1 (2017-03)\n窗口标题增加台号。\n (2017-02)\n合并导入重复代码。\n加入逗号判断，解决导入崩溃。\n增加导入直播列表菜单。\n上一个、下一个按钮换台。\n增加直播列表。\n\n1.0 (2017-02)\n静音修改图标和拖动条。\n增加快进、快退。\n增加时间。\n修复拖动进度条卡顿BUG。\n全屏修改进度条样式。\n实现全屏。\n增加视频控件。\n增加控制栏。";
    QDialog *dialog = new QDialog;
    dialog->setWindowIcon(QIcon(":/icon.png"));
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox = new QVBoxLayout;
    QTextBrowser *textBrowser = new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox = new QHBoxLayout;
    QPushButton *btnConfirm = new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec() == QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::on_action_aboutQt_triggered()
{
    QMessageBox::aboutQt(NULL, "关于 Qt");
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰媒体播放器 3.0\n一款基于 Qt5 的媒体播放器。\n作者：海天鹰\nE-mail: sonichy@163.com\n主页：https://github.com/sonichy\n致谢：\n播放列表：http://blog.sina.com.cn/s/blog_74a7e56e0101agit.html\n获取媒体信息：https://www.zhihu.com/question/36859497");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void MainWindow::on_pushButton_play_clicked()
{
    playPause();
}

void MainWindow::on_pushButton_stop_clicked()
{
    player->stop();
}

void MainWindow::on_pushButton_seek_backward_clicked()
{
    player->setPosition(player->position() - 5000);
    GTI->setPlainText(ui->label_time->text());
    GTI->show();
    timer_information->start(3000);
}

void MainWindow::on_pushButton_seek_forward_clicked()
{
    player->setPosition(player->position() + 5000);
    GTI->setPlainText(ui->label_time->text());
    GTI->show();
    timer_information->start(3000);
}

void MainWindow::on_pushButton_skip_backward_clicked()
{
    playlist->previous();
    QMediaContent MC = playlist->media(playlist->currentIndex());
    player->setMedia(MC);
    player->play();
    QString surl = MC.canonicalUrl().toString();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if((*it)->toolTip(0) == surl){
            qDebug() << (*it)->text(0);
            ui->treeWidget->setCurrentItem(*it);
            QString s = (*it)->parent()->text(0) + (*it)->text(0);
            setWindowTitle(s);
            GTI->setPlainText(s);
            GTI->show();
            timer_information->start(3000);
            break;
        }
        ++it;
    }
}

void MainWindow::on_pushButton_skip_forward_clicked()
{
    playlist->next();
    QMediaContent MC = playlist->media(playlist->currentIndex());
    player->setMedia(MC);
    player->play();
    QString surl = MC.canonicalUrl().toString();
    QTreeWidgetItemIterator it(ui->treeWidget);
    while (*it) {
        if((*it)->toolTip(0) == surl){
            qDebug() << (*it)->text(0);
            ui->treeWidget->setCurrentItem(*it);
            QString s = (*it)->parent()->text(0) + (*it)->text(0);
            setWindowTitle(s);
            GTI->setPlainText(s);
            GTI->show();
            timer_information->start(3000);
            break;
        }
        ++it;
    }
}

void MainWindow::on_pushButton_mute_clicked()
{
    on_action_volumeMute_triggered();
}

void MainWindow::on_pushButton_playbackMode_clicked()
{
    qDebug() << playlist->playbackMode();
    if(playlist->playbackMode() == QMediaPlaylist::Sequential){
        ui->pushButton_playbackMode->setIcon(QIcon(":/PlaybackModeCurrentItemInLoop"));
        ui->pushButton_playbackMode->setToolTip("单曲循环");
        playlist->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
    }else if(playlist->playbackMode() == QMediaPlaylist::CurrentItemInLoop){
        ui->pushButton_playbackMode->setIcon(QIcon(":/PlaybackModeLoop"));
        ui->pushButton_playbackMode->setToolTip("列表循环");
        playlist->setPlaybackMode(QMediaPlaylist::Loop);
    }else if(playlist->playbackMode() == QMediaPlaylist::Loop){
        ui->pushButton_playbackMode->setIcon(QIcon(":/PlaybackModeRandom"));
        playlist->setPlaybackMode(QMediaPlaylist::Random);
        ui->pushButton_playbackMode->setToolTip("列表随机");
    }else if(playlist->playbackMode() == QMediaPlaylist::Random){
        ui->pushButton_playbackMode->setIcon(QIcon(":/PlaybackModeSequential"));
        ui->pushButton_playbackMode->setToolTip("顺序播放");
        playlist->setPlaybackMode(QMediaPlaylist::Sequential);
    }
}

void MainWindow::on_action_fullscreen_triggered()
{
    EEFullscreen();
}

void MainWindow::on_pushButton_fullscreen_clicked()
{
    EEFullscreen();
}

void MainWindow::enterFullscreen()
{
    isListShow = ui->widgetZY->isVisible();
    showFullScreen();
    ui->menuBar->hide();
    ui->controlPanel->hide();
    ui->statusBar->hide();
    ui->sliderProgress->hide();
    setCursor(QCursor(Qt::BlankCursor));
    ui->sliderProgress->setStyleSheet("QSlider{ background-color:black; }"
                                      "QSlider::groove:Horizontal { height:5px; border-color: #333333;}" //主体
                                      "QSlider::sub-page:Horizontal { background-color: #020202; }" // 已滑过
                                      "QSlider::add-page:Horizontal { background-color: #111111; }" // 未滑过
                                      "QSlider::handle:Horizontal { height: 10px; width:10px; margin:-5px 5px -5px 5px; border-radius:5px; background:#000000; border:1px solid #333333}"); // 拖柄
    PMAFullscreen->setText("退出全屏&X");
    if (isListShow)
        ui->widgetZY->setVisible(false);
    ui->centralWidget->setStyleSheet("color:gray; background-color:black;");
    fitDesktop();
}

void MainWindow::exitFullscreen()
{
    if (isFullScreen()) {
        showNormal();
        setCursor(QCursor(Qt::ArrowCursor));
        ui->menuBar->show();
        ui->controlPanel->show();
        ui->statusBar->show();
        ui->sliderProgress->show();
        ui->sliderProgress->setStyleSheet("");
        PMAFullscreen->setText("全屏&F");
        ui->centralWidget->setStyleSheet("");
        if (isListShow)
            ui->widgetZY->setVisible(isListShow);
        scale(sr);
    }
}

void MainWindow::EEFullscreen()
{
    if (isFullScreen()) {
        exitFullscreen();
    } else {
        enterFullscreen();
    }
}

void MainWindow::durationChange(qint64 d)
{
    ui->sliderProgress->setMaximum(d);
    //qDebug() << "player->duration() =" << player->duration() << d;
    QTime t(0,0,0);
    t = t.addMSecs(d);
    STimeDuration = t.toString("hh:mm:ss");
}

void MainWindow::positionChange(qint64 p)
{
    if (!ui->sliderProgress->isSliderDown())
        ui->sliderProgress->setValue(p);
    setSTime(p);
}

void MainWindow::setSTime(qint64 v)
{
    QTime t(0,0,0);
    t = t.addMSecs(v);
    QString STimeElapse = t.toString("HH:mm:ss");
    //    t.setHMS(0,0,0);
    //    t = t.addMSecs(player->duration());
    //    QString STimeDuration = t.toString("hh:mm:ss");
    ui->label_time->setText(STimeElapse + "/" + STimeDuration);
    ui->sliderProgress->setToolTip(STimeElapse);
}

void MainWindow::volumeChange(int v)
{
    ui->sliderVolume->setValue(v);
    ui->sliderVolume->setToolTip(QString::number(v));
    GTI->setPlainText("音量：" + QString::number(v));
    GTI->show();
    timer_information->start(3000);
    if (v == 0) {
        ui->pushButton_mute->setIcon(QIcon::fromTheme("audio-volume-muted"));
    } else if (v < 33) {
        ui->pushButton_mute->setIcon(QIcon::fromTheme("audio-volume-low"));
    } else if (v < 66) {
        ui->pushButton_mute->setIcon(QIcon::fromTheme("audio-volume-medium"));
    } else {
        ui->pushButton_mute->setIcon(QIcon::fromTheme("audio-volume-high"));
    }
}

void MainWindow::mutedChange(bool muted)
{
    if (muted) {
        ui->pushButton_mute->setIcon(QIcon::fromTheme("audio-volume-muted"));
        ui->sliderVolume->setValue(0);
    } else {
        volumeChange(player->volume());
    }
}

void MainWindow::playPause()
{
    //qDebug() << "state=" << player->state();
    if (player->state() == QMediaPlayer::PlayingState) {
        player->pause();
    } else if (player->state() == QMediaPlayer::PausedState) {
        player->play();
    } else if (player->state() == QMediaPlayer::StoppedState) {
        player->play();
    }
}

void MainWindow::createPopmenu()
{
    popmenu = new QMenu();
    PMAPlay = new QAction("播放&P", this);
    PMAFullscreen = new QAction("全屏&F", this);
    PMACapture = new QAction("截图&C", this);
    PMAInfo = new QAction("信息&I", this);
    popmenu->addAction(PMAPlay);
    popmenu->addAction(PMAFullscreen);
    popmenu->addAction(PMACapture);
    popmenu->addAction(PMAInfo);
    connect(PMAPlay, SIGNAL(triggered()), this, SLOT(playPause()));
    connect(PMAFullscreen, SIGNAL(triggered()), this, SLOT(EEFullscreen()));
    connect(PMACapture, SIGNAL(triggered()), this, SLOT(on_action_capture_triggered()));
    connect(PMAInfo, SIGNAL(triggered()), this, SLOT(on_action_info_triggered()));

    QAction *action_rotate = new QAction("旋转镜像", this);
    popmenu->addAction(action_rotate);
    QMenu *menu = new QMenu;
    QAction *action_rotate_ccw = new QAction("逆时针旋转", this);
    menu->addAction(action_rotate_ccw);
    QAction *action_rotate_cw = new QAction("顺时针旋转", this);
    menu->addAction(action_rotate_cw);
    QAction *action_mirror_horizontal = new QAction("水平镜像", this);
    menu->addAction(action_mirror_horizontal);
    QAction *action_mirror_vertical = new QAction("垂直镜像", this);
    menu->addAction(action_mirror_vertical);
    action_rotate->setMenu(menu);
    //视频旋转：https://github.com/mgardner99/490/blob/master/mainwindow.cpp#L318
    connect(action_rotate_ccw, &QAction::triggered, [=](){
        qreal x = GVI->boundingRect().width()/2.0;
        qreal y = GVI->boundingRect().height()/2.0;
        angle -= 90;
        GVI->setTransform(QTransform().translate(x,y).rotate(angle).translate(-x,-y));
        qDebug() << GVI->size();
    });
    connect(action_rotate_cw, &QAction::triggered, [=](){
        qreal x = GVI->boundingRect().width() / 2.0;
        qreal y = GVI->boundingRect().height() / 2.0;
        angle += 90;
        GVI->setTransform(QTransform().translate(x,y).rotate(angle).translate(-x,-y));
    });
    connect(action_mirror_horizontal, &QAction::triggered, [=](){
        mh = -mh;
        qreal x = GVI->boundingRect().width() / 2.0;
        qreal y = GVI->boundingRect().height() / 2.0;
        GVI->setTransform(QTransform().translate(x, y).scale(mh, 1).translate(-x, -y));
    });
    connect(action_mirror_vertical, &QAction::triggered, [=](){
        mv = -mv;
        qreal x = GVI->boundingRect().width() / 2.0;
        qreal y = GVI->boundingRect().height() / 2.0;
        GVI->setTransform(QTransform().translate(x, y).scale(1, mv).translate(-x, -y));
    });
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event)
{
    popmenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::sliderVolumeMoved(int v)
{
    player->setVolume(v);
}

/*
void MainWindow::playTV(int row,int column)
{
    Q_UNUSED(column);
    QString surl = ui->tableWidget->item(row,1)->text();
    QString realurl = "";
    if(surl.contains("http://vdn.live.cntv.cn/api2/live.do?channel=")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QString hls_url = json.object().value("hls_url").toObject().value("hls4").toString();
        //qDebug() << "play(" << hls_url << ")";
        //player->play(hls_url);
        realurl = hls_url;
    }else if(surl.contains("http://apiv1.starschina.com/cms/v1.0/stream?")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QJsonArray play_urls = json.object().value("data").toObject().value("play_urls").toArray();
        QString urls = play_urls.last().toObject().value("urls").toArray()[0].toString();
        //qDebug() << "play(" << urls << ")";
        //player->play(urls);
        realurl = urls;
    }else if(surl.contains("http://live.api.hunantv.com/pc/getById?")){
        qDebug() << "analyze(" << surl << ")";
        QUrl url = QString(surl);
        QNetworkAccessManager *NAM = new QNetworkAccessManager;
        QNetworkRequest request(url);
        QNetworkReply *reply = NAM->get(request);
        QEventLoop loop;
        connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        loop.exec();
        QByteArray responseText = reply->readAll();
        QJsonDocument json = QJsonDocument::fromJson(responseText);
        QJsonArray html5Sources = json.object().value("data").toObject().value("html5Sources").toArray();
        QString urls = html5Sources.last().toObject().value("url").toString();
        //qDebug() << "play(" << urls << ")";
        //player->play(urls);
        realurl = urls;
    }else{
        //qDebug() << "play(" << surl << ")";
        //player->play(surl);
        realurl = surl;
    }
    qDebug() << "play(" << realurl << ")";
    player->setMedia(QUrl(realurl));
    player->play();
    addHistory(realurl);
    setWindowTitle(ui->tableWidget->item(row,0)->text());
    ui->statusBar->showMessage("播放 " + surl);
    setFocus();
    ui->action_scale1->setChecked(true);
}

void MainWindow::fillTable(QString filename)
{
    QFile *file = new QFile(filename);
    if(file->open(QIODevice::ReadOnly | QIODevice::Text)){
        ui->tableWidget->setRowCount(0);
        QTextStream ts(file);
        ts.setCodec("UTF-8");
        QString s = ts.readAll();
        file->close();
        QStringList line = s.split("\n");
        int j=0;
        for(int i=0; i<line.size(); i++){
            ui->tableWidget->insertRow(i);
            if(QFileInfo(filename).suffix() == "m3u8"){
                if(line.at(i).contains("#EXTINF:")){
                    QStringList SL = line.at(i).split(",");
                    QString title = SL.at(1);
                    ui->tableWidget->setItem(j,0,new QTableWidgetItem(title.replace(".m3u8","")));
                }else if(line.at(i).startsWith("http")){
                    ui->tableWidget->setItem(j,1,new QTableWidgetItem(line.at(i)));
                    j++;
                }
            }else{
                if(line.at(i).contains(",")){
                    QStringList strlist = line.at(i).split(",");
                    ui->tableWidget->setItem(i,0,new QTableWidgetItem(strlist.at(0)));
                    ui->tableWidget->setItem(i,1,new QTableWidgetItem(strlist.at(1).split("#").at(0)));
                }else{
                    ui->tableWidget->setItem(i,0,new QTableWidgetItem("【"+line.at(i)+"】"));
                    ui->tableWidget->setItem(i,1,new QTableWidgetItem(""));
                }
            }
        }
        ui->tableWidget->resizeColumnsToContents();//适应宽度
        ui->statusBar->showMessage("导入 " + filename + "，共" + QString::number(ui->tableWidget->rowCount()) + "个节目");
    }
}
*/

void MainWindow::mousePressEvent(QMouseEvent *event)
{
//    Q_UNUSED(event);
//    if (isFullScreen()) {
//        GTI->setPlainText(ui->label_time->text());
//        GTI->show();
//        timer_information->start(3000);
//    }
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if (player) {
        EEFullscreen();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //无效
    //Q_UNUSED(event);
//    qDebug() << event;
//    if (isFullScreen()) {
//        setCursor(QCursor(Qt::ArrowCursor));
//        ui->sliderProgress->show();
//        QTimer::singleShot(3000, this, [=]{
//            ui->sliderProgress->hide();
//            setCursor(QCursor(Qt::BlankCursor));
//        });
//    }
}

void MainWindow::metaDataChange()
{
    //qDebug() << "metaDataChange" << player->metaData(QMediaMetaData::Resolution);
    if(player->metaData(QMediaMetaData::Resolution) != QVariant::Invalid && !isFullScreen() && !isMaximized()){
        if(widthV0 != player->metaData(QMediaMetaData::Resolution).toSize().width() || heightV0 != player->metaData(QMediaMetaData::Resolution).toSize().height()){
            int wl = 0;
            if (ui->widgetZY->isVisible())
                wl = ui->widgetZY->width();
            widthV0 = player->metaData(QMediaMetaData::Resolution).toSize().width();
            heightV0 = player->metaData(QMediaMetaData::Resolution).toSize().height();
            widthV = widthV0;
            heightV = heightV0;
            scene->setSceneRect(0, 0, widthV, heightV);
            GVI->setSize(QSizeF(widthV, heightV));
            resize(widthV + wl, heightV + ui->controlPanel->height() + 60);
        }
    }
    if (!player->metaData(QMediaMetaData::ThumbnailImage).isNull()) {
        QImage imageCover = player->metaData(QMediaMetaData::ThumbnailImage).value<QImage>();
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //if(e->mimeData()->hasFormat("text/uri-list")) //只接收文件列表
    e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e) //释放对方时，执行的操作
{
    QList<QUrl> urls = e->mimeData()->urls();
    if (urls.isEmpty())
        return;

    QString fileName = urls.first().toLocalFile();

    foreach (QUrl u, urls) {
        qDebug() << u.toString();
    }
    qDebug() << urls.size();

    if (fileName.isEmpty())
        return;

    open(fileName);
}

void MainWindow::errorHandle(QMediaPlayer::Error error)
{
    switch (error) {
    case QMediaPlayer::NoError:
        break;
    case QMediaPlayer::ResourceError:
        ui->statusBar->showMessage("ResourceError");
        break;
    case QMediaPlayer::FormatError:
        ui->statusBar->showMessage("FormatError");
        break;
    case QMediaPlayer::NetworkError:
        ui->statusBar->showMessage("NetworkError");
        break;
    case QMediaPlayer::AccessDeniedError:
        ui->statusBar->showMessage("AccessDeniedError");
        break;
    case QMediaPlayer::ServiceMissingError:
        ui->statusBar->showMessage("ServiceMissingError");
        break;
    default:
        break;
    }
}

void MainWindow::analyze()
{
    QByteArray BA = QByteArray::fromPercentEncoding(dialogUrl->ui->lineEdit->text().toUtf8());
    QString surl = BA;
    dialogUrl->ui->lineEdit->setText(surl);
    if (!surl.isEmpty()) {
        ui->widgetZY->hide();
        if (surl.contains(".m3u8")) {
            player->setMedia(QUrl(surl));
            player->play();
            setWindowTitle(QFileInfo(surl).fileName());
            ui->statusBar->showMessage(surl);
            addHistory(surl);
        } else if (surl.contains(";")) {
            QStringList clip = surl.split(";");
            MPLurl->clear();
            dialogUrl->ui->tableWidget->setRowCount(0);
            for (int i=0; i<clip.size(); i++) {
                MPLurl->addMedia(QUrl(clip.at(i)));
                dialogUrl->ui->tableWidget->insertRow(i);
                dialogUrl->ui->tableWidget->setItem(i,0,new QTableWidgetItem(QFileInfo(clip.at(i)).fileName()));
            }
            dialogUrl->ui->tableWidget->resizeColumnsToContents();
            player->setPlaylist(MPLurl);
            player->play();
        } else {
            player->setMedia(QUrl(surl));
            player->play();
            setWindowTitle(QFileInfo(surl).fileName());
            ui->statusBar->showMessage(surl);
            addHistory(surl);
        }
        ui->action_scale1->setChecked(true);
    }
}

void MainWindow::MPLCIChange(int index)
{
    if (index != -1) {
        dialogUrl->ui->tableWidget->setCurrentCell(index,0);
        QString surl = MPLurl->currentMedia().canonicalUrl().toString();
        setWindowTitle(QString::number(index+1) + ":" + QFileInfo(surl).fileName());
        ui->statusBar->showMessage(surl);
        addHistory(surl);
    }
}

void MainWindow::playURL(int r,int c)
{
    Q_UNUSED(c);
    MPLurl->setCurrentIndex(r);
    player->play();
}

void MainWindow::stateChange(QMediaPlayer::State state)
{
    //qDebug() << state;
    if (state == QMediaPlayer::PlayingState) {
        ui->pushButton_play->setIcon(QIcon::fromTheme("media-playback-pause"));
    } else if (state == QMediaPlayer::PausedState) {
        ui->pushButton_play->setIcon(QIcon::fromTheme("media-playback-start"));
    } else if (state == QMediaPlayer::StoppedState) {
        ui->pushButton_play->setIcon(QIcon::fromTheme("media-playback-start"));
    }
}

void MainWindow::mediaStatusChange(QMediaPlayer::MediaStatus status)
{
    qDebug() << status;
    if (status == QMediaPlayer::EndOfMedia) {//自动播放下一个
        if (playlist->playbackMode() == QMediaPlaylist::CurrentItemInLoop || playlist->playbackMode() == QMediaPlaylist::Sequential) {
            playlist->next();
            qDebug() << "playlist"<< playlist->currentIndex();
            QMediaContent MC = playlist->media(playlist->currentIndex());
            player->setMedia(MC);
            player->play();
            QString surl = MC.canonicalUrl().toString();
            QTreeWidgetItemIterator it(ui->treeWidget);
            while (*it) {
                if ((*it)->toolTip(0) == surl) {
                    ui->treeWidget->setCurrentItem(*it);
                    QString s = (*it)->parent()->text(0) + (*it)->text(0);
                    setWindowTitle(s);
                    GTI->setPlainText(s);
                    GTI->show();
                    timer_information->start(3000);
                    break;
                }
                ++it;
            }
        }
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    //qDebug() << event->spontaneous();
    if (event->spontaneous() && !isFullScreen()  && (widthV != event->size().width() || heightV != event->size().height())) {
        widthV = event->size().width();
        heightV = event->size().height();
        //qDebug() << widthV << heightV;
        //scene->setSceneRect(QRect(0, 0, event->size().width(), event->size().height()));
        //GVI->setSize(event->size());
        scale(sr);
        float swh = (float) event->size().width() / event->size().height();
        ui->statusBar->showMessage("比例：" + QString::number(swh));
    }
}

void MainWindow::addHistory(QString url)
{
    QAction *action = new QAction(url, this);
    action->setData(url);
    ui->menu_history->addAction(action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(openHistory(bool)));
    if(ui->menu_history->actions().size()>20)
        ui->menu_history->removeAction(ui->menu_history->actions().at(0));
}

void MainWindow::openHistory(bool b)
{
    Q_UNUSED(b);
    QAction *action = qobject_cast<QAction*>(sender());
    filename = action->data().toString();
    if (filename.contains("://")) {
        player->setMedia(QUrl(filename));
        setWindowTitle(filename);
    } else {
        player->setMedia(QUrl::fromLocalFile(filename));
        setWindowTitle(QFileInfo(filename).fileName());
    }
    player->play();
}

void MainWindow::scale(float s)
{
    if (widthV != 0 || heightV != 0) {
        if (isMaximized())
            showNormal();
        scene->setSceneRect(QRect(0, 0, widthV * s, heightV * s));
        GVI->setSize(QSize(widthV * s, heightV * s));
        if (!isFullScreen()) {//窗口适应视频
            if (ui->widgetZY->isVisible()) {
                resize(widthV * s + ui->widgetZY->width(), heightV * s + ui->menuBar->height() + ui->sliderProgress->height() +ui->controlPanel->height() + ui->statusBar->height());
            } else {
                resize(widthV * s, heightV * s + ui->menuBar->height() + ui->sliderProgress->height() +ui->controlPanel->height() + ui->statusBar->height());
            }
        }
    }
}

void MainWindow::on_action_scale0_triggered()
{
    widthV = widthV0;
    heightV = heightV0;
    sr = 1;
    scale(sr);
    ui->action_scale0->setChecked(true);
}

void MainWindow::on_action_scale0_5_triggered()
{
    sr = 0.5;
    scale(sr);
    ui->action_scale0_5->setChecked(true);
}

void MainWindow::on_action_scale1_triggered()
{
    sr = 1;
    scale(sr);
    ui->action_scale1->setChecked(true);
}

void MainWindow::on_action_scale1_5_triggered()
{
    sr = 1.5;
    scale(sr);
    ui->action_scale1_5->setChecked(true);
}

void MainWindow::on_action_scale2_triggered()
{
    sr = 2;
    scale(sr);
    ui->action_scale2->setChecked(true);
}

void MainWindow::fitDesktop()
{
    if (isFullScreen()) {
        float s1 = (float) QApplication::desktop()->width() / widthV;
        float s2 = (float) QApplication::desktop()->height() / heightV;
        float s = qMin(s1, s2);
        scene->setSceneRect(QRect(0, 0, widthV * s, heightV * s));
        GVI->setSize(QSize(widthV * s, heightV * s));
    }
}

void MainWindow::sliderProgressMoved(int v)
{
    player->setPosition(v);
}

void MainWindow::sliderProgressReleased()
{
    player->setPosition(ui->sliderProgress->value());
}

void MainWindow::autoCheckVersion()
{
    checkVersion(false);
    //isManualUpdate = false;
}

void MainWindow::manualCheckVersion(bool b)
{
    Q_UNUSED(b);
    checkVersion(true);
}

void MainWindow::checkVersion(bool b)
{
    //qDebug() << b;
    QString surl = "https://raw.githubusercontent.com/sonichy/HTYMediaPlayer/master/version";
    QUrl url(surl);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray BA = reply->readAll();
    qDebug() << surl ;
    qDebug() << BA;
    QStringList SLVN = QString(BA).split(".");
    QStringList SLV = version.split(".");
    if((SLVN.at(0) > SLV.at(0)) || (SLVN.at(0) == SLV.at(0) && SLVN.at(1) > SLV.at(1))){
        QMessageBox::StandardButton SB = QMessageBox::question(NULL, "海天鹰播放器 更新", "检查到有更新，是否从 " + version + " 升级到 " + BA + " ?", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        if(SB == QMessageBox::Yes){
            surl = "https://codeload.github.com/sonichy/HTYMediaPlayer/zip/master";
            reply = manager.get(QNetworkRequest(QUrl(surl)));
            connect(reply, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(updateProgress(qint64,qint64)));
            connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
            loop.exec();
            //qDebug() << reply->readAll();
            QString filepath = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first() + "/HTYMP.zip" ;
            qDebug() << filepath;
            QFile file(filepath);
            file.open(QIODevice::WriteOnly);
            file.write(reply->readAll());
            file.close();
            ui->statusBar->showMessage("更新包已下载到 " + filepath);
        }
    }else if(b){
        QMessageBox::information(NULL, "海天鹰播放器 更新", "当前版本 " + version +" 是最新的版本！");
        //isManualUpdate = true;
    }else{
        ui->statusBar->showMessage("当前版本 " + version +" 是最新的版本！");
    }
}

void MainWindow::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    ui->statusBar->showMessage(QString("更新进度 %1 / %2").arg(SBytes(bytesReceived)).arg(SBytes(bytesTotal)));
}

QString MainWindow::SBytes(qint64 bytes)
{
    QString unit = "B";
    double dbytes = bytes * 1.0;
    if (bytes > 999999999) {
        dbytes /= (1024*1024*1024);
        unit = "GB";
    } else {
        if (bytes > 999999) {
            dbytes /= (1024*1024);
            unit = "MB";
        } else {
            if (bytes > 999) {
                dbytes /= 1024;
                unit = "KB";
            }
        }
    }
    return QString("%1%2").arg(QString::number(dbytes,'f',2)).arg(unit);
}

/*
bool MainWindow::importXSPF(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly | QFile::Text)) {
        QString s = "Error: Cannot read file " + fileName + " : " +file.errorString();
        qDebug() << s;
        appandText("log.txt", s);
        return false;
    }
    QDomDocument doc;
    QString errorMsg;
    int errorLine;
    int errorColumn;
    if (!doc.setContent(&file, false, &errorMsg, &errorLine, &errorColumn)) {
        file.close();
        QString s = "import " + fileName + " error: " + errorLine + "," + errorColumn + ": " + errorMsg;
        qDebug() << s;
        appandText("log.txt", s);
        return false;
    }
    file.close();
    ui->tableWidget->setRowCount(0);
    QDomElement root = doc.documentElement();
    //qDebug() << root.nodeName();
    QDomNodeList trackList = root.elementsByTagName("track");
    for(int i=0; i<trackList.length(); i++){
        QString title = trackList.at(i).toElement().elementsByTagName("title").at(0).toElement().text();
        QString location = trackList.at(i).toElement().elementsByTagName("location").at(0).toElement().text();
        //qDebug() << title << location;
        ui->tableWidget->insertRow(i);
        ui->tableWidget->setItem(i,0,new QTableWidgetItem(title));
        ui->tableWidget->setItem(i,1,new QTableWidgetItem(location));
    }
    ui->statusBar->showMessage("导入 " + QString::number(trackList.length()) +" 个节目");
    return true;
}
*/

void MainWindow::appandText(QString fileName, QString text)
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    text = "\n" + currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + " " + text;
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QIODevice::Append)) {
        file.write(text.toUtf8());
        file.close();
    }
}

QByteArray MainWindow::getReply(QString surl)
{
    QNetworkAccessManager *NAM = new QNetworkAccessManager;
    QNetworkRequest request;
    request.setUrl(QUrl(surl));
    QNetworkReply *reply = NAM->get(request);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
    return reply->readAll();
}

QByteArray MainWindow::postReply(QString surl,QString spost)
{
    QNetworkAccessManager *NAM = new QNetworkAccessManager;
    QNetworkRequest request;
    request.setUrl(QUrl(surl));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    QByteArray BA_post;
    BA_post.append(spost);
    QNetworkReply *reply = NAM->post(request, BA_post);
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    reply->deleteLater();
    return reply->readAll();
}

void MainWindow::search()
{
    QString s = ui->lineEdit_search->text();
    if (s != "") {
        ui->statusBar->showMessage("搜索\"" + s + "\"", 5000);
        QString surl = "http://www.zdziyuan.com/inc/s_feifei3zuidam3u8/?m=vod-search&wd=" + s;
        //QString surl = "https://cj.okzy.tv/inc/feifei3ckm3u8s/?m=vod-search&wd=" + s;
        QJsonDocument JD = QJsonDocument::fromJson(getReply(surl));
        int status = JD.object().value("status").toInt();
        if (status == 200) {
            //清空节点：https://www.cnblogs.com/azbane/p/11966343.html
            int count = TWI_search->childCount();
            while (count--) {
                QTreeWidgetItem *TWI = TWI_search->child(count);
                TWI_search->removeChild(TWI);
                delete TWI;
                //TWI = nullptr;
            }
            QJsonArray data = JD.object().value("data").toArray();
            for (int i=0; i<data.size(); i++) {
                QTreeWidgetItem *TWI1 = new QTreeWidgetItem(TWI_search);
                TWI1->setText(0, data[i].toObject().value("vod_name").toString());
                QString vod_url = data[i].toObject().value("vod_url").toString();
                QStringList SL1 = vod_url.split("\r\n");
                for (int k=0; k<SL1.length(); k++) {
                    if (SL1.at(k).endsWith(".m3u8")) {
                        QStringList SL2 = SL1.at(k).split("$");
                        QTreeWidgetItem *TWI2 = new QTreeWidgetItem(TWI1);
                        TWI2->setText(0, SL2.at(0));
                        TWI2->setToolTip(0, SL2.at(1));
                    }
                }
            }
            TWI_search->setExpanded(true);
        } else {
            ui->statusBar->showMessage("搜索错误", 5000);
        }
    }
}

void MainWindow::treeWidgetItemDoubleClicked(QTreeWidgetItem *item, int column)
{
    //Q_UNUSED(column);
    qDebug() << item->parent() << column << item->text(column) << item->toolTip(column);
    if (item->parent()) {
        setWindowTitle(item->parent()->text(0) + item->text(0));
        ui->statusBar->showMessage("播放 " + item->toolTip(0));
        player->setMedia(QUrl(item->toolTip(0)));
        player->play();

        playlist->clear();
        playlist->setPlaybackMode(QMediaPlaylist::Sequential);
        QTreeWidgetItemIterator it(item->parent());
        while (*it) {
            if ((*it)->toolTip(0) != "") {//过滤parent(第一个)
                playlist->addMedia(QUrl((*it)->toolTip(0)));
                //qDebug() << (*it)->text(0) << (*it)->toolTip(0) << playlist->addMedia(QUrl((*it)->toolTip(0)));
            }
            ++it;
        }
        int index = item->parent()->indexOfChild(item);
        qDebug() << "QTreeItem" << index;
        playlist->setCurrentIndex(index);
    }
}

void MainWindow::genTreeLive()
{
    QString filepath = QApplication::applicationDirPath() + "/live.txt";
    QFile *file = new QFile(filepath);
    if (file->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream TS(file);
        TS.setCodec("UTF-8");
        QString s = TS.readAll();
        file->close();
        QTreeWidgetItem *TWI = new QTreeWidgetItem(ui->treeWidget);
        TWI->setText(0, "直播");
        TWI->setToolTip(0, "");
        QStringList SL = s.split("\n");
        for (int i=0; i<SL.size(); i++) {
            QStringList SL1 = SL.at(i).split(",");
            QTreeWidgetItem *TWI1 = new QTreeWidgetItem(TWI);
            if (SL1.size() > 1) {
                TWI1->setText(0, SL1.at(0));
                TWI1->setToolTip(0, SL1.at(1));
            } else {
                qDebug() << "Split Error: Line" << i << SL.at(i);
            }
        }
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == ui->graphicsView) {
        if (event->type() == QEvent::MouseMove) { //晃动显示并开始计时
            if (isFullScreen()) {
                setCursor(Qt::ArrowCursor);
                ui->sliderProgress->show();
                ui->controlPanel->show();
                timer_controlPanel->start(3000);
                QString s = QDateTime::currentDateTime().toString("hh:mm");
                GTI->setPlainText(s);
                GTI->show();
                timer_information->start(3000);
            }
            return true;
        }
    } else if (obj == ui->controlPanel) {
        if (event->type() == QEvent::Enter) { //进入停止计时
            timer_controlPanel->stop();
            timer_information->stop();
        } else if (event->type() == QEvent::Leave) { //离开开始计时
            timer_controlPanel->start(3000);
            timer_information->start(3000);
        }
    } else {
        return QObject::eventFilter(obj, event);
    }
}