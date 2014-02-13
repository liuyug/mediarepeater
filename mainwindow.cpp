#include <QtGui>
#include <QtWebKit>

#include "seekslider.h"
#include "videowidget.h"
#include "mainwindow.h"

MainWindow::MainWindow()
{
    audioOutput = new Phonon::AudioOutput(Phonon::VideoCategory, this);
    videoOutput = new MediaVideoWidget(this);
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

    Phonon::createPath(mediaObject, audioOutput);
    Phonon::createPath(mediaObject, videoOutput);

    setupActions();
    setupMenus();
    setupUi();
    timeLcd->display("00:00:00");
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
    QMessageBox::information(this, tr("About Media Repeater"),
            tr("The Media Repeater use Phonon - the multimedia"
                " framework that comes with Qt - to create a simple player."));
}

void MainWindow::stateChanged(Phonon::State newState, Phonon::State /* oldState */)
{
    switch (newState) {
        case Phonon::ErrorState:
            qDebug()<<"Error"<<mediaObject->errorString();
            break;
        case Phonon::PlayingState:
            playAction->setEnabled(false);
            pauseAction->setEnabled(true);
            stopAction->setEnabled(true);
            break;
        case Phonon::StoppedState:
            stopAction->setEnabled(false);
            playAction->setEnabled(true);
            pauseAction->setEnabled(false);
            timeLcd->display("00:00:00");
            break;
        case Phonon::PausedState:
            pauseAction->setEnabled(false);
            stopAction->setEnabled(true);
            playAction->setEnabled(true);
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
}

void MainWindow::aboutToFinish()
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
        mediaObject->enqueue(Phonon::MediaSource(
                    medias[playList->item(next_idx)->text()]
                    ));
    }
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

void MainWindow::setRepeatAB()
{
    qDebug()<<"Repeat AB"<<repeatABAction->isChecked();
    statusBar()->showMessage(tr("Repeat AB"));
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

void MainWindow::hasVideoChanged(bool hasVideo)
{
    if (hasVideo) 
        videoOutput->enterVideoMode();
    else
        videoOutput->enterImageMode();
}

void MainWindow::setupActions()
{
    playAction = new QAction(style()->standardIcon(QStyle::SP_MediaPlay), tr("Play"), this);
    playAction->setDisabled(true);
    pauseAction = new QAction(style()->standardIcon(QStyle::SP_MediaPause), tr("Pause"), this);
    pauseAction->setDisabled(true);
    stopAction = new QAction(style()->standardIcon(QStyle::SP_MediaStop), tr("Stop"), this);
    stopAction->setDisabled(true);
    nextAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipForward), tr("Next"), this);
    previousAction = new QAction(style()->standardIcon(QStyle::SP_MediaSkipBackward), tr("Previous"), this);
    addFilesAction = new QAction(tr("Add &Files"), this);
    addFilesAction->setShortcut(tr("Ctrl+F"));
    exitAction = new QAction(tr("E&xit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    aboutAction = new QAction(tr("A&bout"), this);
    aboutQtAction = new QAction(tr("About &Qt"), this);
    aAction = new QAction(tr("Mark A"), this);
    aAction->setShortcut(tr("Ctrl+A"));
    bAction = new QAction(tr("Mark B"), this);
    bAction->setShortcut(tr("Ctrl+B"));
    clearABAction = new QAction(tr("Clear AB"), this);
    repeatABAction = new QAction(tr("Repeat AB"), this);
    repeatABAction->setCheckable(true);
    repeatABAction->setShortcut(tr("Ctrl+R"));
    // for playlist
    addAction = new QAction(tr("+"), this);
    removeAction = new QAction(tr("-"), this);

    connect(playAction, SIGNAL(triggered()), mediaObject, SLOT(play()));
    connect(pauseAction, SIGNAL(triggered()), mediaObject, SLOT(pause()) );
    connect(stopAction, SIGNAL(triggered()), mediaObject, SLOT(stop()));
    connect(nextAction, SIGNAL(triggered()), this, SLOT(playNext()));
    connect(previousAction, SIGNAL(triggered()), this, SLOT(playPrevious()));
    connect(addFilesAction, SIGNAL(triggered()), this, SLOT(addFiles()));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(repeatABAction, SIGNAL(triggered()), this, SLOT(setRepeatAB()));
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
    QToolBar *bar = new QToolBar;

    bar->addAction(playAction);
    bar->addAction(pauseAction);
    bar->addAction(stopAction);
    bar->addAction(previousAction);
    bar->addAction(nextAction);
    bar->addAction(aAction);
    bar->addAction(bAction);
    bar->addAction(repeatABAction);
    bar->addAction(clearABAction);

    seekSlider = new MediaSeekSlider(this);
    seekSlider->setMediaObject(mediaObject);
    connect(clearABAction, SIGNAL(triggered()), seekSlider, SLOT(clearABPosition()));

    volumeSlider = new Phonon::VolumeSlider(this);
    volumeSlider->setAudioOutput(audioOutput);
    volumeSlider->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

    QPalette palette;
    palette.setBrush(QPalette::Light, Qt::darkGray);

    timeLcd = new QLCDNumber;
    timeLcd->setPalette(palette);
    timeLcd->setDigitCount(8);

    QHBoxLayout *seekerLayout = new QHBoxLayout;
    seekerLayout->addWidget(seekSlider);
    seekerLayout->addWidget(timeLcd);

    QHBoxLayout *playbackLayout = new QHBoxLayout;
    playbackLayout->addWidget(bar);
    playbackLayout->addStretch();
    playbackLayout->addWidget(volumeSlider);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(videoOutput);
    mainLayout->addLayout(seekerLayout);
    mainLayout->addLayout(playbackLayout);

    QWidget *controlPanel = new QWidget;
    controlPanel->setLayout(mainLayout);

    QString defaultPage =tr("<h1>Welcome Media Repeater</h1>");
    webView = new QWebView();
    webView->setHtml(defaultPage);
    webView->show();

    QVBoxLayout *wvLayout = new QVBoxLayout;
    wvLayout->addWidget(webView);
    QWidget *wvWidget = new QWidget;
    wvWidget->setLayout(wvLayout);

    QSplitter *vSplitter = new QSplitter();
    vSplitter->setOrientation(Qt::Vertical);
    vSplitter->addWidget(controlPanel);
    vSplitter->addWidget(wvWidget);

    playList = new QListWidget();
    connect(playList, SIGNAL(itemActivated(QListWidgetItem *)),
            this, SLOT(listClicked(QListWidgetItem *)));

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

    bar = new QToolBar;
    bar->addAction(addAction);
    bar->addAction(removeAction);

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
