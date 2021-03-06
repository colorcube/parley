/***************************************************************************
    Copyright 2015 Inge Wallin
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


// Own
#include "utils.h"

// Qt
#include <QPainter>
#include <QRect>

// KEduVocDocument library
#include <keduvocdocument.h>
#include <keduvocexpression.h>

// Parley
#include "prefs.h"

// ----------------------------------------------------------------
//                         class WordCount


WordCount::WordCount()
{
    clear();
}


void WordCount::clear()
{
    for (int i = 0; i <= KV_MAX_GRADE; ++i) {
        grades[i] = 0;
        pregrades[i] = 0;
    }
    invalid = 0;

    initialWords = 0;
    totalWords = 0;
}

int WordCount::percentageCompleted() const
{
    // To calculate the percentage done we add:
    //  * 1..KV_MAX_GRADE points for words in the initial phase (grade = 0, pregrade > 0)
    //  * KV_MAX_GRADE * (1..KV_MAX_GRADE) points for words in the long-term phase (grade>0)
    // So the maximum number of points is KV_MAX_GRADE^2 per word.
    // 
    // In the final calculation, we exclude all invalid words from the percentage.
    int points = 0;
    for (int i = 0; i < KV_MAX_GRADE + 1; ++i) {
        points += pregrades[i] * i;
        points += grades[i] * KV_MAX_GRADE * i;
    }

    if (totalWords - invalid == 0) {
        // Don't divide by 0.
        return 0;
    }
    else {
        return 100 * points / ((totalWords - invalid) * KV_MAX_GRADE * KV_MAX_GRADE);
    }
}


void WordCount::fillFromContainer(KEduVocContainer &container, int translationIndex,
                                  KEduVocContainer::EnumEntriesRecursive recursive)
{
    clear();

    foreach (KEduVocExpression *entry, container.entries(recursive)) {
        KEduVocTranslation &translation(*entry->translation(translationIndex));

        ++totalWords;
        if (translation.isEmpty()) {
            ++invalid;
        } else if (translation.preGrade() > 0) {
            // Initial phase (we assume correctness, i.e. if pregrade>0 then grade = 0)
            ++initialWords;
            ++pregrades[translation.preGrade()];
        } else {
            // Long term or unpracticed
            ++grades[translation.grade()];
        }
    }
}


// ----------------------------------------------------------------
//                         class confidenceColors


ConfidenceColors::ConfidenceColors(ColorScheme colorScheme)
{
    initColors(colorScheme);
}


void ConfidenceColors::initColors(ColorScheme colorScheme)
{
    switch (colorScheme) {
    case MultiColorScheme:
    default: // Not default at the last line.  Hope this works...

        longTermColors[0] = QColor(25,38,41);
        //longTermColors[1] = QColor(Qt::yellow);
        longTermColors[1] = QColor(25,38,41,64);
        longTermColors[2] = QColor(237,21,21);
        longTermColors[3] = QColor(246,116,0);
        longTermColors[4] = QColor(201,206,59);
        longTermColors[5] = QColor(28,220,154);
        longTermColors[6] = QColor(17,209,22);
        longTermColors[7] = QColor(61,174,253);

        initialTermColor = QColor(25,38,41,64); // Find something else

        invalidColor = QColor(Qt::red);
        break;

    case ProgressiveColorScheme:
        {
            static const int AlphaMax = 255;
            static const int AlphaStep = ((AlphaMax - 10) / KV_MAX_GRADE);

            QColor color;

            // Confidence 1..max
            for (int grade = 1; grade <= KV_MAX_GRADE; ++grade) {
                color = Prefs::gradeColor();
                color.setAlpha(AlphaMax - (KV_MAX_GRADE - grade) * AlphaStep);

                longTermColors[grade] = color;
            }

            // Unpracticed (confidence 0)
            color = QColor("#FFFFFF");
            color.setAlpha(AlphaMax);
            longTermColors[0] = color;

            // Use one color for all initial phase values
            color = Prefs::preGradeColor();
            color.setAlpha(AlphaMax);
            initialTermColor = color;

            // Invalid
            invalidColor = Prefs::invalidUnitColor();

            break;
        }
    }

    // These two are placeholders for the wordcloud background color.
    frontEndColors[0] = QColor(255,221,217);
    frontEndColors[1] = QColor(238,232,213);
}


// ----------------------------------------------------------------
//                         Various utility functions


void paintColorBar(QPainter &painter, const QRect &rect,
                   const WordCount &wordCount, const ConfidenceColors &colors)
{
    // The outline of the total bar.
    QRect roundedRect(rect);
    roundedRect.adjust(1, 1, -1, -1);
    QPainterPath roundedPath;
    roundedPath.addRoundedRect(roundedRect, 2.0, 2.0);

    int xPosition = 0;

    // >0: grade, 0: initial, -1: untrained, -2: invalid
    for (int i = KV_MAX_GRADE; i >= -2; --i) {
        qreal fraction;
        QColor color;

        // Get the fraction and the color
        if (i > 0) {
            // long term
            fraction = qreal(wordCount.grades[i]) / qreal(wordCount.totalWords);
            color = colors.longTermColors[i];
        } else if (i == 0) {
            // initial term
            fraction = qreal(wordCount.initialWords) / qreal(wordCount.totalWords);
            color = colors.initialTermColor;
        } else if (i == -1) {
            // untrained (stored in longterm[0])
            fraction = qreal(wordCount.grades[0]) / qreal(wordCount.totalWords);
            color = colors.longTermColors[0];
        } else {
            fraction = qreal(wordCount.invalid) / qreal(wordCount.totalWords);
            color = colors.invalidColor;
        }

        // Create a rect from the current fraction and 
        // adjust the rect to the outer limits.
        int barElementWidth = fraction * rect.width();
        //int barElementWidth = fraction * roundedRect.width(); <-- Use this instead?
        QRectF barElement(rect.x() + xPosition, rect.y(), barElementWidth, rect.height());
        QPainterPath barElementPath;
        barElementPath.addRect(barElement);

        // Intersect the QPainterPath of inner rectangle with outer,
        // so that the inner rectangle takes the shape of the outer rounded rectangle.
        QPainterPath barElementIntersectedPath = roundedPath.intersected(barElementPath);
        xPosition += barElementWidth;

        // Paint!
        painter.setBrush(QBrush(color));
        painter.drawPath(barElementIntersectedPath);
    }
}
