/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <QMainWindow>
#include <QMap>
#include <QPointer>

QT_BEGIN_NAMESPACE
class QAction;
class QComboBox;
class QFontComboBox;
class QTextCharFormat;
class QMenu;
class QPrinter;
QT_END_NAMESPACE

#include <QTextCursor>
#include <QTextEdit>


/* MY ADD START */
class User{
public:
    User(int u, QString n, QColor col, int tc): uid(u), nick(n), color(col), curs(tc){}
    int uid;                    // se faccio map<int, user> non serve, la uso come chiave
    QString nick;
    QColor color;
    int curs;
    QImage icon; // controllare il tipo
};

//nuova idea di messaggio beside NotifyCursor e Message
// messaggio consiste nella classe User
// ???viene inviato al momento dell aconnessione al server insieme ad identificazione o quando lo si modifica(nick, icona..)???

class NotifyCursor{
public:
    NotifyCursor(int curs, int u): cursPos(curs), uid(u){}
    int cursPos;
    int uid;
};

class Symbol {
public:
    Symbol(QChar i, int i1, int i2, std::vector<int>& vector): c(i), siteid(i1), count(i2), fract(vector){}
    QChar c;
    int siteid;
    int count;
    std::vector<int> fract;
};

class Message {
public:
    Message(char i, Symbol& pSymbol, int i1): mType(i), sym(pSymbol), genFrom(i1){}
    int mType;
    Symbol sym;
    int genFrom;

};

class MyQTextEdit: public QTextEdit{
    Q_OBJECT
public:
    MyQTextEdit(QWidget* p);
    ~MyQTextEdit();
    void paintEvent(QPaintEvent *e);
    void localInsert(int i, QChar i1);
    void localErase(int i);
private:
    //NetworkServer& _server ;
    int _siteId = 0;
    QMap<int, User> _users;
public:
    QMap<int, QTextCursor> _cursors;
    int getSiteId();
    QString to_string();
    void process(const Message &m);
    void process(const NotifyCursor &n);
private:
    std::vector<Symbol> _symbols;
    int _counter = 0;
public slots:
    void CatchChangeSignal(int pos, int rem, int add); // move to private?

    void myCursorPositionChanged();
};

/* MY ADD END */

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit(QWidget *parent = 0);

    bool load(const QString &f);

public slots:
    void fileNew();
protected:
    void virtual closeEvent(QCloseEvent *e) override;

private slots:
    void fileOpen();
    bool fileSave();
    bool fileSaveAs();
    void filePrint();
    void filePrintPreview();
    void filePrintPdf();

    void textBold();
    void textUnderline();
    void textItalic();
    void textFamily(const QString &f);
    void textSize(const QString &p);
    void textStyle(int styleIndex);
    void textColor();
    void textAlign(QAction *a);

    void currentCharFormatChanged(const QTextCharFormat &format);
    void cursorPositionChanged();

    void clipboardDataChanged();
    void about();
    void printPreview(QPrinter *);

private:
    void setupFileActions();
    void setupEditActions();
    void setupTextActions();
    bool maybeSave();
    void setCurrentFileName(const QString &fileName);

    void mergeFormatOnWordOrSelection(const QTextCharFormat &format);
    void fontChanged(const QFont &f);
    void colorChanged(const QColor &c);
    void alignmentChanged(Qt::Alignment a);

    QAction *actionSave;
    QAction *actionTextBold;
    QAction *actionTextUnderline;
    QAction *actionTextItalic;
    QAction *actionTextColor;
    QAction *actionAlignLeft;
    QAction *actionAlignCenter;
    QAction *actionAlignRight;
    QAction *actionAlignJustify;
    QAction *actionUndo;
    QAction *actionRedo;
#ifndef QT_NO_CLIPBOARD
    QAction *actionCut;
    QAction *actionCopy;
    QAction *actionPaste;
#endif

    QComboBox *comboStyle;
    QFontComboBox *comboFont;
    QComboBox *comboSize;

    QToolBar *tb;
    QString fileName;
    MyQTextEdit *textEdit;                          // ONLY CHANGE

};

#endif // TEXTEDIT_H
