/***************************************************************************

    $Id$

                            input several descriptions

    -----------------------------------------------------------------------

    begin                : Thu Dec 9 1999
    copyright            : (C) 1999-2001 Ewald Arnold
                           (C) 2001 The KDE-EDU team
    email                : kvoctrain@ewald-arnold.de

    -----------------------------------------------------------------------

    $Log$
    Revision 1.3  2001/10/13 11:45:29  coolo
    includemocs and other smaller cleanups. I tried to fix it, but as it's still
    qt2 I can't test :(

    Revision 1.2  2001/10/12 19:15:40  arnold
    switched dialog files to qt-designer

    Revision 1.1  2001/10/05 15:37:45  arnold
    import of version 0.7.0pre8 to kde-edu


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "LessonInputDlg.h"

#include <qpixmap.h>
#include <qpushbutton.h>
#include <qlabel.h>

#include <klocale.h>
#include <kapp.h>

#include <kv_resource.h>

#include <compat_2x.h>

LessonInputDlg::LessonInputDlg
(
        QString  _input,
        QString  _descr,
        QString  _label,
	QWidget* parent,
	const char* name
)
	:
	LessonInputDlgForm( parent, name, true )
{
	connect( e_title, SIGNAL(returnPressed()), SLOT(accept()) );
	connect( b_cancel, SIGNAL(clicked()), SLOT(reject()) );
	connect( b_ok, SIGNAL(clicked()), SLOT(accept()) );
        b_ok->setDefault(true);

        if (_descr.isEmpty() )
          _descr = i18n("Input lesson description");

        title_label->setText (_label);
        setCaption(kapp->makeStdCaption(_descr));

        input = _input;
        e_title->setText (input);
	connect( e_title, SIGNAL(textChanged(const QString&)), SLOT(slotInputChanged(const QString&)) );
   	e_title->setFocus();
	e_title->selectAll();
        setIcon (QPixmap (EA_KDEDATADIR("",  "kvoctrain/mini-kvoctrain.xpm" )));
}


void LessonInputDlg::slotInputChanged(const QString& s)
{
  input = s;
}
#include "LessonInputDlg.moc"
