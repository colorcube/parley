/***************************************************************************

    $Id$

                         maintain a kvoctrain document

    -----------------------------------------------------------------------

    begin                : Thu Mar 11 20:50:53 MET 1999
                                           
    copyright            : (C) 1999-2001 Ewald Arnold
                           (C) 2001 The KDE-EDU team
                         
    email                : kvoctrain@ewald-arnold.de                                    

    -----------------------------------------------------------------------

    $Log$
    Revision 1.1  2001/10/05 15:42:01  arnold
    import of version 0.7.0pre8 to kde-edu


 ***************************************************************************

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef KVOCTRAINDOC_H
#define KVOCTRAINDOC_H 

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <qobject.h>

#include "kvoctrainexpr.h"

#include "kvt-xml/XmlReader.h"
#include "kvt-xml/XmlWriter.h"

#define KVD_ZERO_TIME  934329599   // 1999-08-10 23:59:59, never change
#define KVD_VERS_PREFIX " v"       // kvoctrain v0.1.0

/** XML tags and attribute names */

#define KV_DOCTYPE     "kvtml"     // doctype
#define KV_TITLE       "title"     // doc title
#define KV_AUTHOR      "author"    // doc author
#define KV_LICENSE     "license"   // doc license
#define KV_DOC_REM     "remark"    // doc remark
#define KV_LINES       "lines"     // entries
#define KV_GENERATOR   "generator" // who generated the doc
#define KV_COLS        "cols"      // columns
#define KV_ENCODING    "encoding"  // document encoding

#define KV_EXPR        "e"         // entry for one expression
#define KV_ORG         "o"         // original expression in specified language
#define KV_TRANS       "t"         // translated expression in specified language
#define KV_LANG        "l"         // language: en, de, it, fr ...
#define KV_QUERY       "q"         // query: org or translation
#define KV_O           "o"         // org
#define KV_T           "t"         // translation
#define KV_GRADE       "g"         // grade of knowledge: 0=well known, x=not known for x times
#define KV_LESS_MEMBER "m"         // member of lesson 1 .. x
#define KV_COUNT       "c"         // number of times queried
#define KV_SIZEHINT    "width"     // recommended column width
#define KV_CHARSET     "charset"   // recommended charset
#define KV_BAD         "b"         // number of times failed
#define KV_DATE        "d"         // last query date
#define KV_DATE2       "w"         // last query date, compressed format
#define KV_REMARK      "r"         // remark for this entry
#define KV_FAUX_AMI_F  "ff"        // false friend of this entry from org
#define KV_FAUX_AMI_T  "tf"        // false friend of this entry to org
#define KV_SYNONYM     "y"         // synonym (same meaning) of expr
#define KV_ANTONYM     "a"         // antonym (oppositite) of expr
#define KV_PRONUNCE    "p"         // how to pronunce this expression
#define KV_SELECTED    "s"         // entry selected
#define KV_EXPRTYPE    "t"         // type of expression
#define KV_EXAMPLE     "x"         // example string with word
#define KV_USAGE       "u"         // usage label
#define KV_PARAPHRASE  "h"         // paraphrase for expression

/*
 <type>
  <desc no="1">My type 1</desc>
  <desc no="2">My type 2</desc>
 </type>
*/

#define KV_TYPE_GRP    "type"      // type descriptor group
#define KV_TYPE_DESC   "desc"      // type descriptor
#define KV_TYPE_NO     "no"        // type descriptor number

/*
 <usage>
  <desc no="1">My usage 1</desc>
  <desc no="2">My usage 2</desc>
 </type>
*/

#define KV_USAGE_GRP    "usage"     // usage descriptor group
#define KV_USAGE_DESC   "desc"      // usage descriptor
#define KV_USAGE_NO     "no"        // usage descriptor number

/*
 <lesson width="138">
  <desc no="1">Lesson #1</desc>
  <desc no="2" query="1">Lesson #2</desc>
 </lesson>
*/

#define KV_LESS_GRP    "lesson"    // lesson descriptor group
#define KV_LESS_CURR   "current"   // is current lesson
#define KV_LESS_DESC   "desc"      // lesson descriptor
#define KV_LESS_QUERY  "query"     // lesson contained in query
#define KV_LESS_NO     "no"        // lesson descriptor number

/*
 <tense>
  <desc no="1">user tense #1</desc>
  <desc no="2">user tense #2</desc>
 </tense>
*/

#define KV_TENSE_GRP    "tense"     // tense descriptor group
#define KV_TENSE_DESC   "desc"      // tense descriptor
#define KV_TENSE_NO     "no"        // tense descriptor number

/*
 <options>
  <sort on="1"/>
 </options>
*/

#define KV_OPTION_GRP  "options"   // internal options group
#define KV_OPT_SORT    "sort"      // allow sorting
#define KV_BOOL_FLAG   "on"        // general boolean flag

/*
 <article>
  <e l="de">        lang determines also lang order in entries !!
   <fi>eine</fi>    which must NOT differ
   <fd>die</fd>
   <mi>ein</mi>
   <md>der</md>
   <ni>ein</ni>
   <nd>das</nd>
  </e>
 </article>
*/

#define KV_ARTICLE_GRP "article"   // article descriptor group
#define KV_ART_ENTRY   "e"         // article entry
#define KV_ART_FD      "fd"        // female definite
#define KV_ART_MD      "md"        // male definite
#define KV_ART_ND      "nd"        // natural definite
#define KV_ART_FI      "fi"        // female indefinite
#define KV_ART_MI      "mi"        // male indefinite
#define KV_ART_NI      "ni"        // natural indefinite

/*
 <comparison>
   <l1>good</l1>
   <l2>better</l2>
   <l3>best</l3>
 </comparison>
*/

#define KV_COMPARISON_GRP "comparison"   // comparison descriptor group
#define KV_COMP_L1        "l1"           // base form
#define KV_COMP_L2        "l2"           // next form
#define KV_COMP_L3        "l3"           // last form

/*
 <conjugation>        used in header for definiton of "prefix"
  <e l="de">          lang determines also lang order in entries !!
   <s1>I</s1>         which must NOT differ in subsequent <e>-tags
   <s2>you<2>
   <s3f>he</s3f>
   <s3m>she</s3m>
   <s3n>it</s3n>
   <p1>we</p1>
   <p2>you</p2>
   <p3f>they</p3f>
   <p3m>they</p3m>
   <p3n>they</p3n>
  </e>
 </conjugation>

 <conjugation>        and in entry for definition of tenses of (irreg.) verbs
  <t n="sipa">       
   <s1>go</s1>
   <s2>go</s2>
   <s3f>goes</s3f>
   <s3m>goes</s3m>
   <s3n>goes</s3n>
   <p1>go</p1>
   <p2>go</p2>
   <p3f>go</p3f>
   <p3m>go</p3m>
   <p3n>go</p3n>
  </t>
 </conjugation>
*/

#define KV_CONJUG_GRP  "conjugation" // conjugation descriptor group
#define KV_CON_ENTRY   "e"           // conjugation entry (header)
#define KV_CON_TYPE    "t"           // conjugation type  (voc entries)
#define KV_CON_NAME    "n"           // conjugation type name (voc entries)
#define KV_CON_P1S     "s1"          // 1. person singular
#define KV_CON_P2S     "s2"          // 2. person singular
#define KV_CON_P3SF    "s3f"         // 3. person singular female
#define KV_CON_P3SM    "s3m"         // 3. person singular male
#define KV_CON_P3SN    "s3n"         // 3. person singular natural
#define KV_CON_P1P     "p1"          // 1. person plural
#define KV_CON_P2P     "p2"          // 2. person plural
#define KV_CON_P3PF    "p3f"         // 3. person plural female
#define KV_CON_P3PM    "p3m"         // 3. person plural male
#define KV_CON_P3PN    "p3n"         // 3. person plural natural
#define KV_CONJ_COMMON "common"      // female contains common for all three

#define LEX_IDENT_50   "Vocabulary Trainer V5.0"

enum KV_Encoding {
/*
                   KE_ANY,
                   KE_8859_1,
                   KE_8859_2,
                   KE_8859_3,
                   KE_8859_4,
                   KE_8859_5,
                   KE_8859_6,
                   KE_8859_7,
                   KE_8859_8,
                   KE_8859_9,
                   KE_8859_15,
                   KE_KOI8_R,
                   KE_Locale,
*/
                   KE_8BIT,
                   KE_UTF8 };

#define KES_ANY         ""
#define KES_8859_1      "ISO 8859-1"
#define KES_8859_2      "ISO 8859-2"
#define KES_8859_3      "ISO 8859-3"
#define KES_8859_4      "ISO 8859-4"
#define KES_8859_5      "ISO 8859-5"
#define KES_8859_6      "ISO 8859-6"
#define KES_8859_7      "ISO 8859-7"
#define KES_8859_8      "ISO 8859-8"
#define KES_8859_9      "ISO 8859-9"
#define KES_KOI8_R      "KIO8-R"
//#define KES_KOI8_1      "KIO8-1"

#define KES_8BIT        "8Bit"
#define KES_UTF8        "UTF-8"

/*  from qt2.1 qtfont_x11.cpp

    { "ISO 8859-1", QFont::ISO_8859_1 },
    { "ISO 8859-2", QFont::ISO_8859_2 },
    { "ISO 8859-3", QFont::ISO_8859_3 },
    { "ISO 8859-4", QFont::ISO_8859_4 },
    { "ISO 8859-5", QFont::ISO_8859_5 },
    { "ISO 8859-6", QFont::ISO_8859_6 },
    { "ISO 8859-7", QFont::ISO_8859_7 },
    { "ISO 8859-8-I", QFont::ISO_8859_8 },
    { "ISO 8859-9", QFont::ISO_8859_9 },
    { "ISO 8859-10", QFont::ISO_8859_10 },
    { "ISO 8859-11", QFont::ISO_8859_11 },
    { "ISO 8859-12", QFont::ISO_8859_12 },
    { "ISO 8859-13", QFont::ISO_8859_13 },
    { "ISO 8859-14", QFont::ISO_8859_14 },
    { "ISO 8859-15", QFont::ISO_8859_15 },
    { "KOI8-R", QFont::KOI8R },
    { "eucJP", QFont::Set_Ja },
    { "SJIS", QFont::Set_Ja },
    { "JIS7", QFont::Set_Ja },
    { "eucKR", QFont::Set_Ko },
    { "TACTIS", QFont::Set_Th_TH },
    { "GBK", QFont::Set_GBK },
    { "zh_CN.GBK", QFont::Set_GBK },
    { "eucCN", QFont::Set_Zh },
    { "eucTW", QFont::Set_Zh_TW },
    { "zh_TW.Big5", QFont::Set_Big5 },
    { "Big5", QFont::Set_Big5 },
*/

# define KES_8859_10      "ISO 8859-10"  // additional charsets for font selection in QT2.x
# define KES_8859_11      "ISO 8859-11"
# define KES_8859_12      "ISO 8859-12"
# define KES_8859_13      "ISO 8859-13"
# define KES_8859_14      "ISO 8859-14"
# define KES_8859_15      "ISO 8859-15"

# define KES_SET_JA       "jisx0208.1983-0"
//# define KES_JA_eucJP     "eucJP"
//# define KES_JA_SJIS      "SJIS"
//# define KES_JA_JIS7      "JIS7"

# define KES_SET_KO       "ksc5601.1987-0"
//# define KES_KO_eucKR     "eucKR"

# define KES_SET_TH_TH    "Set_Th_TH"
//# define KES_TH_TH_TACTIS "TACTIS"

# define KES_SET_ZH       "Set_Zh"
# define KES_SET_ZH_TW    "Set_Zh_TW"
//# define KES_ZH_eucCN     "eucCN"
//# define KES_ZH_eucTW     "eucTW"

//# define KES_GBK_0        "GBK"
//# define KES_GBK_zh_CN    "zh_CN.GBK"

# define KES_UNICODE      "Unicode"

# define KES_SET_BIG5     "Set_Big5"
//# define KES_BIG5         "Big5"
//# define KES_BIG5_zh_TW   "zh_TW.Big5"

class QTextStream;
class QStringList;

/*************************************************************
  * This class contains the expressions of your vocabulary
  ************************************************************/

class kvoctrainDoc : public QObject
{
  Q_OBJECT
 public:

  enum FileType { kvd_none, automatic,
                  kvtml,
                  kvtbin,
                  vt_lex, vt_vcb, csv /*, kvoclearn, qvocab*/ };

  /** Constructor for the fileclass of the application
   *
   * @param obj              calling object
   * @param filename         name of file to parse
   */
  kvoctrainDoc(QObject* obj, QString name,
               const QString separator = "\t", QStringList *lang_order = 0);

  /** Destructor for the fileclass of the application */
  ~kvoctrainDoc();

  /** indicates that doc is (not) modified
   *
   * @param dirty   new state
   */
  inline void setModified (bool _dirty = true) { emit docModified(dirty = _dirty); }

  /** appends another entry at the end
   *
   * @param expr            expression to append
   */
  inline void appendEntry (kvoctrainExpr *expr)
    { vocabulary.push_back (*expr); dirty = true; }

  /** removes entry from doc
   *
   * @param index           index of entry
   */
  void removeEntry (int index);

  /** sorts vocabulary alphabetically
   *
   * @param index            index expression
   */
  void sort (int index);

  /** removes equal entries (orig + all translations)
   *
   * @result                 number of removed entries
   */
  int cleanUp();

  /** sorts vocabulary by lesson indices
   */
  bool sortByLesson_index ();

  /** sorts vocabulary by lesson name
   */
  bool sortByLesson_alpha ();

  /** enables sorting
   */
  inline void allowSorting(bool allow) { sort_allowed = allow; }

  /** enables sorting
   */
  inline bool isAllowedSorting() { return sort_allowed; }

  /** returns the modification state of the doc */
  inline bool isModified () const { return dirty; }

  /** returns originalīs identifier
   */
  QString getOriginalIdent () const;

  /** set originalīs identifier
   */
  void setOriginalIdent (const QString id);

  /** returns identifier of translation x
   *
   * @param index            number of translation 1..x
   * @result                 ident string: de=german, en=englisch, ..
   */
  QString getIdent (int index) const;

  /** sets identifier of translation
   *
   * @param index            number of translation 1..x
   * @param lang             ident string: de=german, en=englisch, ..
   */
  void setIdent (int index, const QString lang);

  /** removes identifier an the according translation in all entries
   *
   * @param index            number of translation 1..x
   */
  void removeIdent (int index);

  /** determines if given translation is available and where
   *
   * @param lang             identifier of language
   * @result                 index of translation, 0=original, -1=none
   */
  int findIdent (const QString lang) const;

  /** returns attribute string 
   *
   * @param index            number of attribute
   * @result                 string
   */
  QString getTypeName (int index) const;

  /** sets attribute string
   *
   * @param index            number of attribute
   * @param str              name of attribute
   */
  void setTypeName (int index, QString str);

  /** gets descr of types  */
  inline vector<QString> getTypeDescr() const { return type_descr; }

  /** sets descr of types  */
  inline void setTypeDescr(vector<QString> names) { type_descr = names; }

  /** returns tense string
   *
   * @param index            number of tense
   * @result                 string
   */
  QString getTenseName (int index) const;

  /** sets tense string
   *
   * @param index            number of tense
   * @param str              name of tense
   */
  void setTenseName (int index, QString str);

  /** gets descr of tenses  */
  inline vector<QString> getTenseDescr() const { return tense_descr; }

  /** sets descr of tenses  */
  inline void setTenseDescr(vector<QString> names) { tense_descr = names; }

  /** returns usage string
   *
   * @param index            number of usage
   * @result                 string
   */
  QString getUsageName (int index) const;

  /** sets usage string
   *
   * @param index            number of usage
   * @param str              name of usage
   */
  void setUsageName (int index, QString str);

  /** gets descr of usages  */
  inline vector<QString> getUsageDescr() const { return usage_descr; }

  /** sets descr of usages  */
  inline void setUsageDescr(vector<QString> names) { usage_descr = names; }


  /** saves the data under the given name
   *
   * @param name     if name is empty (or NULL) actual name is preserved
   * @result         true if successful
   */
  bool saveAs (QObject *parent, QString name, QString title,
               FileType ft = automatic,
               const QString separator = "\t" , QStringList *lang_order = 0);

  /** returns count of entries
   */
  inline int numEntries() const { return vocabulary.size(); }

  /** sets grades to KV_NORM_GRADE, counts to 0 ...
   *
   * @param index    index of language 0..x, -1 = all
   * @param lesson   lesson id, if this is 0 all lesson are affected,
   *                 otherwise only matching numbers
   */
  void resetEntry (int index = -1, int lesson = 0);

  /** returns count of different languages
   */
  inline int numLangs() const { return langs.size(); } // org + translations

  /** append new lang ident
   */
  inline void appendLang(QString id) { langs.push_back(id); }

  /** returns pointer to expression object x
   *
   * @param index     index of desired entry
   * @result          pointer to object or NULL if index out of range
   */
  kvoctrainExpr *getEntry(int index);

  /** search substring in vocabulary (case insensitive always)
   *
   * @param substr     partial string to search
   * @param id         which language to search: 0=org, 1..x=translation
   * @param first      index from where to start
   * @param last       index of last entry, -1 goes till end
   * @param word_start false: search partial string,
   *                   true:always from beginning of word
   * @param tolerant
   * @result          index of found entry, -1 if none
   */
  int search(QString substr, int id,
             int first=0, int last=-1, bool word_start = false, bool tolerant=false);

  /** returns name of xml file  */
  inline QString getFileName() const { return mainfile; }

  /** sets name of xml file  */
  inline void setFileName(QString s) { mainfile = s; }

  /** returns title of xml file  */
  QString getTitle() const;

  /** returns author of file  */
  QString getAuthor() const;

  /** returns license of file  */
  QString getLicense() const;

  /** returns remark of file  */
  QString getDocRemark() const;

  inline void getQueryLang(QString &org, QString &trans) const
    { org = queryorg; trans = querytrans; }

  inline void setQueryLang(QString org, QString trans)
    { queryorg = org; querytrans = trans; }

  /** sets title of xml file  */
  void setTitle(QString title);

  /** sets author of file  */
  void setAuthor(QString author);

  /** sets license of file  */
  void setLicense(QString license);

  /** sets remark of file  */
  void setDocRemark(QString rem);

  /** gets version of loaded file  */
  void getVersion(int &major, int &minor, int &patch);

  /** gets encoding loaded file  */
  inline KV_Encoding getEncoding() const { return doc_encoder; }

  /** gets encoding loaded file  */
  inline void setEncoding(KV_Encoding enc) { doc_encoder = enc; }

  /** returns current lesson index  */
  inline int getCurrentLesson() const { return current_lesson; }

  /** sets current lesson index
   * @param lesson    index of lesson
   */
  inline void setCurrentLesson(int lesson) { current_lesson = lesson; }

  /** returns descr of lesson  */
  QString getLessonDescr(int index) const;

  /** returns lessons in current query  */
  vector<int> getLessonsInQuery() const;

  /** sets lessons in current query  */
  void setLessonsInQuery(vector<int>);

  inline vector<QString> getLessonDescr() const { return lesson_descr; }

  inline int numLessons () const {return (int) lesson_descr.size(); }

  /** sets descr of lesson  */
  inline void setLessonDescr(vector<QString> names) { lesson_descr = names; }

  /** returns pointer to conjugations if available
   *
   * @param index            index of translation
   */
  Conjugation getConjugation(int index) const;

  /** sets conjugations
   *
   * @param index            index of translation
   * @param con              conjugation block
   */
  void setConjugation(int index, const Conjugation &con);

  /** returns pointer to articles if available
   *
   * @param index            index of translation
   */
  Article getArticle(int index) const;

  /** sets articles
   *
   * @param index            index of translation
   * @param art              article block
   */
  void setArticle(int index, const Article &art);

  /** compress date */
  QString compressDate(unsigned long) const;

  /** decompress date */
  unsigned long decompressDate(QString) const;

  /** returns recommended size
   *
   * @param index            number of expr, -1 = lesson
   * @result                 width of column
   */
  int getSizeHint (int index) const;

  /** sets recommended size
   *
   * @param index            number of expr, -1 = lesson
   * @param width            width of column
   */
  void setSizeHint (int index, const int width);

#if QT_VERSION < 300
  /** returns recommended charset
   *
   * @param index            number of expr
   * @result                 charset
   */
  QFont::CharSet getCharSet (int index) const;

  /** returns recommended charset as string
   *
   * @param index            number of expr
   * @result                 charset in string form
   */
  QString getCharSetString (int index) const;

  // relation QFont::xx -> readable names
  static QString charSet2String (QFont::CharSet cs, bool show_std = false, bool show_countries = false);
  static QFont::CharSet string2CharSet (QString &s);

  // X uses different names
  static QString charSet2XName(QFont::CharSet charSet);
  static QFont::CharSet XName2CharSet (QString &s);

  /** sets recommended charset
   *
   * @param index            number of expr
   * @param width            charset
   */
  void setCharSet (int index, const QFont::CharSet cs);
#endif

  bool unknownAttribute (int line, const QString &name, const QString &attr);
  void unknownElement (int line, const QString &elem );
  void errorKvtMl (int line, const QString &text );
  void warningKvtMl (int line, const QString &text );

  void errorLex (int line, const QString &text );

  void errorCsv (int line, const QString &text );

  FileType detectFT(const QString &filename);

  friend class QueryManager;

signals:
  void progressChanged (kvoctrainDoc *, int curr_percent);
  void docModified (bool mod);

protected:

  void Init();

  /** sets version of loaded file  */
  void setVersion(QString ver);

  bool saveToKvtMl     (QTextStream& os, QString title);
  bool loadFromKvtMl   (QTextStream& is);
  bool parseBody_kvtml (XmlElement elem, XmlReader& xml);
  bool parseBody_e     (XmlElement elem, XmlReader& xml);
  bool extract_simple_tag (QString tag_name, XmlReader& xml,
                           XmlElement &elem, QString &data,
                           bool check_empty = true);

  bool extract_T_GROUP_attr (XmlReader &xml,
                             XmlElement &elem);
  bool extract_T_DESCR_attr (XmlReader &xml,
                             XmlElement &elem, int &no);
  bool extract_O_T_attr (XmlReader &xml,
                         XmlElement &elem,
                         QString &lang,
                         grade_t &grade, grade_t &rev_grade,
                         int &count, int &rev_count,
                         time_t &date, time_t &rev_date,
                         QString &remark,
                         int &bcount, int &rev_bcount,
                         QString &query_id,
                         QString &pronunce,
                         int &width,
#if QT_VERSION < 300
                         QFont::CharSet &cs,
#endif
                         QString &type,
                         QString &faux_ami_f,
                         QString &faux_ami_t,
                         QString &synonym,
                         QString &example,
                         QString &antonym,
                         QString &usage,
                         QString &paraphrase);
  bool extract_L_DESCR_attr (XmlReader &xml,
                             XmlElement &elem, int &no,
                             bool &isCurr, bool &inQuery);
  bool extract_KVT_E_attr (XmlReader& xml,
                           XmlElement   &elem,
                           int          &lesson,
                           bool         &sel,
                           QString      &type);
  bool check_Empty_attr (QString tag_name,
                         XmlReader &xml,
                         XmlElement &elem);
  bool extract_CON_E_attr (XmlReader& xml,
                           XmlElement &elem,
                           QString &lang);
  bool extract_T_attr (XmlReader &xml,
                       XmlElement &elem,
                       QString &name);
  bool extract_BOOL_attr (XmlReader &xml,
                          XmlElement &elem,
                          const QString &tag_name,
                          const QString &attr_name,
                          bool &flag);
  bool extract_L_GROUP_attr (XmlReader &xml,
                             XmlElement &elem,
                             int &width);

  bool saveLessonKvtMl (XmlWriter &xml);
  bool loadLessonKvtMl (XmlElement elem, XmlReader& xml);
  bool saveTypeNameKvtMl (XmlWriter &xml);
  bool loadTypeNameKvtMl (XmlElement elem, XmlReader& xml);
  bool saveTenseNameKvtMl (XmlWriter &xml);
  bool loadTenseNameKvtMl (XmlElement elem, XmlReader& xml);
  bool saveUsageNameKvtMl (XmlWriter &xml);
  bool loadUsageNameKvtMl (XmlElement elem, XmlReader& xml);
  bool saveOptionsKvtMl  (XmlWriter &xml);
  bool loadOptionsKvtMl  (XmlElement elem, XmlReader& xml);
  bool saveArticleKvtMl  (XmlWriter &xml);
  bool loadArticleKvtMl  (XmlElement elem, XmlReader& xml);
  bool saveConjugHeader  (vector<Conjugation> &curr_conjug,
                          XmlWriter &xml);
  bool saveConjug        (const Conjugation &curr_conjug, QString type,
                          XmlWriter &xml, QString identstr);
  bool saveConjugEntry   (Conjugation &curr_conjug,
                          XmlWriter &xml, int ident);
  bool loadConjugKvtMl   (vector<Conjugation> &curr_conjug, const QString entry_tag,
                          XmlElement elem, XmlReader& xml);
  bool saveComparison    (const Comparison comp, XmlWriter &xml, int ident);
  bool loadComparison    (Comparison &comp, XmlElement elem, XmlReader &xml);

  bool saveToLex     (QTextStream& os, QString title);
  bool loadFromLex   (QTextStream& is);
  bool loadLessonLex (QTextStream& is);
  bool saveLessonLex (QTextStream& os);
  bool saveTypeNameLex   (QTextStream& os);
  bool loadTypeNameLex   (QTextStream& is);

  bool saveToCsv     (QTextStream& os, QString title, const QString separator, QStringList *lang_order);
  bool loadFromCsv   (QTextStream& is, const QString separator, QStringList *lang_order);
  bool loadLessonCsv (QTextStream& is);
  bool saveLessonCsv (QTextStream& os);
  bool saveTypeNameCsv   (QTextStream& os);
  bool loadTypeNameCsv   (QTextStream& is);

  bool saveTypeNameVcb (QTextStream &os);
  bool loadTypeNameVcb (QTextStream &is);
  bool saveLessonVcb (QTextStream &os);
  bool loadLessonVcb (QTextStream &is);
  bool saveToVcb (QTextStream& os, QString title);
  bool loadFromVcb (QTextStream& is);
  void errorVcb (int line, const QString &text );

 private:
  bool                  dirty;
  QString               mainfile;
  vector<bool>          sort_lang;
  bool                  sort_lesson;
  bool                  unknown_attr;
  bool                  unknown_elem;
  bool                  sort_allowed;

  // save these to document
  vector<QString>        langs;      //0= origin, 1,.. translations
  int                    cols,
                         lines;
  int                    current_lesson;
  vector<int>            extraSizehints;
  vector<int>            sizehints;
#if QT_VERSION < 300
  vector<QFont::CharSet> charsets;
#endif
  QString                generator;
  QString                queryorg,
                         querytrans;
  vector<kvoctrainExpr>  vocabulary;
  vector<bool>           lessons_in_query;
  vector<QString>        lesson_descr;
  vector<QString>        type_descr;
  vector<QString>        tense_descr;
  vector<QString>        usage_descr;
  QString                doctitle;
  QString                author;
  QString                license;
  QString                doc_remark;
  QString                doc_version;

  vector<Article>        articles;
  vector<Conjugation>    conjugations;
  KV_Encoding            doc_encoder;
};


#endif // KVOCTRAINDOC_H
