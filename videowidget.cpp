
#include "videowidget.h"

MediaVideoWidget::MediaVideoWidget(QWidget *parent) :
    Phonon::VideoWidget(parent)
{
    _label = new QLabel(this);
    _label->setAutoFillBackground(true);
    _label->setBackgroundRole(QPalette::WindowText);
    _label->setForegroundRole(QPalette::Window);
    _label->setScaledContents(true);
    _label->setAlignment(Qt::AlignCenter);
    _image.fill(QColor(Qt::black));
    _imageOk = false;
    enterImageMode();
}

MediaVideoWidget::~MediaVideoWidget() 
{
}

void MediaVideoWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    if(!this->isFullScreen())
        this->enterFullScreen();
    else
        this->setFullScreen(false);
}

void MediaVideoWidget::keyPressEvent(QKeyEvent* event)
{
    if(event->key() == Qt::Key_Escape)
    {
        if(!this->isFullScreen())
            this->enterFullScreen();
        else
            this->setFullScreen(false);
    }
}

void MediaVideoWidget::setVideoImage(QString imagePath)
{
    _imageOk = _image.load(imagePath);
    qDebug()<<"Loading video image"<<(_imageOk?"successed":"failed")<<imagePath;
}

void MediaVideoWidget::enterImageMode()
{
    if(_imageOk) {
        _label->setPixmap(_image);
    } else {
        _label->setText(tr("Audio"));
    }
    if(!_label->isVisible())
        _label->show();
    update();
}

void MediaVideoWidget::enterVideoMode()
{
    _label->hide();
}

void MediaVideoWidget::resizeEvent(QResizeEvent* event)
{
    Phonon::VideoWidget::resizeEvent(event);
    QRect rect = this->geometry();
    rect.moveTo(0, 0);
    _label->setGeometry(rect);
    QFont font = _label->font();
    font.setPixelSize(rect.height() / 2);
    _label->setFont(font);
    update();
}

// vim: sw=4 ts=4 tw=80
