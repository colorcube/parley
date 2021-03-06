/***************************************************************************

    Copyright 2006, 2007 Peter Hedlund <peter.hedlund@kdemail.net>
    Copyright 2007 Frederik Gladhorn <frederik.gladhorn@kdemail.net>

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef VOCABULARYDELEGATE_H
#define VOCABULARYDELEGATE_H

#include "readonlycontainermodel.h"
#include "../scripts/translator.h"

#include <QItemDelegate>
#include <QModelIndex>

class KEduVocDocument;

namespace Editor
{

class VocabularyDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    enum entryColumns {
        Translation = 0,
        Pronunciation,
        WordType,
        Synonym,
        Antonym,
        Example,
        Comment,
        Paraphrase,
//         Audio,
//         Image,
        EntryColumnsMAX
    };

    VocabularyDelegate(QObject *parent = 0);

    QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;

    void setEditorData(QWidget *editor, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const Q_DECL_OVERRIDE;
    void setTranslator(Translator * translator);

    static int columnType(int column);

    bool hasAudio(const QModelIndex &index) const;
    bool hasImage(const QModelIndex &index) const;
    QString audioUrl(const QModelIndex &index) const;
    QString imageUrl(const QModelIndex &index) const;
    QPolygon audioPolygon(const QStyleOptionViewItem &option) const;
    QPolygon imagePolygon(const QStyleOptionViewItem &option) const;

public slots:
    void setDocument(KEduVocDocument *doc);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index ) Q_DECL_OVERRIDE;

private:
    KEduVocDocument *m_doc;
    Translator * m_translator;

    /** Returns the translations of the word of the given index */
    QSet<QString> getTranslations(const QModelIndex & index) const;

    // for the word type combo
    class WordTypeBasicModel;
};

class VocabularyDelegate::WordTypeBasicModel : public ReadonlyContainerModel
{
public:
    explicit WordTypeBasicModel(QObject *parent = 0);
protected:
    KEduVocContainer * rootContainer() const Q_DECL_OVERRIDE;
};

}

#endif
