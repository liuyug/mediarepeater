#include <QtGui>
#include <QtWebKit>

#include "version.h"
#include "seekslider.h"
#include "videowidget.h"
#include "mainwindow.h"


MainWindow::MainWindow()
{
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    //audioDataOutput = new Phonon::AudioDataOutput(this);

    videoWidget = new QWidget(this);
    videoOutput = new MediaVideoWidget(videoWidget);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(videoOutput);
    videoWidget->setLayout(layout);

    mediaObject = new Phonon::MediaObject(this);

    mediaObject->setTickInterval(1000);
    connect(mediaObject, SIGNAL(tick(qint64)), this, SLOT(tick(qint64)));
    connect(mediaObject, SIGNAL(stateChanged(Phonon::State,Phonon::State)),
            this, SLOT(stateChanged(Phonon::State,Phonon::State)));
    connect(mediaObject, SIGNAL(currentSourceChanged(Phonon::MediaSource)),
            this, SLOT(sourceChanged(Phonon::MediaSource)));
    connect(mediaObject, SIGNAL(aboutToFinish()), this, SLOT(aboutToFinish()));
    connect(mediaObject, SIGNAL(hasVideoChanged(bool)),
            this, SLOT(hasVideoChanged(bool)));
    // for audio output data
    //connect(audioDataOutput, SIGNAL(endOfMedia(int)),
    //        this, SLOT(remainingSamples(int)));

    Phonon::createPath(mediaObject, audioOutput);
    //Phonon::createPath(mediaObject, audioDataOutput);
    Phonon::createPath(mediaObject, videoOutput);

    setupActions();
    setupMenus();
    setupUi();
    timeLcd->display("00:00:00");
    videoSize = 0;
}

MainWindow::~MainWindow()
{
    QSettings settings("default.pls", QSettings::IniFormat);
    settings.setIniCodec("utf-8");
    settings.clear();
    int count = playList->count();
    QString key;
    settings.setValue("playlist/NumberOfEntries", count);
    for (int i=0;i<count;i++) {
        key = playList->item(i)->text();
        QByteArray encodedString = QUrl::toPercentEncoding(medias[key], "/");
        settings.setValue(QString("playlist/File%1").arg(i), QString(encodedString));
    }
    settings.sync();
}

void MainWindow::addFiles()
{
    QStringList files = QFileDialog::getOpenFileNames(this, tr("Select Media Files"),
            QDesktopServices::storageLocation(QDesktopServices::MusicLocation),
            "All supported files (*.mp4 *.mp3 *.flac *.wav *.mkv *.avi *.flv)"
            ";;Video files (*.mkv *.flv *.mp4)"
            ";;Audio files(*.flac *.mp3 *.wav)"
            ";;All files(*.*)"
            );

    if (files.isEmpty())
        return;

    foreach (QString string, files) {
        QFileInfo fi(string);
        QString basename = fi.completeBaseName();
        playList->addItem(basename);
        medias[basename] = string;
    }
}

void MainWindow::removeFile()
{
    QList<QListWidgetItem *> items = playList->selectedItems();
    QList<QListWidgetItem *>::iterator item;
    QString playingFile;
    if (mediaObject->state() == Phonon::PlayingState
            || mediaObject->state() == Phonon::LoadingState) {
        playingFile = mediaObject->currentSource().fileName();
    }
    for (item=items.begin();item!=items.end();item++) {
        qDebug()<<"remove"<<(*item)->text();
        if (playingFile == medias[(*item)->text()]) {
            nextAction->trigger();
        }
        medias.remove((*item)->text());
        playList->takeItem(playList->row(*item));
        delete *item;
    }
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Media Repeater"),
            QString(tr("Media Repeater\n"
                "Version: %1\n"
                "Created by LIU Yugang <liuyug@gmail.com>\n"
                "Qt4.6 with Phonon"
              )).arg(version));
}

void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    switch (newState) {
        case Phonon::ErrorState:
            qDebug()<<"State Error"<<__FUNCTION__<<mediaObject->errorString();
            break;
        case Phonon::PlayingState:
            playAction->setEnabled(false);
            pauseAction->setEnabled(true);
            stopAction->setEnabled(true);
            seekForwardAction->setEnabled(true);
            seekBackwardAction->setEnabled(true);
            break;
        case Phonon::StoppedState:
            stopAction->setEnabled(false);
            playAction->setEnabled(true);
            pauseAction->setEnabled(false);
            seekForwardAction->setEnabled(false);
            seekBackwardAction->setEnabled(false);
            timeLcd->display("00:00:00");
            break;
        case Phonon::PausedState:
            pauseAction->setEnabled(false);
            stopAction->setEnabled(true);
            playAction->setEnabled(true);
            seekForwardAction->setEnabled(true);
            seekBackwardAction->setEnabled(true);
            break;
        case Phonon::BufferingState:
            break;
        default:
            ;
    }
}

void MainWindow::tick(qint64 time)
{
    QTime displayTime((time / 3600000) % 60, (time / 60000) % 60, (time / 1000) % 60);
    timeLcd->display(displayTime.toString("hh:mm:ss"));
    if(repeatABAction->isChecked()){
        int a, b;
        a = seekSlider->aPosition();
        b = seekSlider->bPosition();
        if(b > 0 && time > b){
            seekSlider->seek(a);
        }
    }
}

void MainWindow::listClicked(QListWidgetItem *item)
{
    mediaObject->stop();
    mediaObject->clearQueue();
    QString path = medias[item->text()];
    mediaObject->setCurrentSource(Phonon::MediaSource(path));

    mediaObject->play();

    QFileInfo fi = QFileInfo(path);
    QString message = QString(tr("Playing %1")).arg(fi.fileName());
    statusBar()->showMessage(message);
}

void MainWindow::sourceChanged(const Phonon::MediaSource &source)
{
    QFileInfo fi = QFileInfo(source.fileName());
    QList<QListWidgetItem *> list = playList->findItems(
            fi.completeBaseName(),
            Qt::MatchFixedString
            );
    if(!list.empty()) playList->setCurrentItem(list[0]);
    timeLcd->display("00:00:00");
    QString message = QString(tr("Playing %1")).arg(fi.fileName());
    qDebug()<<message;
    statusBar()->showMessage(message);
    // hasVideoChanged don't been emited on music.
    // only write here.
    QString playingFile = source.fileName();
    QStringList exts;
    exts.append(".html");
    exts.append(".htm");
    exts.append(".txt");
    foreach(QString ext, exts) {
        QString txt = playingFile;
        txt.replace(QRegExp("\\..*$"), ext);
        if (QFile::exists(txt)) {
            webView->load(QUrl::fromLocalFile(txt));
            break;
        }
    }
    seekSlider->seekableChanged(true);
    clearABAction->trigger();
}

void MainWindow::aboutToFinish()
{
    if (repeatAction->isChecked()) {
        mediaObject->enqueue(mediaObject->currentSource());
    } else {
        QFileInfo fi = QFileInfo(mediaObject->currentSource().fileName());
        QList<QListWidgetItem *> list = playList->findItems(
                fi.completeBaseName(),
                Qt::MatchFixedString
                );
        int next_idx;
        if (!list.empty()){
            next_idx = playList->row(list[0]) + 1;
            next_idx = (playList->count() > next_idx)? next_idx : 0;
            mediaObject->enqueue(Phonon::MediaSource(
                        medias[playList->item(next_idx)->text()]
                        ));
        }
    }
}

void MainWindow::remainingSamples(int value)
{
    qDebug()<<__FUNCTION__<<value;
}

void MainWindow::playNext()
{
    QFileInfo fi = QFileInfo(mediaObject->currentSource().fileName());
    QList<QListWidgetItem *> list = playList->findItems(
            fi.completeBaseName(),
            Qt::MatchFixedString
            );
    int next_idx;
    if(!list.empty()){
        next_idx = playList->row(list[0]) + 1;
        next_idx = (playList->count() > next_idx)? next_idx : 0;
        mediaObject->stop();
        mediaObject->setCurrentSource(Phonon::MediaSource(
                    medias[playList->item(next_idx)->text()]
                    ));
        mediaObject->play();
    } else {
        mediaObject->stop();
    }
}

void MainWindow::playPrevious()
{
    QFileInfo fi = QFileInfo(mediaObject->currentSource().fileName());
    QList<QListWidgetItem *> list = playList->findItems(
            fi.completeBaseName(),
            Qt::MatchFixedString
            );
    int next_idx;
    if(!list.empty()){
        next_idx = playList->row(list[0]) - 1;
        next_idx = (next_idx > -1)? next_idx : playList->count() - 1;
        mediaObject->stop();
        mediaObject->setCurrentSource(Phonon::MediaSource(
                    medias[playList->item(next_idx)->text()]
                    ));
        mediaObject->play();
    } else {
        mediaObject->stop();
    }
}

void MainWindow::markA()
{
    seekSlider->setAPosition();
    qDebug()<<"Mark A position";
    statusBar()->showMessage(tr("Mark A position"));
}

void MainWindow::markB()
{
    seekSlider->setBPosition();
    qDebug()<<"Mark B position";
    statusBar()->showMessage(tr("Mark B position"));
}

#define HIDE_VIDEO 0
void MainWindow::hasVideoChanged(bool hasVideo)
{
#if HIDE_VIDEO
    if (hasVideo) {
        videoOutput->enterVideoMode();
        if (!videoWidget->isVisible()) {
            videoWidget->show();
        }
    } else {
        videoOutput->enterImageMode();
        if (videoWidget->isVisible()) {
            videoWidget->hide();
        }
    }
#else
    QList<int> widgetSizes = vSplitter->sizes();
    qDebug()<<__FUNCTION__<<widgetSizes;
    if (hasVideo) {
        qDebug()<<videoSize;
        videoOutput->enterVideoMode();
        if (videoWidget->height() == 0) {
            widgetSizes[0] = videoSize>0?videoSize:240;
            widgetSizes[1] -= videoSize;
        }
    } else {
        videoOutput->enterImageMode();
        videoSize = widgetSizes[0];
        widgetSizes[1] += widgetSizes[0];
        widgetSizes[0] = 0;
    }
    qDebug()<<__FUNCTION__<<widgetSizes;
    vSplitter->setSizes(widgetSizes);
#endif
}

void MainWindow::setupActions()
{
    playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    playAction->setShortcut(tr(" "));
    playAction->setDisabled(true);
    pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    pauseAction->setShortcut(tr(" "));
    pauseAction->setDisabled(true);
    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setShortcut(tr("S"));
    stopAction->setDisabled(true);
    seekForwardAction = new QAction(style()->standardIcon(QStyle::SP_MediaSeekForward), tr("Forward"), this);
    seekForwardAction->setShortcut(Qt::Key_Right);
    seekForwardAction->setDisabled(true);
    seekBackwardAction = new QAction(style()->standardIcon(QStyle::SP_MediaSeekBackward), tr("Backward"), this);
    seekBackwardAction->setShortcut(Qt::Key_Left);
    seekBackwardAction->setDisabled(true);
    repeatAction = new QAction(QIcon::fromTheme("view-refresh"), tr("Repeat media"), this);
    repeatAction->setCheckable(true);
    repeatAction->setShortcut(tr("Ctrl+R"));
    nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
    previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
    addFilesAction = new QAction(tr("Add &Files"), this);
    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    aboutAction = new QAction(tr("A&bout"), this);
    aboutQtAction = new QAction(tr("About &Qt"), this);
    aAction = new QAction(tr("Mark A"), this);
    aAction->setShortcut(tr("A"));
    bAction = new QAction(tr("Mark B"), this);
    bAction->setShortcut(tr("B"));
    clearABAction = new QAction(tr("Clear AB"), this);
    clearABAction->setShortcut(tr("C"));
    repeatABAction = new QAction(tr("Repeat AB"), this);
    repeatABAction->setShortcut(tr("R"));
    repeatABAction->setCheckable(true);
    // for playlist
    addAction = new QAction(QIcon::fromTheme("list-add"), tr("Add item"), this);
    removeAction = new QAction(QIcon::fromTheme("list-remove"), tr("Remove item"), this);
    removeAllAction = new QAction(tr("Remove all items"), this);

    connect(playAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
    connect(pauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(nextAction, SIGNAL(triggered()), this, SLOT(playNext()));
    connect(previousAction, SIGNAL(triggered()), this, SLOT(playPrevious()));
    connect(addFilesAction, SIGNAL(triggered()), this, SLOT(addFiles()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(aAction, SIGNAL(triggered()), this, SLOT(markA()));
    connect(bAction, SIGNAL(triggered()), this, SLOT(markB()));
    connect(addAction, SIGNAL(triggered()), this, SLOT(addFiles()));
    connect(removeAction, SIGNAL(triggered()), this, SLOT(removeFile()));
}

void MainWindow::setupMenus()
{
    QMenu *fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(addFilesAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    QMenu *playMenu = menuBar()->addMenu(tr("&Play"));
    playMenu->addAction(playAction);
    playMenu->addAction(pauseAction);
    playMenu->addAction(stopAction);
    playMenu->addSeparator();
    playMenu->addAction(seekForwardAction);
    playMenu->addAction(seekBackwardAction);
    playMenu->addSeparator();
    playMenu->addAction(repeatAction);

    QMenu *repeatMenu = menuBar()->addMenu(tr("&Repeat"));
    repeatMenu->addAction(aAction);
    repeatMenu->addAction(bAction);
    repeatMenu->addAction(clearABAction);
    repeatMenu->addAction(repeatABAction);

    QMenu *aboutMenu = menuBar()->addMenu(tr("&Help"));
    aboutMenu->addAction(aboutAction);
    aboutMenu->addAction(aboutQtAction);
}

void MainWindow::setupUi()
{
    QIcon icon;
    icon = QIcon("mediarepeater.png");
    if(icon.isNull()) {
        icon = QIcon::fromTheme("mediarepeater.png");
    }
    setWindowIcon(icon);
    QToolBar *bar = new QToolBar;

    bar->addAction(playAction);
    bar->addAction(pauseAction);
    bar->addAction(stopAction);
    bar->addAction(repeatAction);
    bar->addAction(seekBackwardAction);
    bar->addAction(seekForwardAction);
    bar->addAction(aAction);
    bar->addAction(bAction);
    bar->addAction(repeatABAction);
    bar->addAction(clearABAction);

    seekSlider = new MediaSeekSlider(this);
    seekSlider->setMediaObject(mediaObject);
    connect(clearABAction, SIGNAL(triggered()), seekSlider, SLOT(clearABPosition()));
    connect(seekForwardAction, SIGNAL(triggered()), seekSlider, SLOT(seekForward()));
    connect(seekBackwardAction, SIGNAL(triggered()), seekSlider, SLOT(seekBackward()));

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setAudioOutput(audioOutput);
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);
    timeLcd->setDigitCount(8);

    QHBoxLayout *seekerLayout = new QHBoxLayout;
    seekerLayout->addWidget(seekSlider, 1);
    seekerLayout->addWidget(timeLcd, 0);

    QHBoxLayout *playbackLayout = new QHBoxLayout;
    playbackLayout->addWidget(bar);
    playbackLayout->addStretch();
    playbackLayout->addWidget(volumeSlider);

    QString defaultPage =tr("<h1>Welcome Media Repeater</h1>");
    webView = new QWebView();
    webView->setHtml(defaultPage);
    webView->show();

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(seekerLayout, 0);
    mainLayout->addLayout(playbackLayout, 0);
    mainLayout->addWidget(webView, 1);

    QWidget *controlPanel = new QWidget;
    controlPanel->setLayout(mainLayout);

    vSplitter = new QSplitter();
    vSplitter->setOrientation(Qt::Vertical);
    vSplitter->addWidget(videoWidget);
    vSplitter->addWidget(controlPanel);

    playList = new QListWidget();
    connect(playList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(listClicked(QListWidgetItem *)));
    connect(removeAllAction, SIGNAL(triggered()), playList, SLOT(clear()));

    QSettings settings("default.pls", QSettings::IniFormat);
    settings.setIniCodec("utf-8");
    int plsCount = settings.value("playlist/NumberOfEntries", 0).toInt();
    QString filepath, basename;
    QFileInfo fi;
    QByteArray ba;
    for (int i=0;i<plsCount;i++) {
        ba = settings.value(QString("playlist/File%1").arg(i)).toByteArray();
        filepath = QUrl::fromPercentEncoding(ba);
        fi.setFile(filepath);
        basename = fi.completeBaseName();
        playList->addItem(basename);
        medias[basename] = filepath;
    }

    QToolButton *tb = new QToolButton(playList);
    tb->setDefaultAction(removeAction);
    QMenu *removeMenu = new QMenu(tb);
    removeMenu->addAction(removeAction);
    removeMenu->addAction(removeAllAction);
    tb->setMenu(removeMenu);

    bar = new QToolBar;
    bar->addAction(addAction);
    bar->addWidget(tb);

    QVBoxLayout *plLayout = new QVBoxLayout;
    plLayout->addWidget(playList);
    plLayout->addWidget(bar);

    QWidget *plWidget = new QWidget;
    plWidget->setLayout(plLayout);

    QSplitter *hSplitter = new QSplitter();
    hSplitter->setOrientation(Qt::Horizontal);
    hSplitter->addWidget(vSplitter);
    hSplitter->addWidget(plWidget);

    QStatusBar *statusbar = new QStatusBar;
    setStatusBar(statusbar);

    setCentralWidget(hSplitter);
    setWindowTitle("Media Repeater");
    statusBar()->showMessage(tr("Ready"));
}

