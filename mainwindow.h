#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "dialogurl.h"
#include "ui_dialogurl.h"
#include <QMainWindow>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QTableWidget>
#include <QMessageBox>
#include <QNetworkReply>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    void createPopmenu();
    void createTVList();
    void mouseDoubleClickEvent(QMouseEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);
    void mouseReleaseEvent(QMouseEvent*);
    void open(QString path);
    QString filename, STimeDuration, version;
    QMediaPlayer *player;
    QVideoWidget *video;
    QMenu *popmenu;
    QAction *PMAPlay,*PMAFullscreen,*PMACapture;
    QTableWidget *table;
    int volume;
    bool m_bPressed;
    float sr;
    QPoint m_point;
    QLabel *labelLogo,*labelTL;
    int widthV,heightV,widtho,heighto,listVisible;
    bool isListShow, isManualUpdate;
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);
    DialogURL *dialogUrl;
    QMediaPlaylist *MPLurl, *playlist;
    QPixmap pixmapAD;
    QString SB(qint64 b);
    void scale(float s);
    void setSTime(qint64);
    QString SBytes(qint64 bytes);
    bool importXSPF(QString fileName);
    void appandText(QString fileName, QString text);

private slots:
    void on_action_open_triggered();
    void on_action_openURL_triggered();
    void on_action_liveList_triggered();
    void on_action_liveImport_triggered();
    void on_action_quit_triggered();
    void on_action_fullscreen_triggered();
    void on_action_capture_triggered();
    void on_action_capture16_triggered();
    void on_action_info_triggered();
    void on_action_volumeUp_triggered();
    void on_action_volumeDown_triggered();
    void on_action_volumeMute_triggered();
    void on_action_help_triggered();
    void on_action_changelog_triggered();
    void on_action_aboutQt_triggered();
    void on_action_about_triggered();
    void on_btnPlay_clicked();
    void on_btnStop_clicked();
    void on_btnSeekB_clicked();
    void on_btnSeekF_clicked();
    void on_btnSkipB_clicked();
    void on_btnSkipF_clicked();
    void on_btnMute_clicked();
    void on_pushButton_playbackMode_clicked();
    void on_btnFullscreen_clicked();
    void on_btnList_clicked();
    void durationChange(qint64);
    void positionChange(qint64);
    void volumeChange(int);
    void mutedChange(bool muted);
    void enterFullscreen();
    void exitFullscreen();
    void EEFullscreen();
    void playPause();
    void keyPressEvent(QKeyEvent*);
    void contextMenuEvent(QContextMenuEvent*);
    void playTV(int,int);
    void fillTable(QString);
    void hideWidget();
    void metaDataChange();
    void showHideList();
    void errorHandle(QMediaPlayer::Error);
    void analyze();
    void playURL(int,int);
    void MPLCIChange(int);
    void stateChange(QMediaPlayer::State);
    void mediaStatusChange(QMediaPlayer::MediaStatus status);
    void resizeEvent(QResizeEvent*);
    void replyAD(QNetworkReply*);
    void addHistory(QString url);
    void openHistory(bool);
    void on_action_scale0_5_triggered();
    void on_action_scale1_triggered();
    void on_action_scale1_5_triggered();
    void on_action_scale2_triggered();
    void fitDesktop();
    void sliderProgressPressed();
    void sliderProgressReleased();
    //void sliderProgressValueChanged(int);
    void sliderProgressMoved(int);
    void sliderVolumeMoved(int);
    void autoCheckVersion();
    void manualCheckVersion(bool);
    void checkVersion(bool);
    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
};

#endif // MAINWINDOW_H
