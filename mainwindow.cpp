#include "mainwindow.h"

#include <QCoreApplication>
#include <QRegExpValidator>
#include <QThread>
#include <QTimer>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    mVideoPlayState = QMediaPlayer::State::StoppedState;
    mVideoDuration = 0;
    mVideoCurrentPos = 0;

    initLayout();
    initPlayer();
    iniSetting();
}


MainWindow::~MainWindow()
{

}

void MainWindow::initLayout()
{
    mMainWidget = new QWidget(this);
    this->setCentralWidget(mMainWidget);

    mVideoWidget = new QVideoWidget(this);
    mVideoWidget->setVisible(false);

    QLabel* mPathTitle = new QLabel(this);
    mPathTitle->setText("輸入影片路徑：");
    mPathEdit = new QLineEdit(this);
    QHBoxLayout *mPathBoxLayout = new QHBoxLayout();
    mPathBoxLayout->addWidget(mPathTitle);
    mPathBoxLayout->addWidget(mPathEdit);

    QLabel* mStartTitle = new QLabel(this);
    mStartTitle->setText("開始時間(e.g 12:55 or 27)：");
    mStartPositionEdit = new QLineEdit(this);
    QLabel* mEndTitle = new QLabel(this);
    mEndTitle->setText("結束時間：");
    mEndPositionEdit = new QLineEdit(this);

    mDurationLabel = new QLabel(this);
    mDurationLabel->setMinimumWidth(80);

    QRegExp rx ("[0-9:]*");
    QRegExpValidator * v = new QRegExpValidator (rx, this);
    mStartPositionEdit->setValidator(v);
    mEndPositionEdit->setValidator(v);

    QHBoxLayout *mPositionBoxLayout = new QHBoxLayout();
    mPositionBoxLayout->addWidget(mStartTitle);
    mPositionBoxLayout->addWidget(mStartPositionEdit);
    mPositionBoxLayout->addWidget(mEndTitle);
    mPositionBoxLayout->addWidget(mEndPositionEdit);
    mPositionBoxLayout->addWidget(mDurationLabel);


    mProgressBar = new QProgressBar(this);
//    mProgressBar->setRange(0, 1000);

    mCurrentTimePositionLabel = new QLabel(this);
    mCurrentTimePositionLabel->setMinimumWidth(100);

    QHBoxLayout *mProgressBoxLayout = new QHBoxLayout();
    mProgressBoxLayout->addWidget(mProgressBar);
    mProgressBoxLayout->addWidget(mCurrentTimePositionLabel);


    mSetButton = new QPushButton(this);
    mSetButton->setText("Set");

    mPlayButton = new QPushButton(this);
    mPlayButton->setText("Play");

    mPauseButton = new QPushButton(this);
    mPauseButton->setText("Pause");

    mFullScreenButton = new QPushButton(this);
    mFullScreenButton->setText("Full Screen");

    QHBoxLayout *mHBoxLayout = new QHBoxLayout();
    mHBoxLayout->addWidget(mSetButton);
    mHBoxLayout->addWidget(mPlayButton);
    mHBoxLayout->addWidget(mPauseButton);
    mHBoxLayout->addWidget(mFullScreenButton);

    mStatusLabel = new QLabel(this);

    QVBoxLayout *mVBoxLayout = new QVBoxLayout();
    mVBoxLayout->addWidget(mVideoWidget, 1);
    mVBoxLayout->addLayout(mProgressBoxLayout);
    mVBoxLayout->addLayout(mPathBoxLayout);
    mVBoxLayout->addLayout(mPositionBoxLayout);
    mVBoxLayout->addLayout(mHBoxLayout);
    mVBoxLayout->addWidget(mStatusLabel);

    mMainWidget->setLayout(mVBoxLayout);

    connect(mSetButton, SIGNAL(clicked()), this, SLOT(s_set()));
    connect(mPlayButton, SIGNAL(clicked()), this, SLOT(s_play()));
    connect(mPauseButton, SIGNAL(clicked()), this, SLOT(s_pause()));
    connect(mFullScreenButton, SIGNAL(clicked()), this, SLOT(s_fullScreen()));
}

void MainWindow::initPlayer()
{
    mPlayer = new QMediaPlayer(this);
    mPlayer->setVideoOutput(mVideoWidget);
//    mPlayer->play();
\
    connect(mPlayer, SIGNAL(stateChanged(QMediaPlayer::State)), this, SLOT(s_stateChanged(QMediaPlayer::State)));
    connect(mPlayer, SIGNAL(durationChanged(qint64)), this, SLOT(s_progress_setMax(qint64)));
    connect(mPlayer, SIGNAL(positionChanged(qint64)), this, SLOT(s_progress_setValue(qint64)));

}

void MainWindow::iniSetting()
{
    mSettingFilePath = QCoreApplication::applicationDirPath() + "/setting.ini";
    qDebug() << "mSettingFilePath" << mSettingFilePath;
    mQSettings = new QSettings(mSettingFilePath, QSettings::IniFormat);

    QString video_path = mQSettings->value(INI_Key_Video_Path, "").toString();
    mPathEdit->setText(video_path);

    QString video_start_pos = mQSettings->value(INI_Key_Start_Pos, "").toString();
    mStartPositionEdit->setText(video_start_pos);

    QString video_end_pos = mQSettings->value(INI_Key_End_Pos, "").toString();
    mEndPositionEdit->setText(video_end_pos);

}

void MainWindow::toFormProgressText(qint64 pos, qint64 duration)
{
//    qDebug() << "toFormProgressText" << pos << "," << duration;
    qint64 array[2];
    array[0] = pos;
    array[1] = duration;

    qint64 minUnit = 1000 * 60;
    int arraySize = sizeof(array)/sizeof(array[0]);
    QString total_timeText = "";
    for(int i = 0 ; i < arraySize ; i++) {
        QString timeText = "";
        qint64 min = 0;
        qint64 sec = 0;

        qint64 targetValue = array[i];
        if (targetValue >=  minUnit) {
            sec = (targetValue % minUnit) / 1000;
            min = targetValue / minUnit;
            timeText = QString::number(min) + ":";
        } else {
            sec = targetValue / 1000;
        }
        timeText = timeText + QString::number(sec);

        if (i == 0) {
            timeText = timeText + " / ";
        }
        total_timeText = total_timeText + timeText;
    }
    mCurrentTimePositionLabel->setText(total_timeText);

}

void MainWindow::s_set()
{
    mManualPause = false;
    QString ui_path = mPathEdit->text();
    QFile videoFile(ui_path);
    if (videoFile.exists()) {
        mFilePath = ui_path;
    } else {
        mStatusLabel->setText("錯誤的檔案路徑");
        return;
    }

    mStartValue = -1;
    QString text_start = mStartPositionEdit->text();
    if (text_start.contains(":")) {
        QStringList v_list = text_start.split(":");
        if (v_list.length() == 2) {
            QString mm = v_list.at(0);
            QString ss = v_list.at(1);

            int mm_i = mm.toInt();
            int ss_i = ss.toInt();
            if (ss_i < 60 ) {
                 mStartValue = (mm_i * 60) + ss_i;
            }
        }
    } else if (text_start.length() > 0){
        mStartValue = text_start.toInt();
    } else if (text_start.length() == 0){
        mStartValue = 0;
    }

    if (mStartValue < 0) {
        mStatusLabel->setText("開始時間格式:錯誤");
        return;
    }

    mEndValue = -1;
    QString text_end = mEndPositionEdit->text();
    if (text_end.contains(":")) {
        QStringList v_list = text_end.split(":");
        if (v_list.length() == 2) {
            QString mm = v_list.at(0);
            QString ss = v_list.at(1);

            int mm_i = mm.toInt();
            int ss_i = ss.toInt();
            if (ss_i < 60 ) {
                 mEndValue = (mm_i * 60) + ss_i;
            }
        }
    } else if (text_end.length() > 0){
        mEndValue = text_end.toInt();
    }  else if (text_end.length() == 0){
        mEndValue = 0;
    }

    if (mEndValue < 0) {
        mStatusLabel->setText("結束時間格式:錯誤");
        return;
    }

    mPlayer->setMedia(QUrl::fromLocalFile(mFilePath));
    mPlayer->setPosition(mStartValue * 1000);
//    mPlayer->setVolume(50);
    mPlayer->play();

    mStatusLabel->setText("設定成功！");

    mQSettings->setValue(INI_Key_Video_Path, mFilePath);
    mQSettings->setValue(INI_Key_Start_Pos, mStartPositionEdit->text());
    mQSettings->setValue(INI_Key_End_Pos, mEndPositionEdit->text());

}

void MainWindow::s_play()
{
//    mPlayer->play();
//    qint64 duration = mPlayer->duration();
//    qDebug() << "duration" << duration;

    mManualPause = false;
    if (mVideoPlayState == QMediaPlayer::State::PausedState) {
        mPlayer->play();
    }

}

void MainWindow::s_pause()
{
    mManualPause = true;
    if (mVideoPlayState == QMediaPlayer::State::PlayingState) {
        mPlayer->pause();
    }

}

void MainWindow::s_fullScreen()
{
//    this->setFullScreen(true);

}

void MainWindow::s_progress_setMax(qint64 value)
{
//    qDebug() << "s_progress_setMax" << value;
    mProgressBar->setMaximum(value);
    mVideoDuration = value;

    if (mEndValue == 0) {
        mEndValue = value;
    }
    QString duration_text;
    duration_text = QString::number(mStartValue) + " -> " + QString::number(mEndValue);
    mDurationLabel->setText(duration_text);
}

void MainWindow::s_progress_setValue(qint64 value)
{
//    qDebug() << "s_progress_setValue" << value;
    mProgressBar->setValue(value);
    mVideoCurrentPos = value;

    toFormProgressText(mVideoCurrentPos, mVideoDuration);

    if (mVideoPlayState == QMediaPlayer::State::PlayingState) {
//        qDebug() << "mVideoCurrentPos" << mVideoCurrentPos << "mEndValue" << mEndValue << " bool" << (mVideoCurrentPos >= mEndValue * 1000);
        if (mVideoCurrentPos >= mEndValue * 1000) {
            mPlayer->pause();

            //等待一定秒數後，再重新播放
            QTimer* timer = new QTimer();
            timer->moveToThread(qApp->thread());
            timer->setSingleShot(true);
            QObject::connect(timer, &QTimer::timeout, timer, [=]()
            {
                // main thread
                if (mManualPause == false) {
                    mPlayer->setPosition(mStartValue * 1000);
                    mPlayer->play();
                }
                timer->deleteLater();
            });
            QMetaObject::invokeMethod(timer, "start", Qt::QueuedConnection, Q_ARG(int, 2 * 1000));

        }
    }

}

void MainWindow::s_stateChanged(QMediaPlayer::State state)
{
//    qDebug() << "s_stateChanged" << state;
    mVideoPlayState = state;

    if ( mVideoWidget->isVisible() == false &&
            mVideoPlayState == QMediaPlayer::State::PlayingState) {
        mVideoWidget->setVisible(true);
    }

}

