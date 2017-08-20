#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QTableWidget>

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
    void mouseDoubleClickEvent(QMouseEvent* event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void open(QString path);
    QString filename;
    QMediaPlayer *player;
    QVideoWidget *video;
    QMenu *popmenu;
    QAction *PMAPlay,*PMAFullscreen,*PMACapture;
    QTableWidget *table;
    int volume;
    bool m_bPressed;
    QPoint m_point;
    //QLabel *labelTL;
    int widthv,heightv,widtho,heighto,listVisible;
    bool isListShow;
    void dragEnterEvent(QDragEnterEvent*);
    void dropEvent(QDropEvent*);

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
    void on_btnFullscreen_clicked();
    void on_btnList_clicked();
    void durationChange(qint64);
    void positionChange(qint64);
    void volumeChange(int);
    void setMPPosition();
    void setVolume();
    void enterFullscreen();
    void exitFullscreen();
    void EEFullscreen();
    void playPause();
    void keyPressEvent(QKeyEvent *event);
    void contextMenuEvent(QContextMenuEvent *event);
    void setSTime(int);
    void playTV(int, int);
    void fillTable(QString);
    void hideWidget();
    void getMetaData();
    void showHideList();
};

#endif // MAINWINDOW_H
