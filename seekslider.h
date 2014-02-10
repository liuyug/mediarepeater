#ifndef SEEKSLIDER_H
#define SEEKSLIDER_H

#include <phonon/phonondefs.h>
#include <phonon/mediaobject.h>
#include <QtGui>


class MediaSeekSlider: public QWidget
{
    Q_OBJECT
    public:
        MediaSeekSlider(QWidget *parent);
        MediaSeekSlider(Phonon::MediaObject * mo, QWidget *parent);
        virtual ~MediaSeekSlider() {};
        Phonon::MediaObject * mediaObject() const; 
        int aPosition() const { return _aPos; };
        int bPosition() const { return _bPos; };
        int value() const { return _value; };
        int minimum() const { return _minimum; };
        int maximum() const { return _maximum; };
        bool hasTracking() const { return _tracking; };
        Qt::Orientation orientation() const { return _orientation; };
        void setRange(int min, int max);
        int position() const { return _position; };
        void setPosition(int pos);

        enum {
            NONE = 0,
            HANDLE,
            GROOVE,
            GROOVE1,
            GROOVE2,
            GROOVEAB,
            SLIDER, // for thumb position
        };

    public Q_SLOTS:
        void setValue(int v);
        void setMinimum(int v) { _minimum = v; };
        void setMaximum(int v) { _maximum = v; };
        void setTracking(bool v) { _tracking = v; };
        void setOrientation(Qt::Orientation o) { _orientation = o; };
        void seek(int msec);
        void tick(qint64 msec);
        void length(qint64 msec);
        void seekableChanged(bool isSeekable);
        void currentSourceChanged();
        void stateChanged(Phonon::State newstate);
        void setMediaObject(Phonon::MediaObject *mo);
        void setAPosition();
        void setBPosition();
        void clearABPosition() { _aPos = _bPos = 0; };

    signals:
            void valueChanged(int v);
            void handleMoved(int v);
    protected:
        void paintEvent(QPaintEvent *event);
        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);
        virtual void enterEvent(QEvent *event);
        virtual void leaveEvent(QEvent *event);

    private:
        // from QSlider
        Qt::Orientation _orientation;
        int _minimum;
        int _maximum;
        int _value;
        int _position;
        bool _tracking;
        // from phonon
        Phonon::MediaObject * _media;
        bool _ticking;  // disable media seek
        int _aPos, _bPos;
        inline int pick(const QPoint &pt) const;
        int pixelPositionFromRangeValue(int pos) const;
        int rangeValueFromPixelPosition(int pos) const;
        // subcontrol
        void init();
        int _margin;
        int _grooveHeight;
        int _handleRadius;
        int _pressedControl;
        int _mouseIn;
        QRect subControlRect(int sc) const;
};

// vim: sw=4 ts=4 tw=80
#endif
