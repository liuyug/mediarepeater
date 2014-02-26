#ifndef VIDEOWIDGET_H

#include <phonon/phonondefs.h>
#include <phonon/videowidget.h>

#include <QtGui>

class MediaVideoWidget: public Phonon::VideoWidget
{
    Q_OBJECT
    public:
        MediaVideoWidget(QWidget *parent);
        ~MediaVideoWidget();
        void setVideoImage(QString imagePath);
        void enterVideoMode();
        void enterImageMode();

        QSize sizeHint() const {
            return QSize(320, 240);
        }
    protected:
        void mouseDoubleClickEvent(QMouseEvent* event);
        void keyPressEvent(QKeyEvent* event);
        void resizeEvent(QResizeEvent* event);

    private:
        QLabel *_label;
        QPixmap _image;
        bool _imageOk;
};

// vim: sw=4 ts=4 tw=80
#endif
