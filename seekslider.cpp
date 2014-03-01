
#include <phonon/mediaobject.h>

#include "seekslider.h"

#define SLIDERDRAG 0

MediaSeekSlider::MediaSeekSlider(QWidget *parent):
    QWidget(parent)
{
    init();
}

MediaSeekSlider::MediaSeekSlider(Phonon::MediaObject *mo, QWidget *parent):
    QWidget(parent)
{
    init();
    setMediaObject(mo);
}

void MediaSeekSlider::init()
{
    _margin = 5;
    _grooveHeight = 8;
    _handleRadius = 8;
    setMinimumSize(200, _handleRadius + 2 + _margin * 2);
    _media = NULL;
    _ticking = false;
    _aPos = _bPos = 0;
    _minimum = _maximum = _value = _position = 0;
    _tracking = false;
    _pressedControl = MediaSeekSlider::NONE;
    _mouseIn = 0;
    setOrientation(Qt::Horizontal);
    QSizePolicy sp(QSizePolicy::Expanding, QSizePolicy::Fixed, QSizePolicy::Slider);
    if (orientation() == Qt::Vertical)
        sp.transpose();
    setSizePolicy(sp);
    setMouseTracking(true);
    connect(this, SIGNAL(valueChanged(int)), SLOT(seek(int)));
    connect(this, SIGNAL(handleMoved(int)), SLOT(seek(int)));
}

void MediaSeekSlider::mouseMoveEvent(QMouseEvent *event)
{
    const QRect sliderRect = subControlRect(MediaSeekSlider::SLIDER);
    if (sliderRect.contains(event->pos())) {
        if (_pressedControl == MediaSeekSlider::HANDLE) {
            setPosition(rangeValueFromPixelPosition(pick(event->pos())));
            update();
        } else {
            int time = rangeValueFromPixelPosition(pick(event->pos()));
            QTime displayTime((time / 3600000) % 60, (time / 60000) % 60, (time / 1000) % 60);
            QToolTip::showText(event->globalPos(),
                    displayTime.toString("hh:mm:ss"),
                    this, QRect(event->pos(), QSize(5, 5)));
        }
    }
    QWidget::mouseMoveEvent(event);
}

void MediaSeekSlider::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        const QRect sliderRect = subControlRect(MediaSeekSlider::SLIDER);
        const QRect handleRect = subControlRect(MediaSeekSlider::HANDLE);
        if (sliderRect.contains(event->pos())) {
            event->accept();
            int pos = rangeValueFromPixelPosition(pick(event->pos()));
            setPosition(pos);
            update();
#if SLIDERDRAG
            if (handleRect.contains(event->pos())) {
                _pressedControl = MediaSeekSlider::HANDLE;
            }
#endif
        } else {
            QWidget::mousePressEvent(event);
        }
    } else {
        QWidget::mousePressEvent(event);
    }
}

void MediaSeekSlider::mouseReleaseEvent(QMouseEvent *event)
{
#if SLIDERDRAG
    _pressedControl = MediaSeekSlider::NONE;
#endif
    QWidget::mouseReleaseEvent(event);
}

void MediaSeekSlider::enterEvent(QEvent *event)
{
#if SLIDERDRAG
    _mouseIn = 1;
#endif
    QWidget::enterEvent(event);
}

void MediaSeekSlider::leaveEvent(QEvent *event)
{
#if SLIDERDRAG
    _mouseIn = 0;
#endif
    QWidget::leaveEvent(event);
}

QRect MediaSeekSlider::subControlRect(int sc) const
{
    int x = _margin + _handleRadius / 2, y = (height() - _grooveHeight) / 2;
    int w = width() - _margin * 2 - _handleRadius;
    int h = _grooveHeight;
    if (sc == MediaSeekSlider::SLIDER) {
    } else if (sc==MediaSeekSlider::GROOVE) {
    } else if (sc==MediaSeekSlider::GROOVE1) {
        w = pixelPositionFromRangeValue(position());
    } else if (sc==MediaSeekSlider::GROOVE2) {
    } else if (sc==MediaSeekSlider::GROOVEAB) {
        int a = pixelPositionFromRangeValue(aPosition());
        x += a;
        w = 2;
        if (bPosition() > 0) {
            w = w + pixelPositionFromRangeValue(bPosition()) - a;
        }
    } else if (sc==MediaSeekSlider::GROOVEA) {
        int a = pixelPositionFromRangeValue(aPosition());
        x += a;
        w = 2;
    } else if (sc==MediaSeekSlider::GROOVEB) {
        int b = pixelPositionFromRangeValue(bPosition());
        x += b;
        w = 2;
    } else if (sc==MediaSeekSlider::HANDLE) {
        x = x + pixelPositionFromRangeValue(position()) - _handleRadius + 1;
        y = height() / 2 - _handleRadius + 1;
        w = _handleRadius * 2;
        h = w;
    }
    return QRect(x, y, w, h);
}

void MediaSeekSlider::paintEvent(QPaintEvent *event)
{
    QRect subRect;
    QPainter painter(this);

    const QPalette & pal = palette();
    QColor grooveShadow(pal.shadow().color());
    QColor grooveMidlight(pal.midlight().color());
    QColor grooveLight(pal.light().color());
    QColor grooveHighlight(pal.highlight().color());
    QLinearGradient lineGradient;
    // draw slider bar
    subRect = subControlRect(MediaSeekSlider::GROOVE);
    painter.setPen(grooveMidlight);
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(subRect.adjusted(-1, -1, 0, 0));
    painter.setPen(grooveMidlight.lighter(130));
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(subRect.adjusted(0, 0, 1, 1));
    painter.setPen(grooveShadow.darker(150));
    lineGradient.setStart(subRect.x() + 1, subRect.y() + 1);
    lineGradient.setFinalStop(subRect.x() + 1, subRect.bottom());
    lineGradient.setColorAt(0.0, grooveShadow.darker(110));
    lineGradient.setColorAt(1.0, grooveShadow.lighter(110));
    painter.setBrush(QBrush(lineGradient));
    painter.drawRect(subRect);
    // draw AB bar
    if (_aPos > 0 && _bPos > 0) {
        QColor abColor(pal.linkVisited().color());
        subRect = subControlRect(MediaSeekSlider::GROOVEAB);
        painter.setPen(abColor.darker(140));
        lineGradient.setStart(subRect.x() + 1, subRect.y() + 1);
        lineGradient.setFinalStop(subRect.x() + 1, subRect.bottom());
        lineGradient.setColorAt(0.0, abColor.darker(110));
        lineGradient.setColorAt(1.0, abColor.lighter(110));
        painter.setBrush(QBrush(lineGradient));
        painter.drawRect(subRect);
    }
    // draw process bar
    subRect = subControlRect(MediaSeekSlider::GROOVE1);
    if (subRect.width() > 0) {
        painter.setPen(grooveHighlight.darker(140));
        lineGradient.setStart(subRect.x() + 1, subRect.y() + 1);
        lineGradient.setFinalStop(subRect.x() + 1, subRect.bottom());
        lineGradient.setColorAt(0.0, grooveHighlight.darker(110));
        lineGradient.setColorAt(1.0, grooveHighlight.lighter(110));
        painter.setBrush(QBrush(lineGradient));
        painter.drawRect(subRect);
    }
    // draw A and B point
    if (_aPos > 0) {
        QColor aColor(pal.linkVisited().color());
        subRect = subControlRect(MediaSeekSlider::GROOVEA);
        painter.setPen(aColor.darker(140));
        lineGradient.setStart(subRect.x() + 1, subRect.y() + 1);
        lineGradient.setFinalStop(subRect.x() + 1, subRect.bottom());
        lineGradient.setColorAt(0.0, aColor.darker(110));
        lineGradient.setColorAt(1.0, aColor.lighter(110));
        painter.setBrush(QBrush(lineGradient));
        painter.drawRect(subRect);
    }

    if (_bPos > 0) {
        QColor bColor(pal.linkVisited().color());
        subRect = subControlRect(MediaSeekSlider::GROOVEB);
        painter.setPen(bColor.darker(140));
        lineGradient.setStart(subRect.x() + 1, subRect.y() + 1);
        lineGradient.setFinalStop(subRect.x() + 1, subRect.bottom());
        lineGradient.setColorAt(0.0, bColor.darker(110));
        lineGradient.setColorAt(1.0, bColor.lighter(110));
        painter.setBrush(QBrush(lineGradient));
        painter.drawRect(subRect);
    }

#if SLIDERDRAG
    if (_mouseIn) {
        subRect = subControlRect(MediaSeekSlider::HANDLE);
        painter.setRenderHint(QPainter::Antialiasing);
        QColor handleColor;
        if (_pressedControl == MediaSeekSlider::HANDLE) {
            handleColor = grooveHighlight.darker(150);
        } else {
            handleColor = grooveShadow.darker(150);
        }
        painter.setPen(handleColor);
        painter.setBrush(grooveLight);
        painter.drawEllipse(subRect.center(), _handleRadius, _handleRadius);
        painter.setPen(Qt::NoPen);
        painter.setBrush(handleColor);
        painter.drawEllipse(subRect.center(), _handleRadius/3, _handleRadius/3);
    }
#endif
}

void MediaSeekSlider::setValue(int v)
{
    if (v < minimum())
        _value = minimum();
    else if (v > maximum())
        _value = maximum();
    else
        _value = v;
    emit valueChanged(_value);
    if (hasTracking()) {
        _position = _value;
        update();
    }
}

void MediaSeekSlider::setPosition(int pos)
{
    if (hasTracking())
    {
        if (pos < minimum())
            _position = minimum();
        else if (pos > maximum())
            _position = maximum();
        else
            _position = pos;
        emit handleMoved(pos);
        update();
    }
}

void MediaSeekSlider::setRange(int min, int max)
{
    _minimum = min;
    _maximum = max;
}

void MediaSeekSlider::setMediaObject(Phonon::MediaObject *mo)
{
    if(_media)
        disconnect(_media, 0, this, 0);
    _media = mo;
    if (_media) {
        connect(_media, SIGNAL(stateChanged(Phonon::State, Phonon::State)),
                SLOT(stateChanged(Phonon::State)));
        connect(_media, SIGNAL(totalTimeChanged(qint64)), SLOT(length(qint64)));
        connect(_media, SIGNAL(tick(qint64)), SLOT(tick(qint64)));
        connect(_media, SIGNAL(seekableChanged(bool)), SLOT(seekableChanged(bool)));
        connect(_media, SIGNAL(currentSourceChanged(const Phonon::MediaSource&)),
                SLOT(currentSourceChanged()));
        stateChanged(_media->state());
        seekableChanged(_media->isSeekable());
        length(_media->totalTime());
    } else {
        stateChanged(Phonon::StoppedState);
        seekableChanged(false);
    }
}

Phonon::MediaObject * MediaSeekSlider::mediaObject() const
{
    return _media;
}

void MediaSeekSlider::seek(int msec)
{
    if (!_ticking && _media) {
        _media->seek(msec);
    }
}

void MediaSeekSlider::tick(qint64 msec)
{
    _ticking = true;
    setValue(msec);
    _ticking = false;
}

void MediaSeekSlider::length(qint64 msec)
{
    _ticking = true;
    setRange(0, msec);
    _ticking = false;
}

void MediaSeekSlider::seekableChanged(bool isSeekable)
{
    setTracking(isSeekable);
    if (!isSeekable || !_media) {
        setEnabled(false);
    } else {
        switch (_media->state()) {
            case Phonon::PlayingState:
                if (_media->tickInterval() == 0) {
                    _media->setTickInterval(350);
                }
            case Phonon::BufferingState:
            case Phonon::PausedState:
                setEnabled(true);
                break;
            case Phonon::StoppedState:
            case Phonon::LoadingState:
            case Phonon::ErrorState:
                qDebug()<<"State Error"<<__FUNCTION__<<mediaObject()->errorString();
                setEnabled(false);
                _ticking = true;
                setValue(0);
                _ticking = false;
                break;
        }
    }
}

void MediaSeekSlider::currentSourceChanged()
{
    QMouseEvent event(QEvent::MouseButtonRelease,
            QPoint(), Qt::LeftButton, 0, 0);
    QApplication::sendEvent(this, &event);
}

void MediaSeekSlider::stateChanged(Phonon::State newstate)
{
    if (!_media || !_media->isSeekable())
    {
        setEnabled(false);
        return;
    }
    switch (newstate)
    {
        case Phonon::PlayingState:
            if (_media->tickInterval() == 0)
            {
                _media->setTickInterval(350);
            }
        case Phonon::BufferingState:
        case Phonon::PausedState:
            setEnabled(true);
            break;
        case Phonon::StoppedState:
        case Phonon::LoadingState:
        case Phonon::ErrorState:
            qDebug()<<"State Error"<<__FUNCTION__<<mediaObject()->errorString();
            setEnabled(false);
            _ticking = true;
            setValue(0);
            _ticking = false;
            break;
    }
}

void MediaSeekSlider::setAPosition()
{
    _aPos = value();
}

void MediaSeekSlider::setBPosition()
{
    _bPos = value();
    if(_bPos < _aPos) {
        int temp = _aPos;
        _aPos = _bPos;
        _bPos = temp;
    }
}

inline int MediaSeekSlider::pick(const QPoint &pt) const
{
    return orientation() == Qt::Horizontal ? pt.x() : pt.y();
}

int MediaSeekSlider::pixelPositionFromRangeValue(int value) const
{
    QRect subRect = subControlRect(MediaSeekSlider::SLIDER);
    int min = minimum();
    int max = maximum();
    int span = subRect.width();

    uint range = max - min;
    uint p = value - min;

    if (span <= 0 || value < min || max <= min)
        return 0;
    if (value > max)
        return span;

    if (range > (uint)INT_MAX/4096) {
        double dpos = (double(p))/(double(range)/span);
        return int(dpos);
    } else if (range > (uint)span) {
        return (2 * p * span + range) / (2*range);
    } else {
        uint div = span / range;
        uint mod = span % range;
        return p * div + (2 * p * mod + range) / (2 * range);
    }
}

int MediaSeekSlider::rangeValueFromPixelPosition(int pos) const
{
    QRect subRect = subControlRect(MediaSeekSlider::SLIDER);
    int min = minimum();
    int max = maximum();
    int span = subRect.width();
    int p = pos - subRect.x() - 1;

    if (span <= 0 || p <= 0)
        return min;
    if (p >= span)
        return max;

    uint range = maximum() - minimum();

    if ((uint)span > range)
    {
        int tmp = (2 * p * range + span) / (2 * span);
        return tmp + min;
    } else {
        uint div = range / span;
        uint mod = range % span;
        int tmp = p * div + (2 * p * mod + span) / (2 * span);
        return tmp + min;
    }
}

// vim: sw=4 ts=4 tw=80
