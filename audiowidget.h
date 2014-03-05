#ifndef AUDIOWIDGET_H
#define AUDIOWIDGET_H

#include <QtGui>
#include <phonon/audiodataoutput.h>

class MediaAudioWidget: public QWidget
{
    Q_OBJECT
    private:
        QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > m_audiodata;
        int m_sampleRate;
        int m_channel;
        int m_samplesPerPixel;
        QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> > m_paintingData;
        int m_elapse;

    public Q_SLOTS:
        void dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& data);
        void paintEvent(QPaintEvent *event);

    public:
        MediaAudioWidget();
        void init(int rate);
        QSize sizeHint() const {
            return QSize(100, 200);
        }
};

#endif /* AUDIOWIDGET_H */
