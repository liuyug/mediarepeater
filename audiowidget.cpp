
#include <QtGui>
#include <phonon/audiodataoutput.h>

#include "audiowidget.h"


MediaAudioWidget::MediaAudioWidget()
{
    m_channel = 2;
    m_samplesPerPixel = 256;
    m_sampleRate = 44100;
}

void MediaAudioWidget::init(int rate)
{
    m_sampleRate = rate;
    m_paintingData.clear();
    qDebug()<<"Sample Rate:"<<rate;
}

void MediaAudioWidget::dataReceived(const QMap<Phonon::AudioDataOutput::Channel, QVector<qint16> >& data)
{
    m_audiodata[Phonon::AudioDataOutput::LeftChannel] += data[Phonon::AudioDataOutput::LeftChannel];
    m_audiodata[Phonon::AudioDataOutput::RightChannel] += data[Phonon::AudioDataOutput::RightChannel];
    int curSampleNumber = m_audiodata[Phonon::AudioDataOutput::LeftChannel].size();
    long l, r;
    int pos = 0;
    QVector<qint16> ldata, rdata;
    while(curSampleNumber >= m_samplesPerPixel) {
        l = r = 0;
        ldata = m_audiodata[Phonon::AudioDataOutput::LeftChannel].mid(pos, m_samplesPerPixel);
        rdata = m_audiodata[Phonon::AudioDataOutput::RightChannel].mid(pos, m_samplesPerPixel);
#if 0
        for(int i = 0; i < ldata.size(); i++){
            l += ldata[i];
            r += rdata[i];
        }
        //qDebug()<<l/ m_samplesPerPixel<<r/ m_samplesPerPixel;
        m_paintingData[Phonon::AudioDataOutput::LeftChannel].append(l / m_samplesPerPixel);
        m_paintingData[Phonon::AudioDataOutput::RightChannel].append(r / m_samplesPerPixel);
#else
        qSort(ldata);
        qSort(rdata);
        l = ldata[ldata.size()/2];
        r = rdata[rdata.size()/2];
        m_paintingData[Phonon::AudioDataOutput::LeftChannel].append(l);
        m_paintingData[Phonon::AudioDataOutput::RightChannel].append(r);
#endif
        curSampleNumber -= m_samplesPerPixel;
        pos += m_samplesPerPixel;
    }
    m_audiodata[Phonon::AudioDataOutput::LeftChannel].remove(0, pos);
    m_audiodata[Phonon::AudioDataOutput::RightChannel].remove(0, pos);
    update();
}

void MediaAudioWidget::paintEvent(QPaintEvent *event)
{
    //      qDebug()<<"In Paint Event";
    int w = size().width();
    int h = size().height();
    QPainter painter;
    painter.begin(this);
    painter.setBrush(Qt::white);
    //Draw A Background Rectangle
    painter.drawRect(0,0,w,h);

    int pixelPerSecond = m_sampleRate / m_samplesPerPixel;

    int wavMax = 0x7fff; // sample size 16 bit
    //Draw the wave form
    int dataSize = m_paintingData[Phonon::AudioDataOutput::LeftChannel].size();
    int viewDataBegin = qMax(0, dataSize - w);
    int viewDataEnd = viewDataBegin + qMin(w, dataSize);
    for(int x = viewDataBegin; x < viewDataEnd; x++){
        painter.drawLine(x % w, h / 4, x % w,
                h / 4 +
                m_paintingData[Phonon::AudioDataOutput::LeftChannel][x] * h / 2 / wavMax);
        painter.drawLine(x % w, h * 3 / 4, x % w,
                h * 3 / 4+
                m_paintingData[Phonon::AudioDataOutput::RightChannel][x] * h / 2 / wavMax);
    }
    painter.setPen(Qt::green);
    for (int x = 0; x < w; x += pixelPerSecond) {
        painter.drawLine(x, 0, x, h);
    }
    painter.drawLine(0,0,0,h);
    painter.drawLine(0,h*0.25,w,h*0.25);
    painter.drawLine(0,h*0.5,w,h*0.5);
    painter.drawLine(0,h*0.75,w,h*0.75);

    painter.end();
}


