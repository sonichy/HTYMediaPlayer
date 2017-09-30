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
//#include <QPalette>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->mainToolBar->hide();
    ui->btnSkipB->setIcon(style()->standardIcon(QStyle::SP_MediaSkipBackward));
    ui->btnSeekB->setIcon(style()->standardIcon(QStyle::SP_MediaSeekBackward));
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    ui->btnSkipF->setIcon(style()->standardIcon(QStyle::SP_MediaSkipForward));
    ui->btnSeekF->setIcon(style()->standardIcon(QStyle::SP_MediaSeekForward));
    ui->btnStop->setIcon(style()->standardIcon(QStyle::SP_MediaStop));
    ui->btnMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));

    video = new QVideoWidget;    

//    labelLogo=new QLabel(this);
//    labelLogo->setPixmap(QPixmap(":/icon.png"));
//    labelLogo->adjustSize();
//    labelLogo->show();

    ui->vbox->addWidget(video);
    video->setMouseTracking(true);
    player = new QMediaPlayer;
    player->setVolume(100);
    player->setVideoOutput(video);
    video->show();
    move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);

    connect(player,SIGNAL(durationChanged(qint64)),this,SLOT(durationChange(qint64)));
    connect(player,SIGNAL(positionChanged(qint64)),this,SLOT(positionChange(qint64)));
    connect(player,SIGNAL(volumeChanged(int)),this,SLOT(volumeChange(int)));
    connect(player,SIGNAL(metaDataChanged()),this,SLOT(metaDataChange()));
    connect(player,SIGNAL(error(QMediaPlayer::Error)),this,SLOT(errorHandle(QMediaPlayer::Error)));
    connect(player,SIGNAL(stateChanged(QMediaPlayer::State)),SLOT(stateChange(QMediaPlayer::State)));

    connect(new QShortcut(QKeySequence(Qt::Key_O),this), SIGNAL(activated()),this, SLOT(on_action_open_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_U),this), SIGNAL(activated()),this, SLOT(on_action_openURL_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_T),this), SIGNAL(activated()),this, SLOT(on_action_liveList_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_I),this), SIGNAL(activated()),this, SLOT(on_action_liveImport_triggered()));    
    connect(new QShortcut(QKeySequence(Qt::Key_Escape),this), SIGNAL(activated()),this, SLOT(exitFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Return),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Enter),this), SIGNAL(activated()),this, SLOT(EEFullscreen()));
    connect(new QShortcut(QKeySequence(Qt::Key_Space),this), SIGNAL(activated()),this, SLOT(playPause()));
    connect(new QShortcut(QKeySequence(Qt::Key_Left),this), SIGNAL(activated()),this, SLOT(on_btnSeekB_clicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_Right),this), SIGNAL(activated()),this, SLOT(on_btnSeekF_clicked()));
    connect(new QShortcut(QKeySequence(Qt::Key_P),this), SIGNAL(activated()),this, SLOT(on_action_capture_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Up),this), SIGNAL(activated()),this, SLOT(on_action_volumeUp_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_Down),this), SIGNAL(activated()),this, SLOT(on_action_volumeDown_triggered()));
    connect(new QShortcut(QKeySequence(Qt::Key_M),this), SIGNAL(activated()),this, SLOT(on_action_volumeMute_triggered()));
    connect(ui->sliderProgress,SIGNAL(sliderReleased()),this,SLOT(setMPPosition()));
    connect(ui->sliderProgress,SIGNAL(valueChanged(int)),this,SLOT(setSTime(int)));
    connect(ui->sliderVolume,SIGNAL(sliderReleased()),this,SLOT(setVolume()));

    ui->tableWidget->setColumnHidden(1,true);
    ui->tableWidget->setStyleSheet("QTableWidget{color:white; background-color:black;} QTableWidget::item:selected{background-color:#222222;}");
    connect(ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(playTV(int,int)));
    createPopmenu();

    widtho = width() - video->width();
    heighto = height() - video->height();
    qDebug() << widtho << heighto;

    QStringList Largs=QApplication::arguments();
    qDebug() << Largs;
    if(Largs.length()>1){
        open(Largs.at(1));        
    }

    fillTable("tv.txt");

    dialogUrl = new DialogURL(this);
    connect(dialogUrl->ui->pushButtonAnalyze,SIGNAL(pressed()),this,SLOT(analyze()));
    connect(dialogUrl->ui->tableWidget,SIGNAL(cellClicked(int,int)),this,SLOT(playURL(int,int)));
    MPLurl = new QMediaPlaylist;
    MPLurl->setPlaybackMode(QMediaPlaylist::Sequential);
    connect(MPLurl,SIGNAL(currentIndexChanged(int)),this,SLOT(MPLCIChange(int)));

    //player->setMedia(QUrl("qrc:/icon.png"));
//    player->setMedia(QUrl("http://www.bydauto.com.cn/uploads/image/20170825/1503648763587493.jpg"));
//    player->play();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_open_triggered()
{
    if(filename==""){
        filename = QFileDialog::getOpenFileName(this, "打开媒体文件", ".");
    }else{
        filename = QFileDialog::getOpenFileName(this, "打开媒体文件", filename);
    }
    if(!filename.isEmpty()){
        open(filename);
    }
}

void MainWindow::open(QString path)
{
    player->setMedia(QUrl::fromLocalFile(path));
    player->play();
    setWindowTitle(QFileInfo(path).fileName());
    ui->statusBar->showMessage("打开 " + path);
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    ui->tableWidget->hide();
}

void MainWindow::on_action_openURL_triggered()
{
    dialogUrl->show();
}

void MainWindow::on_btnList_clicked()
{
    showHideList();
}

void MainWindow::on_action_liveList_triggered()
{
    showHideList();
}

void MainWindow::showHideList()
{
    if(ui->tableWidget->isVisible()){
        ui->tableWidget->setVisible(false);
        resize(QSize(width()-ui->tableWidget->width(),height()));
    }else{
        ui->tableWidget->setVisible(true);
        resize(QSize(width()+ui->tableWidget->width(),height()));
    }
}

void MainWindow::on_action_liveImport_triggered(){
    if(filename==""){        
        filename = QFileDialog::getOpenFileName(this, "打开文本", "tv.txt", "文本文件(*.txt)");
    }else{
        filename = QFileDialog::getOpenFileName(this, "打开文本", filename, "文本文件(*.txt)");
    }
    if(!filename.isEmpty()){
       fillTable(filename);
    }
}

void MainWindow::on_action_quit_triggered()
{
    qApp->quit();
}

void MainWindow::on_action_capture_triggered(){
    //if(isFullScreen()){ui->sliderProgress->hide();}
    QEventLoop eventloop;
    QTimer::singleShot(500, &eventloop, SLOT(quit()));
    eventloop.exec();
    QDateTime now = QDateTime::currentDateTime();
    QString path = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first()+"/"+now.toString("yyyyMMddHHmmss")+".jpg";
    //QPixmap pixmap = QPixmap::grabWindow(QApplication::desktop()->winId(),video->mapToGlobal(video->pos()).x(),video->mapToGlobal(video->pos()).y(),video->width(),video->height());
    QPixmap pixmap = QGuiApplication::primaryScreen()->grabWindow(0,video->mapToGlobal(video->pos()).x(),video->mapToGlobal(video->pos()).y(),video->width()-1,video->height()-1);
    pixmap.save(path,0,100);
    ui->statusBar->showMessage("截图 "+path);
    //if(isFullScreen()){ui->sliderProgress->show();}
}

QString SB(qint64 b){
    QString s="";
    if(b>999999999){
        s=QString::number(b/(1024*1024*1024.0),'f',2)+"GB";
    }else{
        if(b>999999){
            s=QString::number(b/(1024*1024.0),'f',2)+"MB";
        }else{
            if(b>999){
                s=QString::number(b/1024.0,'f',2)+"KB";
            }else{
                s=QString::number(b/1.0,'f',2)+"B";
            }
        }
    }
    return s;
}

void MainWindow::on_action_capture16_triggered(){
    QEventLoop eventloop;
    QTimer::singleShot(500, &eventloop, SLOT(quit()));
    eventloop.exec();
    QPixmap pixc[16];
    player->pause();
    player->setMuted(true);
    QPixmap pixDS=QPixmap(1280+50,960+80+40);
    pixDS.fill(Qt::white);
    int k=0;
    QPainter painter(&pixDS);
    painter.drawPixmap(20,10,QPixmap("icon.png").scaled(60,60));
    painter.drawText(300,30,"文件名称：" + QFileInfo(filename).fileName());
    painter.drawText(300,60,"文件大小：" + SB(QFileInfo(filename).size()));
    painter.drawText(500,60,"视频尺寸：" + QString::number(widthv) + " X " + QString::number(heightv));
    QTime t(0,0,0);
    t=t.addMSecs(player->duration());
    QString STimeTotal=t.toString("hh:mm:ss");
    painter.drawText(700,60,"视频时长："+STimeTotal);
    for(qint64 i=player->duration()/16;i<player->duration();i+=player->duration()/16){
        player->setPosition(i);
        pixc[k]=QGuiApplication::primaryScreen()->grabWindow(0,video->mapToGlobal(video->pos()).x(),video->mapToGlobal(video->pos()).y(),video->width(),video->height()).scaled(320,240);
        QTime t(0,0,0);
        t=t.addMSecs(player->position());
        QString STimeElapse=t.toString("hh:mm:ss");
        QPainter painter(&pixc[k]);
        painter.setPen(QPen(Qt::white));
        painter.drawText(270,235,STimeElapse);
        QEventLoop eventloop;
        QTimer::singleShot(500, &eventloop, SLOT(quit()));
        eventloop.exec();
        k++;
    }
    k=0;
    for(int y=0;y<4;y++){
        for(int x=0;x<4;x++){
            painter.drawPixmap(x*(320+10)+10,y*(240+10)+80,pixc[k]);
            k++;
        }
    }
    QFont font("Arial",20,QFont::Normal,false);
    painter.setFont(font);
    painter.drawText(100,50,"海天鹰播放器");
    QString path=QStandardPaths::writableLocation(QStandardPaths::DesktopLocation)+ "/Summary_"+QFileInfo(filename).baseName()+".jpg";
    pixDS.save(path,0,100);
    player->play();
    player->setMuted(false);
}

void MainWindow::on_action_info_triggered()
{
    QStringList SLMD = player->availableMetaData();
    //qDebug() << SLMD;
    QString s="";
    for(int i=0; i<SLMD.size(); i++){
        if(SLMD.at(i)=="PixelAspectRatio" || SLMD.at(i)=="Resolution"){
            s += SLMD.at(i) + ": " + QString::number(player->metaData(SLMD.at(i)).toSize().width()) + " X " + QString::number(player->metaData(SLMD.at(i)).toSize().height()) + "\n";
        }else{
            s += SLMD.at(i) + ": " + player->metaData(SLMD.at(i)).toString() + "\n";
        }
    }
    QMessageBox MBInfo(QMessageBox::NoIcon, "信息", s);
    QImage imageCover = player->metaData(QMediaMetaData::ThumbnailImage).value<QImage>();
    MBInfo.setIconPixmap(QPixmap::fromImage(imageCover));
    MBInfo.setWindowIcon(QIcon(":/icon.png"));
    MBInfo.exec();
}

void MainWindow::on_action_volumeUp_triggered()
{
    player->setVolume(player->volume()+1);
}

void MainWindow::on_action_volumeDown_triggered()
{
    player->setVolume(player->volume()-1);
}

void MainWindow::on_action_volumeMute_triggered()
{
    if(player->isMuted()){
        player->setMuted(false);
        ui->sliderVolume->setValue(volume);
        //ui->btnMute->setIcon(QIcon("volume-high.png"));
        ui->btnMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolume));
    }else{
        volume=player->volume();
        player->setMuted(true);
        ui->sliderVolume->setValue(0);
        //ui->btnMute->setIcon(QIcon("volume-muted.png"));
        ui->btnMute->setIcon(style()->standardIcon(QStyle::SP_MediaVolumeMuted));
    }
}

void MainWindow::on_action_help_triggered(){
    QMessageBox MB(QMessageBox::NoIcon, "帮助", "快捷键：\nO\t打开文件\nU\t打开网址\nT\t直播列表\nI\t导入直播文件\n空格\t播放、暂停\n回车、双击\t全屏、退出全屏\nEsc\t退出全屏\n上\t增加音量\n下\t减小音量\n左 \t快退\n右\t快进\nM\t静音\nP\t截图");
    MB.exec();
}

void MainWindow::on_action_changelog_triggered()
{
    QString s="1.6\n(2017-09)\n解决网络视频媒体信息频繁变更引起界面多余的缩放动作。\n增加解析分号分隔的网络媒体字符串到播放列表。\n遍历媒体信息。\n\n1.5\n(2017-09)\n增加显示错误信息。\n(2017-08-20)\n增加拖放打开文件。\n\n1.4\n(2017-06)\n更新日志太长，消息框改成带滚动条的文本框。\n打开本地文件，自动隐藏直播列表。\n(2017-05)\n系统升级后出现有声音无视频，根据 https://bugreports.qt.io/browse/QTBUG-23761，卸载 sudo apt-get remove gstreamer1.0-vaapi 修复。\n\n1.3 (2017-04)\n记忆全屏前直播列表是否显示，以便退出全屏后恢复。\n直播列表做进主窗体内并支持显隐。\n\n1.2 (2017-03)\n增加打开方式打开文件。\n右键增加截图菜单。\n增加剧情连拍。\n增加截图。\n\n1.1 (2017-03)\n窗口标题增加台号。\n (2017-02)\n合并导入重复代码。\n加入逗号判断，解决导入崩溃。\n增加导入直播列表菜单。\n上一个、下一个按钮换台。\n增加直播列表。\n\n1.0 (2017-02)\n静音修改图标和拖动条。\n增加快进、快退。\n增加时间。\n修复拖动进度条卡顿BUG。\n全屏修改进度条样式。\n实现全屏。\n增加视频控件。\n增加控制栏。";
    QDialog *dialog=new QDialog;
    dialog->setWindowTitle("更新历史");
    dialog->setFixedSize(400,300);
    QVBoxLayout *vbox=new QVBoxLayout;
    QTextBrowser *textBrowser=new QTextBrowser;
    textBrowser->setText(s);
    textBrowser->zoomIn();
    vbox->addWidget(textBrowser);
    QHBoxLayout *hbox=new QHBoxLayout;
    QPushButton *btnConfirm=new QPushButton("确定");
    hbox->addStretch();
    hbox->addWidget(btnConfirm);
    hbox->addStretch();
    vbox->addLayout(hbox);
    dialog->setLayout(vbox);
    dialog->show();
    connect(btnConfirm, SIGNAL(clicked()), dialog, SLOT(accept()));
    if(dialog->exec()==QDialog::Accepted){
        dialog->close();
    }
}

void MainWindow::on_action_aboutQt_triggered()
{
    QMessageBox::aboutQt(NULL, "关于 Qt");
}

void MainWindow::on_action_about_triggered()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "关于", "海天鹰媒体播放器 1.6\n一款基于 Qt 的媒体播放器。\n作者：黄颖\nE-mail: sonichy@163.com\n主页：sonichy.96.lt\n致谢：\n播放列表：http://blog.sina.com.cn/s/blog_74a7e56e0101agit.html\n获取媒体信息：https://www.zhihu.com/question/36859497");
    aboutMB.setIconPixmap(QPixmap(":/icon.png"));
    aboutMB.exec();
}

void MainWindow::on_btnPlay_clicked()
{
    playPause();
}

void MainWindow::on_btnStop_clicked()
{
    player->stop();
    ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
}

void MainWindow::on_btnSeekB_clicked()
{
    player->setPosition(player->position()-5000);
}

void MainWindow::on_btnSeekF_clicked()
{
    player->setPosition(player->position()+5000);
}

void MainWindow::on_btnSkipB_clicked()
{
    if(ui->tableWidget->currentRow()>0){
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()-1,0);
        playTV(ui->tableWidget->currentRow(),1);
    }
}

void MainWindow::on_btnSkipF_clicked()
{
    if(ui->tableWidget->currentRow()<ui->tableWidget->rowCount()-1){
        ui->tableWidget->setCurrentCell(ui->tableWidget->currentRow()+1,0);
        playTV(ui->tableWidget->currentRow(),1);
    }
}

void MainWindow::on_btnMute_clicked()
{
    on_action_volumeMute_triggered();
}

void MainWindow::on_action_fullscreen_triggered(){
    EEFullscreen();
}

void MainWindow::on_btnFullscreen_clicked(){
    EEFullscreen();
}

void MainWindow::enterFullscreen(){
    isListShow=ui->tableWidget->isVisible();
    showFullScreen();
    ui->menuBar->hide();
    ui->controlPanel->hide();
    ui->statusBar->hide();
    //ui->sliderProgress->hide();
    //ui->sliderProgress->setStyleSheet("background-color:black;");
    ui->sliderProgress->setStyleSheet("QSlider{ background-color:black; }"
                                      "QSlider::groove:Horizontal { height:5px; border-color: #333333;}" //主体
                                      "QSlider::sub-page:Horizontal { background-color: #020202; }" // 已滑过
                                      "QSlider::add-page:Horizontal { background-color: #111111; }" // 未滑过
                                      "QSlider::handle:Horizontal { height: 10px; width:10px; margin:-5px 5px -5px 5px; border-radius:5px; background:#000000; border:1px solid #333333}"); // 拖柄
    PMAFullscreen->setText("退出全屏");
    if(isListShow)ui->tableWidget->setVisible(false);
    ui->verticalLayout_2->setMargin(0);
    //labelLogo->move((video->width()-labelLogo->width())/2, (video->height()-labelLogo->height())/2);
}

void MainWindow::exitFullscreen()
{
    showNormal();
    ui->menuBar->show();
    ui->controlPanel->show();
    ui->statusBar->show();
    ui->sliderProgress->show();
    ui->sliderProgress->setStyleSheet("");
    PMAFullscreen->setText("全屏");
    ui->verticalLayout_2->setMargin(1);
    if(isListShow)ui->tableWidget->setVisible(isListShow);
    //labelLogo->move((video->width()-labelLogo->width())/2, (video->height()-labelLogo->height())/2);
}

void MainWindow::EEFullscreen(){
    if(isFullScreen()){
        exitFullscreen();
    }else{
        enterFullscreen();
    }    
}

void MainWindow::durationChange(qint64 d){
    ui->sliderProgress->setMaximum(d);
    //qDebug() << "player->duration()=" << player->duration() << d;
}

void MainWindow::positionChange(qint64 p){
    ui->sliderProgress->setValue(p);
}

void MainWindow::setSTime(int v){
    QTime t(0,0,0);
    t=t.addMSecs(v);
    QString sTimeElapse=t.toString("hh:mm:ss");
    t.setHMS(0,0,0);
    t=t.addMSecs(player->duration());
    QString sTimeTotal=t.toString("hh:mm:ss");
    ui->labelTimeVideo->setText(sTimeElapse+"/"+sTimeTotal);
    //ui->sliderProgress->setToolTip(sTimeElapse);
}

void MainWindow::volumeChange(int v){
    ui->sliderVolume->setValue(v);
    ui->sliderVolume->setToolTip(QString::number(v));
//    labelTL->setText("音量："+QString::number(v));
//    labelTL->show();
//    QTimer::singleShot(3000,this,SLOT(hideWidget()));
}

void MainWindow::playPause(){
    //qDebug() << "state=" << player->state();
    if(player->state()==QMediaPlayer::PlayingState){
        player->pause();
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
    }else if(player->state()==QMediaPlayer::PausedState){
        player->play();
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));        
    }else if(player->state()==QMediaPlayer::StoppedState){
        player->play();
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event){
    if(event->key() ==  Qt::Key_P){
        playPause();
    }else if(event->key() == Qt::Key_Enter){
        EEFullscreen();
    }
}

void MainWindow::createPopmenu(){
    popmenu = new QMenu();
    PMAPlay = new QAction("播放",this);
    PMAFullscreen = new QAction("全屏",this);
    PMACapture = new QAction("截图",this);
    popmenu->addAction(PMAPlay);
    popmenu->addAction(PMAFullscreen);
    popmenu->addAction(PMACapture);
    connect(PMAPlay, SIGNAL(triggered()), this, SLOT(playPause()));
    connect(PMAFullscreen, SIGNAL(triggered()), this, SLOT(EEFullscreen()));
    connect(PMACapture, SIGNAL(triggered()), this, SLOT(on_action_capture_triggered()));
}

void MainWindow::contextMenuEvent(QContextMenuEvent *event){
    //pop_menu->clear();
    popmenu->exec(QCursor::pos());
    event->accept();
}

void MainWindow::setMPPosition(){
    player->setPosition(ui->sliderProgress->value());
}

void MainWindow::setVolume(){
    player->setVolume(ui->sliderVolume->value());
}

void MainWindow::playTV(int row,int column){
    Q_UNUSED(column);
    QString surl=ui->tableWidget->item(row,1)->text();
    //qDebug() << surl;
    //if(surl!=""){
        player->setMedia(QUrl(surl));
        player->play();
        setWindowTitle(ui->tableWidget->item(row,0)->text());
        ui->statusBar->showMessage("直播 "+surl);
        ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
    //}    
}

void MainWindow::fillTable(QString filename){
    QFile *file=new QFile(filename);
    if(file->open(QIODevice::ReadOnly | QIODevice::Text)){
        ui->tableWidget->setRowCount(0);
        QTextStream ts(file);
        QString s=ts.readAll();
        file->close();
        QStringList line=s.split("\n");
        for(int i=0;i<line.size();i++){
            ui->tableWidget->insertRow(i);
            if(line.at(i).contains(",")){
                QStringList strlist=line.at(i).split(",");                
                ui->tableWidget->setItem(i,0,new QTableWidgetItem(strlist.at(0)));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem(strlist.at(1).split("#").at(0)));
            }else{
                ui->tableWidget->setItem(i,0,new QTableWidgetItem("【"+line.at(i)+"】"));
                ui->tableWidget->setItem(i,1,new QTableWidgetItem(""));
            }
        }
        ui->tableWidget->resizeColumnsToContents();//适应宽度
        ui->statusBar->showMessage("导入 " + filename + "，共" + QString::number(ui->tableWidget->rowCount()) + "个节目");
    }
}

void MainWindow::hideWidget()
{
    //labelTL->hide();
    ui->sliderProgress->hide();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_bPressed = true;
        m_point = event->pos();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    //qDebug() << "MouseMove:" << pos();
    if(m_bPressed)
        move(event->pos() - m_point + pos());
//    if(isFullScreen()){
//        ui->sliderProgress->show();
//        QTimer::singleShot(3000,this,SLOT(hideWidget()));
//    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    m_bPressed = false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent* event)
{
    Q_UNUSED(event);
    if(player){
        EEFullscreen();
    }
}

void MainWindow::metaDataChange()
{
    qDebug() << "metaDataChange" << player->metaData(QMediaMetaData::Resolution);
    if(player->metaData(QMediaMetaData::Resolution)!=QVariant::Invalid && !isFullScreen() && !isMaximized()){
        if(widthv != player->metaData(QMediaMetaData::Resolution).toSize().width() || heightv != player->metaData(QMediaMetaData::Resolution).toSize().height()){
            int wl=0;
            if(ui->tableWidget->isVisible())wl=ui->tableWidget->width();
            widthv = player->metaData(QMediaMetaData::Resolution).toSize().width();
            heightv = player->metaData(QMediaMetaData::Resolution).toSize().height();
            resize(widthv+wl, heightv + ui->controlPanel->height() + 60);
            //move((QApplication::desktop()->width() - width())/2, (QApplication::desktop()->height() - height())/2);
        }
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *e)
{
    //if(e->mimeData()->hasFormat("text/uri-list")) //只能打开文本文件
        e->acceptProposedAction(); //可以在这个窗口部件上拖放对象
}

void MainWindow::dropEvent(QDropEvent *e) //释放对方时，执行的操作
{
    QList<QUrl> urls = e->mimeData()->urls();
    if(urls.isEmpty())
        return ;

    QString fileName = urls.first().toLocalFile();

    foreach (QUrl u, urls) {
        qDebug() << u.toString();
    }
    qDebug() << urls.size();

    if(fileName.isEmpty())
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
    QString surl = dialogUrl->ui->lineEdit->text();
    if(!surl.isEmpty()){
        if(surl.contains(".m3u8")){
            player->setMedia(QUrl(surl));
            player->play();
            setWindowTitle(QFileInfo(surl).fileName());
            ui->statusBar->showMessage(surl);
            ui->btnPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause));
        }
        if(surl.contains(";")){
            QStringList clip = surl.split(";");
            MPLurl->clear();
            dialogUrl->ui->tableWidget->setRowCount(0);
            for(int i=0; i<clip.size(); i++){
                MPLurl->addMedia(QUrl(clip.at(i)));
                dialogUrl->ui->tableWidget->insertRow(i);
                dialogUrl->ui->tableWidget->setItem(i,0,new QTableWidgetItem(QFileInfo(clip.at(i)).fileName()));
            }
            dialogUrl->ui->tableWidget->resizeColumnsToContents();
            player->setPlaylist(MPLurl);
            player->play();
            ui->tableWidget->hide();
        }
    }
}

void MainWindow::MPLCIChange(int index)
{
    if(index!=-1){
        dialogUrl->ui->tableWidget->setCurrentCell(index,0);
        setWindowTitle(QString::number(index+1) + ":" + QFileInfo(MPLurl->currentMedia().canonicalUrl().toString()).fileName());
        ui->statusBar->showMessage(MPLurl->currentMedia().canonicalUrl().toString());
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
    qDebug() << state;
    if(state == QMediaPlayer::PlayingState){
//        labelLogo->hide();
    }
    if(state == QMediaPlayer::PausedState){
//        labelLogo->show();
    }
    if(state == QMediaPlayer::StoppedState){        
//        labelLogo->show();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    //labelLogo->move((video->width()-labelLogo->width())/2, (video->height()-labelLogo->height())/2);
}
