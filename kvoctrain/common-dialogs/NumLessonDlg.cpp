/***************************************************************************

    $Id$

                     input number of entries per lesson

    -----------------------------------------------------------------------

    begin                : Sat May 27 19:37:59 2000
    copyright            : (C) 1999-2001 Ewald Arnold
                           (C) 2001 The KDE-EDU team
    email                : kvoctrain@ewald-arnold.de

    -----------------------------------------------------------------------

    $Log$
    Revision 1.3  2001/10/13 11:45:29  coolo
    includemocs and other smaller cleanups. I tried to fix it, but as it's still
    qt2 I can't test :(

    Revision 1.2  2001/10/12 19:15:05  arnold
    switched dialog files to qt-designer

    Revision 1.1  2001/10/05 15:37:45  arnold
    import of version 0.7.0pre8 to kde-edu


 ***************************************************************************/

#include "NumLessonDlg.h"

#include <kapp.h>

#include <kv_resource.h>
#include <compat_2x.h>

#include <qlineedit.h>
#include <qpushbutton.h>

NumLessonDlg::NumLessonDlg
(
  int         num,
	QWidget    *parent,
	const char *name
)
	:
	NumLessonDlgForm( parent, name )
{
      input = num;
      connect( e_title, SIGNAL(textChanged(const QString&)), SLOT(slotInputChanged(const QString&)) );
      connect( e_title, SIGNAL(returnPressed()), SLOT(accept()) );
      connect( b_cancel, SIGNAL(clicked()), SLOT(reject()) );
      connect( b_ok, SIGNAL(clicked()), SLOT(accept()) );
      b_ok->setDefault(true);

      QString s;
      s.setNum (input);
      e_title->setText(s);
      setIcon (QPixmap (EA_KDEDATADIR("",  "kvoctrain/mini-kvoctrain.xpm" )));
      setCaption( kapp->makeStdCaption(i18n("Assign lessons")));
}


//NumLessonDlg::~NumLessonDlg()
//{
//}


void NumLessonDlg::slotInputChanged(const QString& s)
{
  input = QString(s).toInt();
}
#include "NumLessonDlg.moc"
