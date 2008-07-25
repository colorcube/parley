//
// C++ Interface: articlemcinput
//
// Description:
//
//
// Author: David Capel <wot.narg@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef ARTICLEMCINPUT_H
#define ARTICLEMCINPUT_H

#include "mcinput.h"

class PracticeEntry;

class ArticleMCInput : public MCInput
{
    Q_OBJECT
     public:
        ArticleMCInput(KSvgRenderer * renderer, QGraphicsView * view, const QString& elementId, class KEduVocDocument * doc, QWidget * parent = 0);

    public slots:
        void slotSetAnswers(PracticeEntry*);
        void slotEmitAnswer();
    private:
        KEduVocDocument* m_doc;
};

#endif
