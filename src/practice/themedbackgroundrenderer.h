/***************************************************************************
    Copyright 2010 Daniel Laidig <laidig@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef PRACTICE_THEMEDBACKGROUNDRENDERER_H
#define PRACTICE_THEMEDBACKGROUNDRENDERER_H

#include <QObject>
#include <QFuture>
#include <QFutureWatcher>

#include <ksvgrenderer.h>

class QMargins;

namespace Practice {

class ThemedBackgroundRenderer : public QObject
{
    Q_OBJECT

public:
    enum ScaleBase {
        NoScale,
        Horizontal,
        Vertical,
        Rect
    };

    enum Edge {
        Top,
        Bottom,
        Left,
        Right,
        Center
    };

    enum Align {
        Corner,
        LeftTop, // left or top (depending on orientation of the edge)
        Centered,
        RightBottom // right or bottom (depending on orientation of the edge)
    };

    ThemedBackgroundRenderer(QObject* parent = 0);
    ~ThemedBackgroundRenderer() {}

    void setSvgFilename(const QString& filename);
    QPixmap getPixmapForId(const QString& id);
    QMargins contentMargins();
    
public Q_SLOTS:
    void setSize(const QSize& size);
    void clearRects();
    void addRect(const QString& name, const QRect& rect);
    void updateBackground();

    void renderingFinished();

signals:
    void backgroundChanged(QPixmap pixmap);
   
private:
    QImage renderBackground();
    void renderRect(const QString& name, const QRect& rect, QPainter *p);
    void renderItem(const QString& id, const QRect& rect, QPainter *p, ScaleBase scaleBase, Qt::AspectRatioMode aspectRatio, Edge edge, Align align, bool inside);
    QRect scaleRect(QRect itemRect, const QRect& baseRect, ScaleBase scaleBase, Qt::AspectRatioMode aspectRatio);
    QRect alignRect(QRect itemRect, const QRect& baseRect, Edge edge, Align align, bool inside);

    QFuture<QImage> m_future;
    QFutureWatcher<QImage> m_watcher;
    KSvgRenderer m_renderer;
    QList<QPair<QString, QRect> > m_rects;
    QSize m_size;
    bool m_queuedRequest;
};

}

#endif // PRACTICE_THEMEDBACKGROUNDRENDERER_H