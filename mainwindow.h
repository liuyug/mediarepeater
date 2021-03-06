#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <phonon/audiooutput.h>
#include <phonon/videowidget.h>
#include <phonon/seekslider.h>
#include <phonon/mediaobject.h>
#include <phonon/volumeslider.h>
#include <phonon/backendcapabilities.h>

class QAction;
class QListWidget;
class QLCDNumber;
class QWebView;
class QListWidgetItem;
class QSplitter;

class MediaSeekSlider;
class MediaVideoWidget;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    public:
        MainWindow();
        ~MainWindow();

        QSize sizeHint() const {
            return QSize(800, 600);
        }

    private Q_SLOTS:
        void addFiles();
        void removeFile();
        void about();
        void stateChanged(Phonon::State newState, Phonon::State oldState);
        void tick(qint64 time);
        void sourceChanged(const Phonon::MediaSource &source);
        void aboutToFinish();
        void playNext();
        void playPrevious();
        void hasVideoChanged(bool);
        void listClicked(QListWidgetItem * item);
        void markA();
        void markB();
        // audio output data
        void remainingSamples(int value);

    private:
        void setupActions();
        void setupMenus();
        void setupUi();

        MediaSeekSlider *seekSlider;
        Phonon::MediaObject *mediaObject;
        Phonon::AudioOutput *audioOutput;
        // Qt4.6 do not support AudioDataOutput
        //Phonon::AudioDataOutput *audioDataOutput;
        MediaVideoWidget *videoOutput;
        Phonon::VolumeSlider *volumeSlider;
        QMap<QString, QString> medias;

        QWidget *videoWidget;
        QSplitter *vSplitter;

        QAction *playAction;
        QAction *pauseAction;
        QAction *stopAction;
        QAction *repeatAction;
        QAction *seekForwardAction;
        QAction *seekBackwardAction;
        QAction *nextAction;
        QAction *previousAction;
        QAction *aAction;
        QAction *bAction;
        QAction *clearABAction;
        QAction *repeatABAction;
        QAction *addFilesAction;
        QAction *exitAction;
        QAction *aboutAction;
        QAction *aboutQtAction;

        // playlist
        QAction *addAction;
        QAction *removeAction;
        QAction *removeAllAction;

        QLCDNumber *timeLcd;
        QListWidget *playList;
        QWebView *webView;
        int videoSize;
};

#endif
