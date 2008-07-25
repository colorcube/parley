//
// C++ Interface: mixedlettersprompt
//
// Description:
//
//
// Author: David Capel <wot.narg@gmail.com>, (C) 2008
//
/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/


#ifndef MIXEDLETTERSPROMPT_H
#define MIXEDLETTERSPROMPT_H

#include <QWidget>
#include <QList>

class KSvgRenderer;
class QString;
class QGraphicsView;
class QWidget;
class QGraphicsScene;
class QGraphicsTextItem;

class MixedLettersPrompt : public QWidget
{
    Q_OBJECT

    public:
        MixedLettersPrompt(KSvgRenderer * renderer, QGraphicsView * view, const QString& elementId, QWidget * parent = 0);
        QStringList scramble(const QString& input);
    public slots:
        void slotSetText(const QString& solution);
        void slotAnswerChanged(const QString& answer);
    private:
        KSvgRenderer * m_renderer;
        QRectF m_backgroundRect;
        QGraphicsScene* m_scene;
        QGraphicsView* m_view;
        QList<QGraphicsTextItem*> m_letters;
        QString m_solution;
};

#endif