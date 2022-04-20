#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include<QMediaPlaylist>
#include<QMediaPlayer>
#include<QVideoWidget>
#include<QFileDialog>
#include<QProgressBar>
#include<QSlider>
#include <QBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QSettings>
#include <QResizeEvent>


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    int mCurrentProgressValue;//單位是秒
    int mStartValue;//單位是秒 這是人去設定的開始秒數
    int mEndValue;//單位是秒 這是人去設定的結束秒數

    qint64 mVideoDuration;//單位是毫秒 播放影片的總長度
    qint64 mVideoCurrentPos;//單位是毫秒 目前正播放到哪了
    bool mManualPause = false;

    QMediaPlayer::State mVideoPlayState;

    QString mFilePath;

private:

    QString mSettingFilePath;
    QSettings *mQSettings;

    QString INI_Key_Video_Path = "INI_Key_Video_Path";
    QString INI_Key_Start_Pos = "INI_Key_Start_Pos";
    QString INI_Key_End_Pos = "INI_Key_End_Pos";

    QWidget* mMainWidget;
    QProgressBar* mProgressBar;
    QLabel *mCurrentTimePositionLabel;
    QLineEdit* mPathEdit;
    QLineEdit* mStartPositionEdit;//輸入的格式是 mm:ss
    QLineEdit* mEndPositionEdit;//輸入的格式是 mm:ss
    QLabel* mDurationLabel;

    QPushButton* mSetButton;
    QPushButton* mPlayButton;
    QPushButton* mPauseButton;
    QPushButton* mFullScreenButton;

    QLabel* mStatusLabel;

    QMediaPlayer* mPlayer;
    QVideoWidget* mVideoWidget;

    void initLayout();
    void initPlayer();
    void iniSetting();

    void toFormProgressText(qint64 pos, qint64 duration);

private slots:
    void s_set();
    void s_play();
    void s_pause();
    void s_fullScreen();
    void s_progress_setMax(qint64 value);
    void s_progress_setValue(qint64 value);


    void s_stateChanged(QMediaPlayer::State state);


};

#endif // MAINWINDOW_H
