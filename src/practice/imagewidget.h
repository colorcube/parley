/***************************************************************************
    Copyright 2009 Daniel Laidig <d.laidig@gmx.de>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICE_IMAGEWIDGET_H
#define PRACTICE_IMAGEWIDGET_H

#include <QtGui/QWidget>

class QTimeLine;

namespace Practice {

class ImageWidget : public QWidget
{
    Q_OBJECT

public:
    ImageWidget(QWidget *parent = 0);

    void setPixmap(const QPixmap& pixmap);
    void setScalingEnabled(bool scaling, bool onlyDownscaling = true);

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

signals:
    void sizeChanged();

private Q_SLOTS:
    void scalePixmap(bool smooth = true);
    void animationFinished();

private:
    bool m_scaling;
    bool m_onlyDownscaling;
    QPixmap m_originalPixmap;
    QPixmap m_scaledPixmap;
    QPixmap m_scaledBackupPixmap;
    QPixmap m_animationPixmap;
    bool m_scaledPixmapOutOfDate;
    /// Timer used to delay the smooth scaling
    QTimer *m_scaleTimer;
    QTimeLine *m_animation;
};

}

#endif // PRACTICE_IMAGEWIDGET_H
