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

#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QColorDialog>
#include <QComboBox>
#include <QFontComboBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontDatabase>
#include <QMenu>
#include <QMenuBar>
#include <QTextCodec>
#include <QTextEdit>
#include <QStatusBar>
#include <QToolBar>
#include <QTextCursor>
#include <QTextDocumentWriter>
#include <QTextList>
#include <QtDebug>
#include <QCloseEvent>
#include <QMessageBox>
#include <QMimeData>
#if defined(QT_PRINTSUPPORT_LIB)
#include <QtPrintSupport/qtprintsupportglobal.h>
#if QT_CONFIG(printer)
#if QT_CONFIG(printdialog)
#include <QPrintDialog>
#endif
#include <QPrinter>
#if QT_CONFIG(printpreviewdialog)
#include <QPrintPreviewDialog>
#endif
#endif
#endif

#include "textedit.h"

// my include
#include <QPainter>
#include <QtNetwork>


#ifdef Q_OS_MAC
const QString rsrcPath = ":/images/mac";
#else
const QString rsrcPath = ":/images/win";
#endif


TextEdit::TextEdit(QWidget *parent)
    : QMainWindow(parent)
{
#ifdef Q_OS_OSX
    setUnifiedTitleAndToolBarOnMac(true);
#endif
    setWindowTitle(QCoreApplication::applicationName());

    textEdit = new MyQTextEdit(this);                       // MY ONLY CHANGE HERE
    connect(textEdit, &QTextEdit::currentCharFormatChanged,
            this, &TextEdit::currentCharFormatChanged);
    connect(textEdit, &QTextEdit::cursorPositionChanged,this, &TextEdit::cursorPositionChanged);
    setCentralWidget(textEdit);

    setToolButtonStyle(Qt::ToolButtonFollowStyle);
    setupFileActions();
    setupEditActions();
    setupTextActions();

    {
        QMenu *helpMenu = menuBar()->addMenu(tr("Help"));
        helpMenu->addAction(tr("About"), this, &TextEdit::about);
        helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    }

    QFont textFont("Helvetica");
    textFont.setStyleHint(QFont::SansSerif);
    textEdit->setFont(textFont);
    fontChanged(textEdit->font());
    colorChanged(textEdit->textColor());
    alignmentChanged(textEdit->alignment());

    connect(textEdit->document(), &QTextDocument::modificationChanged,
            actionSave, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::modificationChanged,
            this, &QWidget::setWindowModified);
    connect(textEdit->document(), &QTextDocument::undoAvailable,
            actionUndo, &QAction::setEnabled);
    connect(textEdit->document(), &QTextDocument::redoAvailable,
            actionRedo, &QAction::setEnabled);

    setWindowModified(textEdit->document()->isModified());
    actionSave->setEnabled(textEdit->document()->isModified());
    actionUndo->setEnabled(textEdit->document()->isUndoAvailable());
    actionRedo->setEnabled(textEdit->document()->isRedoAvailable());

#ifndef QT_NO_CLIPBOARD
    actionCut->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCut, &QAction::setEnabled);
    actionCopy->setEnabled(false);
    connect(textEdit, &QTextEdit::copyAvailable, actionCopy, &QAction::setEnabled);

    connect(QApplication::clipboard(), &QClipboard::dataChanged, this, &TextEdit::clipboardDataChanged);
#endif

    textEdit->setFocus();
    setCurrentFileName(QString());

#ifdef Q_OS_MACOS
    // Use dark text on light background on macOS, also in dark mode.
    QPalette pal = textEdit->palette();
    pal.setColor(QPalette::Base, QColor(Qt::white));
    pal.setColor(QPalette::Text, QColor(Qt::black));
    textEdit->setPalette(pal);
#endif

}

void TextEdit::closeEvent(QCloseEvent *e)
{
    if (maybeSave())
        e->accept();
    else
        e->ignore();
}

void TextEdit::setupFileActions()
{
    QToolBar *tb = addToolBar(tr("File Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&File"));

    const QIcon newIcon = QIcon::fromTheme("document-new", QIcon(rsrcPath + "/filenew.png"));
    QAction *a = menu->addAction(newIcon,  tr("&New"), this, &TextEdit::fileNew);
    tb->addAction(a);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::New);

    const QIcon openIcon = QIcon::fromTheme("document-open", QIcon(rsrcPath + "/fileopen.png"));
    a = menu->addAction(openIcon, tr("&Open..."), this, &TextEdit::fileOpen);
    a->setShortcut(QKeySequence::Open);
    tb->addAction(a);

    menu->addSeparator();

    const QIcon saveIcon = QIcon::fromTheme("document-save", QIcon(rsrcPath + "/filesave.png"));
    actionSave = menu->addAction(saveIcon, tr("&Save"), this, &TextEdit::fileSave);
    actionSave->setShortcut(QKeySequence::Save);
    actionSave->setEnabled(false);
    tb->addAction(actionSave);

    a = menu->addAction(tr("Save &As..."), this, &TextEdit::fileSaveAs);
    a->setPriority(QAction::LowPriority);
    menu->addSeparator();

#ifndef QT_NO_PRINTER
    const QIcon printIcon = QIcon::fromTheme("document-print", QIcon(rsrcPath + "/fileprint.png"));
    a = menu->addAction(printIcon, tr("&Print..."), this, &TextEdit::filePrint);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(QKeySequence::Print);
    tb->addAction(a);

    const QIcon filePrintIcon = QIcon::fromTheme("fileprint", QIcon(rsrcPath + "/fileprint.png"));
    menu->addAction(filePrintIcon, tr("Print Preview..."), this, &TextEdit::filePrintPreview);

    const QIcon exportPdfIcon = QIcon::fromTheme("exportpdf", QIcon(rsrcPath + "/exportpdf.png"));
    a = menu->addAction(exportPdfIcon, tr("&Export PDF..."), this, &TextEdit::filePrintPdf);
    a->setPriority(QAction::LowPriority);
    a->setShortcut(Qt::CTRL + Qt::Key_D);
    tb->addAction(a);

    menu->addSeparator();
#endif

    a = menu->addAction(tr("&Quit"), this, &QWidget::close);
    a->setShortcut(Qt::CTRL + Qt::Key_Q);
}

void TextEdit::setupEditActions()
{
    QToolBar *tb = addToolBar(tr("Edit Actions"));
    QMenu *menu = menuBar()->addMenu(tr("&Edit"));

    const QIcon undoIcon = QIcon::fromTheme("edit-undo", QIcon(rsrcPath + "/editundo.png"));
    actionUndo = menu->addAction(undoIcon, tr("&Undo"), textEdit, &QTextEdit::undo);
    actionUndo->setShortcut(QKeySequence::Undo);
    tb->addAction(actionUndo);

    const QIcon redoIcon = QIcon::fromTheme("edit-redo", QIcon(rsrcPath + "/editredo.png"));
    actionRedo = menu->addAction(redoIcon, tr("&Redo"), textEdit, &QTextEdit::redo);
    actionRedo->setPriority(QAction::LowPriority);
    actionRedo->setShortcut(QKeySequence::Redo);
    tb->addAction(actionRedo);
    menu->addSeparator();

#ifndef QT_NO_CLIPBOARD
    const QIcon cutIcon = QIcon::fromTheme("edit-cut", QIcon(rsrcPath + "/editcut.png"));
    actionCut = menu->addAction(cutIcon, tr("Cu&t"), textEdit, &QTextEdit::cut);
    actionCut->setPriority(QAction::LowPriority);
    actionCut->setShortcut(QKeySequence::Cut);
    tb->addAction(actionCut);

    const QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(rsrcPath + "/editcopy.png"));
    actionCopy = menu->addAction(copyIcon, tr("&Copy"), textEdit, &QTextEdit::copy);
    actionCopy->setPriority(QAction::LowPriority);
    actionCopy->setShortcut(QKeySequence::Copy);
    tb->addAction(actionCopy);

    const QIcon pasteIcon = QIcon::fromTheme("edit-paste", QIcon(rsrcPath + "/editpaste.png"));
    actionPaste = menu->addAction(pasteIcon, tr("&Paste"), textEdit, &QTextEdit::paste);
    actionPaste->setPriority(QAction::LowPriority);
    actionPaste->setShortcut(QKeySequence::Paste);
    tb->addAction(actionPaste);
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::setupTextActions()
{
    QToolBar *tb = addToolBar(tr("Format Actions"));
    QMenu *menu = menuBar()->addMenu(tr("F&ormat"));

    const QIcon boldIcon = QIcon::fromTheme("format-text-bold", QIcon(rsrcPath + "/textbold.png"));
    actionTextBold = menu->addAction(boldIcon, tr("&Bold"), this, &TextEdit::textBold);
    actionTextBold->setShortcut(Qt::CTRL + Qt::Key_B);
    actionTextBold->setPriority(QAction::LowPriority);
    QFont bold;
    bold.setBold(true);
    actionTextBold->setFont(bold);
    tb->addAction(actionTextBold);
    actionTextBold->setCheckable(true);

    const QIcon italicIcon = QIcon::fromTheme("format-text-italic", QIcon(rsrcPath + "/textitalic.png"));
    actionTextItalic = menu->addAction(italicIcon, tr("&Italic"), this, &TextEdit::textItalic);
    actionTextItalic->setPriority(QAction::LowPriority);
    actionTextItalic->setShortcut(Qt::CTRL + Qt::Key_I);
    QFont italic;
    italic.setItalic(true);
    actionTextItalic->setFont(italic);
    tb->addAction(actionTextItalic);
    actionTextItalic->setCheckable(true);

    const QIcon underlineIcon = QIcon::fromTheme("format-text-underline", QIcon(rsrcPath + "/textunder.png"));
    actionTextUnderline = menu->addAction(underlineIcon, tr("&Underline"), this, &TextEdit::textUnderline);
    actionTextUnderline->setShortcut(Qt::CTRL + Qt::Key_U);
    actionTextUnderline->setPriority(QAction::LowPriority);
    QFont underline;
    underline.setUnderline(true);
    actionTextUnderline->setFont(underline);
    tb->addAction(actionTextUnderline);
    actionTextUnderline->setCheckable(true);

    menu->addSeparator();

    const QIcon leftIcon = QIcon::fromTheme("format-justify-left", QIcon(rsrcPath + "/textleft.png"));
    actionAlignLeft = new QAction(leftIcon, tr("&Left"), this);
    actionAlignLeft->setShortcut(Qt::CTRL + Qt::Key_L);
    actionAlignLeft->setCheckable(true);
    actionAlignLeft->setPriority(QAction::LowPriority);
    const QIcon centerIcon = QIcon::fromTheme("format-justify-center", QIcon(rsrcPath + "/textcenter.png"));
    actionAlignCenter = new QAction(centerIcon, tr("C&enter"), this);
    actionAlignCenter->setShortcut(Qt::CTRL + Qt::Key_E);
    actionAlignCenter->setCheckable(true);
    actionAlignCenter->setPriority(QAction::LowPriority);
    const QIcon rightIcon = QIcon::fromTheme("format-justify-right", QIcon(rsrcPath + "/textright.png"));
    actionAlignRight = new QAction(rightIcon, tr("&Right"), this);
    actionAlignRight->setShortcut(Qt::CTRL + Qt::Key_R);
    actionAlignRight->setCheckable(true);
    actionAlignRight->setPriority(QAction::LowPriority);
    const QIcon fillIcon = QIcon::fromTheme("format-justify-fill", QIcon(rsrcPath + "/textjustify.png"));
    actionAlignJustify = new QAction(fillIcon, tr("&Justify"), this);
    actionAlignJustify->setShortcut(Qt::CTRL + Qt::Key_J);
    actionAlignJustify->setCheckable(true);
    actionAlignJustify->setPriority(QAction::LowPriority);

    // Make sure the alignLeft  is always left of the alignRight
    QActionGroup *alignGroup = new QActionGroup(this);
    connect(alignGroup, &QActionGroup::triggered, this, &TextEdit::textAlign);

    if (QApplication::isLeftToRight()) {
        alignGroup->addAction(actionAlignLeft);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignRight);
    } else {
        alignGroup->addAction(actionAlignRight);
        alignGroup->addAction(actionAlignCenter);
        alignGroup->addAction(actionAlignLeft);
    }
    alignGroup->addAction(actionAlignJustify);

    tb->addActions(alignGroup->actions());
    menu->addActions(alignGroup->actions());

    menu->addSeparator();

    QPixmap pix(16, 16);
    pix.fill(Qt::black);
    actionTextColor = menu->addAction(pix, tr("&Color..."), this, &TextEdit::textColor);
    tb->addAction(actionTextColor);

    tb = addToolBar(tr("Format Actions"));
    tb->setAllowedAreas(Qt::TopToolBarArea | Qt::BottomToolBarArea);
    addToolBarBreak(Qt::TopToolBarArea);
    addToolBar(tb);

    comboStyle = new QComboBox(tb);
    tb->addWidget(comboStyle);
    comboStyle->addItem("Standard");
    comboStyle->addItem("Bullet List (Disc)");
    comboStyle->addItem("Bullet List (Circle)");
    comboStyle->addItem("Bullet List (Square)");
    comboStyle->addItem("Ordered List (Decimal)");
    comboStyle->addItem("Ordered List (Alpha lower)");
    comboStyle->addItem("Ordered List (Alpha upper)");
    comboStyle->addItem("Ordered List (Roman lower)");
    comboStyle->addItem("Ordered List (Roman upper)");
    comboStyle->addItem("Heading 1");
    comboStyle->addItem("Heading 2");
    comboStyle->addItem("Heading 3");
    comboStyle->addItem("Heading 4");
    comboStyle->addItem("Heading 5");
    comboStyle->addItem("Heading 6");

    connect(comboStyle, QOverload<int>::of(&QComboBox::activated), this, &TextEdit::textStyle);

    comboFont = new QFontComboBox(tb);
    tb->addWidget(comboFont);
    connect(comboFont, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textFamily);

    comboSize = new QComboBox(tb);
    comboSize->setObjectName("comboSize");
    tb->addWidget(comboSize);
    comboSize->setEditable(true);

    const QList<int> standardSizes = QFontDatabase::standardSizes();
    for (int size : standardSizes)
        comboSize->addItem(QString::number(size));
    comboSize->setCurrentIndex(standardSizes.indexOf(QApplication::font().pointSize()));

    connect(comboSize, QOverload<const QString &>::of(&QComboBox::activated), this, &TextEdit::textSize);
}

bool TextEdit::load(const QString &f)
{
    if (!QFile::exists(f))
        return false;
    QFile file(f);
    if (!file.open(QFile::ReadOnly))
        return false;

    QByteArray data = file.readAll();
    QTextCodec *codec = Qt::codecForHtml(data);
    QString str = codec->toUnicode(data);
    if (Qt::mightBeRichText(str)) {
        textEdit->setHtml(str);
    } else {
        str = QString::fromLocal8Bit(data);
        textEdit->setPlainText(str);
    }

    setCurrentFileName(f);
    return true;
}

bool TextEdit::maybeSave()
{
    if (!textEdit->document()->isModified())
        return true;

    const QMessageBox::StandardButton ret =
        QMessageBox::warning(this, QCoreApplication::applicationName(),
                             tr("The document has been modified.\n"
                                "Do you want to save your changes?"),
                             QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
        return fileSave();
    else if (ret == QMessageBox::Cancel)
        return false;
    return true;
}

void TextEdit::setCurrentFileName(const QString &fileName)
{
    this->fileName = fileName;
    textEdit->document()->setModified(false);

    QString shownName;
    if (fileName.isEmpty())
        shownName = "untitled.txt";
    else
        shownName = QFileInfo(fileName).fileName();

    setWindowTitle(tr("%1[*] - %2").arg(shownName, QCoreApplication::applicationName()));
    setWindowModified(false);
}

void TextEdit::fileNew()
{
    if (maybeSave()) {
        textEdit->clear();
        setCurrentFileName(QString());
    }

}

void TextEdit::fileOpen()
{
    QFileDialog fileDialog(this, tr("Open File..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    fileDialog.setFileMode(QFileDialog::ExistingFile);
    fileDialog.setMimeTypeFilters(QStringList() << "text/html" << "text/plain");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    const QString fn = fileDialog.selectedFiles().first();
    if (load(fn))
        statusBar()->showMessage(tr("Opened \"%1\"").arg(QDir::toNativeSeparators(fn)));
    else
        statusBar()->showMessage(tr("Could not open \"%1\"").arg(QDir::toNativeSeparators(fn)));
}

bool TextEdit::fileSave()
{
    if (fileName.isEmpty())
        return fileSaveAs();
    if (fileName.startsWith(QStringLiteral(":/")))
        return fileSaveAs();

    QTextDocumentWriter writer(fileName);
    bool success = writer.write(textEdit->document());
    if (success) {
        textEdit->document()->setModified(false);
        statusBar()->showMessage(tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName)));
    } else {
        statusBar()->showMessage(tr("Could not write to file \"%1\"")
                                 .arg(QDir::toNativeSeparators(fileName)));
    }
    return success;
}

bool TextEdit::fileSaveAs()
{
    QFileDialog fileDialog(this, tr("Save as..."));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    QStringList mimeTypes;
    mimeTypes << "application/vnd.oasis.opendocument.text" << "text/html" << "text/plain";
    fileDialog.setMimeTypeFilters(mimeTypes);
    fileDialog.setDefaultSuffix("odt");
    if (fileDialog.exec() != QDialog::Accepted)
        return false;
    const QString fn = fileDialog.selectedFiles().first();
    setCurrentFileName(fn);
    return fileSave();
}

void TextEdit::filePrint()
{
#if QT_CONFIG(printdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog *dlg = new QPrintDialog(&printer, this);
    if (textEdit->textCursor().hasSelection())
        dlg->addEnabledOption(QAbstractPrintDialog::PrintSelection);
    dlg->setWindowTitle(tr("Print Document"));
    if (dlg->exec() == QDialog::Accepted)
        textEdit->print(&printer);
    delete dlg;
#endif
}

void TextEdit::filePrintPreview()
{
#if QT_CONFIG(printpreviewdialog)
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, this, &TextEdit::printPreview);
    preview.exec();
#endif
}

void TextEdit::printPreview(QPrinter *printer)
{
#ifdef QT_NO_PRINTER
    Q_UNUSED(printer);
#else
    textEdit->print(printer);
#endif
}


void TextEdit::filePrintPdf()
{
#ifndef QT_NO_PRINTER
//! [0]
    QFileDialog fileDialog(this, tr("Export PDF"));
    fileDialog.setAcceptMode(QFileDialog::AcceptSave);
    fileDialog.setMimeTypeFilters(QStringList("application/pdf"));
    fileDialog.setDefaultSuffix("pdf");
    if (fileDialog.exec() != QDialog::Accepted)
        return;
    QString fileName = fileDialog.selectedFiles().first();
    QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    textEdit->document()->print(&printer);
    statusBar()->showMessage(tr("Exported \"%1\"")
                             .arg(QDir::toNativeSeparators(fileName)));
//! [0]
#endif
}

void TextEdit::textBold()
{
    QTextCharFormat fmt;
    fmt.setFontWeight(actionTextBold->isChecked() ? QFont::Bold : QFont::Normal);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textUnderline()
{
    QTextCharFormat fmt;
    fmt.setFontUnderline(actionTextUnderline->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textItalic()
{
    QTextCharFormat fmt;
    fmt.setFontItalic(actionTextItalic->isChecked());
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textFamily(const QString &f)
{
    QTextCharFormat fmt;
    fmt.setFontFamily(f);
    mergeFormatOnWordOrSelection(fmt);
}

void TextEdit::textSize(const QString &p)
{
    qreal pointSize = p.toFloat();
    if (p.toFloat() > 0) {
        QTextCharFormat fmt;
        fmt.setFontPointSize(pointSize);
        mergeFormatOnWordOrSelection(fmt);
    }
}

void TextEdit::textStyle(int styleIndex)
{
    QTextCursor cursor = textEdit->textCursor();
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

    switch (styleIndex) {
    case 1:
        style = QTextListFormat::ListDisc;
        break;
    case 2:
        style = QTextListFormat::ListCircle;
        break;
    case 3:
        style = QTextListFormat::ListSquare;
        break;
    case 4:
        style = QTextListFormat::ListDecimal;
        break;
    case 5:
        style = QTextListFormat::ListLowerAlpha;
        break;
    case 6:
        style = QTextListFormat::ListUpperAlpha;
        break;
    case 7:
        style = QTextListFormat::ListLowerRoman;
        break;
    case 8:
        style = QTextListFormat::ListUpperRoman;
        break;
    default:
        break;
    }

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();

    if (style == QTextListFormat::ListStyleUndefined) {
        blockFmt.setObjectIndex(-1);
        int headingLevel = styleIndex >= 9 ? styleIndex - 9 + 1 : 0; // H1 to H6, or Standard
        blockFmt.setHeadingLevel(headingLevel);
        cursor.setBlockFormat(blockFmt);

        int sizeAdjustment = headingLevel ? 4 - headingLevel : 0; // H1 to H6: +3 to -2
        QTextCharFormat fmt;
        fmt.setFontWeight(headingLevel ? QFont::Bold : QFont::Normal);
        fmt.setProperty(QTextFormat::FontSizeAdjustment, sizeAdjustment);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.mergeCharFormat(fmt);
        textEdit->mergeCurrentCharFormat(fmt);
    } else {
        QTextListFormat listFmt;
        if (cursor.currentList()) {
            listFmt = cursor.currentList()->format();
        } else {
            listFmt.setIndent(blockFmt.indent() + 1);
            blockFmt.setIndent(0);
            cursor.setBlockFormat(blockFmt);
        }
        listFmt.setStyle(style);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
}

void TextEdit::textColor()
{
    QColor col = QColorDialog::getColor(textEdit->textColor(), this);
    if (!col.isValid())
        return;
    QTextCharFormat fmt;
    fmt.setForeground(col);
    mergeFormatOnWordOrSelection(fmt);
    colorChanged(col);
}

void TextEdit::textAlign(QAction *a)
{
    if (a == actionAlignLeft)
        textEdit->setAlignment(Qt::AlignLeft | Qt::AlignAbsolute);
    else if (a == actionAlignCenter)
        textEdit->setAlignment(Qt::AlignHCenter);
    else if (a == actionAlignRight)
        textEdit->setAlignment(Qt::AlignRight | Qt::AlignAbsolute);
    else if (a == actionAlignJustify)
        textEdit->setAlignment(Qt::AlignJustify);
}

void TextEdit::currentCharFormatChanged(const QTextCharFormat &format)
{
    fontChanged(format.font());
    colorChanged(format.foreground().color());
}

void TextEdit::cursorPositionChanged()
{


    alignmentChanged(textEdit->alignment());
    QTextList *list = textEdit->textCursor().currentList();
    if (list) {
        switch (list->format().style()) {
        case QTextListFormat::ListDisc:
            comboStyle->setCurrentIndex(1);
            break;
        case QTextListFormat::ListCircle:
            comboStyle->setCurrentIndex(2);
            break;
        case QTextListFormat::ListSquare:
            comboStyle->setCurrentIndex(3);
            break;
        case QTextListFormat::ListDecimal:
            comboStyle->setCurrentIndex(4);
            break;
        case QTextListFormat::ListLowerAlpha:
            comboStyle->setCurrentIndex(5);
            break;
        case QTextListFormat::ListUpperAlpha:
            comboStyle->setCurrentIndex(6);
            break;
        case QTextListFormat::ListLowerRoman:
            comboStyle->setCurrentIndex(7);
            break;
        case QTextListFormat::ListUpperRoman:
            comboStyle->setCurrentIndex(8);
            break;
        default:
            comboStyle->setCurrentIndex(-1);
            break;
        }
    } else {
        int headingLevel = textEdit->textCursor().blockFormat().headingLevel();
        comboStyle->setCurrentIndex(headingLevel ? headingLevel + 8 : 0);
    }
}

void TextEdit::clipboardDataChanged()
{
#ifndef QT_NO_CLIPBOARD
    if (const QMimeData *md = QApplication::clipboard()->mimeData())
        actionPaste->setEnabled(md->hasText());
#endif
}

void TextEdit::about()
{
    QMessageBox::about(this, tr("About"), tr("This example demonstrates Qt's "
        "rich text editing facilities in action, providing an example "
        "document for you to experiment with."));
}

void TextEdit::mergeFormatOnWordOrSelection(const QTextCharFormat &format)
{
    QTextCursor cursor = textEdit->textCursor();
    if (!cursor.hasSelection())
        cursor.select(QTextCursor::WordUnderCursor);
    cursor.mergeCharFormat(format);
    textEdit->mergeCurrentCharFormat(format);
}

void TextEdit::fontChanged(const QFont &f)
{
    comboFont->setCurrentIndex(comboFont->findText(QFontInfo(f).family()));
    comboSize->setCurrentIndex(comboSize->findText(QString::number(f.pointSize())));
    actionTextBold->setChecked(f.bold());
    actionTextItalic->setChecked(f.italic());
    actionTextUnderline->setChecked(f.underline());
}

void TextEdit::colorChanged(const QColor &c)
{
    QPixmap pix(16, 16);
    pix.fill(c);
    actionTextColor->setIcon(pix);
}

void TextEdit::alignmentChanged(Qt::Alignment a)
{
    if (a & Qt::AlignLeft)
        actionAlignLeft->setChecked(true);
    else if (a & Qt::AlignHCenter)
        actionAlignCenter->setChecked(true);
    else if (a & Qt::AlignRight)
        actionAlignRight->setChecked(true);
    else if (a & Qt::AlignJustify)
        actionAlignJustify->setChecked(true);
}

// ------------------------------------------------------------------------------------------------

/* MY ADDS START */

MyQTextEdit::MyQTextEdit(QWidget* p) : QTextEdit(p){

    tcpSocket = new QTcpSocket;

    in.setDevice(tcpSocket);
    in.setVersion(QDataStream::Qt_4_0);

    tcpSocket->connectToHost(QHostAddress::LocalHost, 40123);

    connect(tcpSocket, &QIODevice::readyRead, this, &MyQTextEdit::readMessage);

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    int op = 'l';
    out << op;

    QString username = "bella";
    QString password = "ciao";

    out << username;
    out << password;

    tcpSocket->write(block);

}

MyQTextEdit::~MyQTextEdit(){}                   // se tolgo questo non ho la vtable STUDIA!!!

/* qdatastream operators */
QDataStream &operator<<(QDataStream& out, const Symbol& sen){
    QVector<int> qvect;
    return out << sen.c << sen.count << sen.format << sen.siteid << qvect.fromStdVector(sen.fract);
}
QDataStream &operator>>(QDataStream& in, Symbol& rec){
    QVector<int> qvect;
    in >> rec.c >> rec.count >> rec.format >> rec.siteid >> qvect;
    rec.fract = qvect.toStdVector();
    return in;
}

QDataStream &operator<<(QDataStream& out, const Message& sen){
    return out << sen.totAdd << sen.totRem << sen.genFrom << sen.symToAdd << sen.symToRem;
}
QDataStream &operator>>(QDataStream& in, Message& rec){
    return in >> rec.totAdd >> rec.totRem >> rec.genFrom >> rec.symToAdd >> rec.symToRem;
}

//QDataStream &operator<<(QDataStream& out, const NotifyCursor& sen){
//    return out << sen.uid << sen.cursPos;
//}
//QDataStream &operator>>(QDataStream& in, NotifyCursor& rec){
//    return in >> rec.uid >> rec.cursPos;
//}

QDataStream &operator<<(QDataStream& out, const User& sen){
    return out << sen.uid << sen.icon << sen.nick << sen.color << sen.startCursor;
}
QDataStream &operator>>(QDataStream& in, User& rec){
    return in >> rec.uid >> rec.icon >> rec.nick >> rec.color >> rec.startCursor;
}
template<class T>
QDataStream &operator<<(QDataStream& stream, const std::vector<T>& val){
    stream << static_cast<quint32>(val.size());
    for(auto& singleVal : val)
        stream << singleVal;
    return stream;
}

template<class T>
QDataStream &operator>>(QDataStream& stream, std::vector<T>& val){
    quint32 vecSize;
    val.clear();
    stream >> vecSize;
    val.reserve(vecSize);
    T tempVal;
    while(vecSize--){
        stream >> tempVal;
        val.push_back(tempVal);
    }
    return stream;
}
/* -------------------- */

void MyQTextEdit::CatchChangeSignal(int pos, int rem, int add){

    QList<Symbol> _add = {};
    QList<Symbol> _rem = {};

    if(rem != 0){

        for(int i=0;i<rem;i++){

            if(_symbols.size() > 0) {
                _rem.append( _symbols.at(pos) );

                localErase(pos);
            }
            else if(add) add--;
        }
    }
    if(add != 0){

        auto supportCursor = QTextCursor(this->document());
        for(int i=0; i<add; i++){

            supportCursor.setPosition(pos+i);
            supportCursor.movePosition(QTextCursor::NextCharacter);

            localInsert(pos+i, document()->characterAt(pos+i), supportCursor.charFormat());

            _add.append( _symbols.at(pos+i) );
        }
    }

    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    qDebug("sending a message");
    out << 'm';
    out << Message(add, rem, _siteId, _add, _rem);

    tcpSocket->write(block);

    // debugging purpose
//    QString textA;
//    for(auto s : _add){
//        textA.append(s.c);
//    }

//    QString textR;
//    for(auto s : _rem){
//        textR.append(s.c);
//    }

//    QString fromT;
//    QString fromS;

//    for(auto s: _symbols)
//        fromS.append(s.c);

//    fromT = document()->toRawText();

//    if(fromT != fromS)
//        qDebug() << "DIVERGENZA!! testo: " << fromT << "e simboli: " << fromS;

}

// feature not required, look for other chunks to comment out
//void MyQTextEdit::myCursorPositionChanged(){

//    int mypos = textCursor().position();
//    NotifyCursor notify(mypos, _siteId);

//    QByteArray block;
//    QDataStream out(&block, QIODevice::WriteOnly);
//    out.setVersion(QDataStream::Qt_4_0);

//    int op = 'c';
//    out << op;

//    out << NotifyCursor(mypos, _siteId);
//    tcpSocket->write(block);

//}

std::vector<int> MyQTextEdit::prefix(std::vector<int> id, int depth, int substitute)
{
    std::vector<int> idCopy = {};
    for (int cpt = 0; cpt <= depth; cpt++) {
        if (cpt < id.size()) {
            idCopy.push_back(id.at(cpt));
        }
        else {
            idCopy.push_back(substitute);
        }
    }
    return idCopy;
}

void MyQTextEdit::localInsert(int index, QChar value, QTextCharFormat charFormat)
{
    std::vector<int> myfract = {};

    auto before = _symbols.size() > index-1 ? _symbols.at(index-1).fract : std::vector<int>();
    auto after = _symbols.size() > index ? _symbols.at(index).fract : std::vector<int>();

    int depth = 0;
    int interval = 0;

    while (interval < 1) {
        interval = prefix(after, depth, 1000).back() - prefix(before, depth, 0).back() - 1;
        depth++;
    }

    int step = std::min(2, interval);

    int addVal = QRandomGenerator::global()->bounded(0, step) + 1;
    myfract = prefix(before, depth-1, 0);

    myfract.back() += addVal;

    Symbol mysym{value, _siteId, _counter, myfract, charFormat};
    _symbols.insert(std::next(_symbols.begin(), index), mysym);

    _counter++;
}

void MyQTextEdit::localErase(int i) {

    _symbols.erase(_symbols.begin()+i);

}

void MyQTextEdit::paintEvent(QPaintEvent *event) {

    QTextEdit::paintEvent(event);

    for(auto u: _users){
        if(u.uid != _siteId){
            const QRect qRect = cursorRect(_cursors.find(u.uid).value());
            QPainter qPainter(viewport());
            qPainter.fillRect(qRect, u.color);
        }
    }

    QTextCursor cursor(document());

    int pos = 0;
    for(auto s : _symbols){
        cursor.setPosition(pos);

        // IGNORE symbol written by ME and SPECIAL ones, look Qchar doc for 13
        if(s.siteid!=_siteId && !s.c.isSpace() ) {         //old filter was "s.c.category() > 13"
            const QRect qRect1 = cursorRect(cursor);
            cursor.setPosition(pos+1, QTextCursor::KeepAnchor);
            const QRect qRect2 = cursorRect(cursor);

            // qt docs warns against using topleft and bottomright type of stuff
            QRect qRectSum(qRect1.topLeft(), qRect2.bottomRight());
            QPainter qPainter(viewport());
            qPainter.setCompositionMode(QPainter::CompositionMode_Darken); //rimane comunque una paraculata

            qPainter.fillRect(qRectSum, _users.find(s.siteid).value().color);

        }
        cursor.clearSelection();
        pos++;
    }

}

void MyQTextEdit::process(const User &u) {

    if(_users.contains(u.uid)) {
        _users.erase(_users.find(u.uid));
        _users.insert(u.uid, u);
    }
    else { 
        _users.insert(u.uid, u);
        if(u.uid!=_siteId){
            _cursors.insert(u.uid, QTextCursor(this->document()));
        }
    }

}

// useless feature
//void MyQTextEdit::process(const NotifyCursor &n) {

//    auto q = _cursors.find(n.uid);
//    q->setPosition(n.cursPos);

//}

void MyQTextEdit::process(const Message& m) {

    disconnect(document(), &QTextDocument::contentsChange,
            this, &MyQTextEdit::CatchChangeSignal);
//    disconnect(this, &QTextEdit::cursorPositionChanged,
//            this, &MyQTextEdit::myCursorPositionChanged);

    _cursors.find(m.genFrom)->beginEditBlock();

    //vars for dichotomy search
    int lowbound;
    int upbound;
    int index;


    /* let's look for syms to erase */
    for(auto mi = m.symToRem.begin(); mi != m.symToRem.end(); mi++){

        // dichotomy
        lowbound = 0;
        upbound = _symbols.size();

        while(lowbound < upbound){
            index = (upbound+lowbound) /2;
            auto curr = _symbols.at(index);

            /* to check if it's the sym I'm looking for siteid & count are enough */
            if (curr.siteid == mi->siteid && curr.count == mi->count) {

                _symbols.erase(_symbols.begin()+index);

                _cursors.find(m.genFrom)->setPosition(index);
                _cursors.find(m.genFrom)->deleteChar();

                break;
            }

            auto fract = curr.fract;
            int digit = 0;
            bool fract_over = false;

            while (fract.size() > digit && mi->fract.size() > digit)
            {
                if (fract.at(digit) > mi->fract.at(digit)) {
                    //go lower
                    upbound = index;
                    fract_over = true;
                    break;
                }
                if(fract.at(digit) < mi->fract.at(digit)){
                    //go upper
                    lowbound = index;
                    fract_over = true;
                    break;
                }
                digit++;
            }

            if(!fract_over){
                // until now vectors are equal
                // maybe one of the two vector continues
                if(fract.size() > digit) {
                    if(fract.at(digit) > 0){
                        //go lower
                        upbound = index;
                    }
                }
                else if (mi->fract.size() > digit) {
                    if(mi->fract.at(digit) == 0){
                        //go upper
                        lowbound = index;
                    }
                }
            }

        }


//        i = 0;
//        // classic version alternative to dich
//        for (auto it = _symbols.begin(); it != _symbols.end(); it++) {

//            /* to check if it's the sym I'm looking for siteid & count are enough */
//            if (it->siteid == mi->siteid && it->count == mi->count) {

//                _symbols.erase(it);

//                _cursors.find(m.genFrom)->setPosition(i);
//                _cursors.find(m.genFrom)->deleteChar();

//                break;
//            }


//            i++;
//        }



    }

    /* let's look for syms to add */
    for(auto mi = m.symToAdd.begin(); mi != m.symToAdd.end(); mi++){
        int i;
        bool found = false;


        for (i = 0; i < _symbols.size() && !found; i++) {
            bool next = false;
            auto curr = _symbols.at(i).fract;
            int digit = 0;

            while (!found && !next &&
                        curr.size() > digit && mi->fract.size() > digit)
            {
                if (curr.at(digit) > mi->fract.at(digit)) {
                    found = true;
                    i--;
                }
                if(curr.at(digit) < mi->fract.at(digit)){
                    next = true;
                }
                digit++;
            }

            // until now vectors are equal
            if(!found && !next) {
                // maybe one of the two vector continues
                if(curr.size() > digit) {
                    if(curr.at(digit) > 0){
                        found = true;
                        i--;
                    }
                }
                else if (mi->fract.size() > digit) {
                    if(mi->fract.at(digit) == 0){
                        found = true;
                        i--;
                    }
                }
                else {
                    // identical fract, then I look at _site Id than at count
                    if(_symbols.at(i).siteid > mi->siteid){
                        found = true;
                        i--;
                    }
                    else if(_symbols.at(i).siteid < mi->siteid){
                        found = true;
                    }
                    else {
                        if(_symbols.at(i).count > mi->count){
                            found = true;
                            i--;
                        }
                        else if(_symbols.at(i).count < mi->count){
                            found = true;
                        }
                        // I shouldn't be here, it's the same symbol!!!!
                    }
                }
            }
        }

        _symbols.insert(_symbols.begin() + i, *mi);

        _cursors.find(m.genFrom)->setPosition(i);
        _cursors.find(m.genFrom)->insertText(mi->c, mi->format);
    }

    _cursors.find(m.genFrom)->endEditBlock();

    connect(document(), &QTextDocument::contentsChange,
            this, &MyQTextEdit::CatchChangeSignal);
//    connect(this, &QTextEdit::cursorPositionChanged,
//            this, &MyQTextEdit::myCursorPositionChanged);

}

void MyQTextEdit::readMessage()
{
    Message msg;
    Symbol sym;
    User usr;
//    NotifyCursor nfy;

    quint32 uid;                // to be moved!!!!
    int magic;


    do {

        in.startTransaction();
        in >> magic;

        switch(magic){
        // cursor position update is now disabled, deemed useless
//        case 'c':
//            in >> nfy;
//            process(nfy);
//            break;
        case 'm':
            in.startTransaction();
            in >> msg;
            if(in.commitTransaction())
                process(msg);
            break;
        case 'u':
            in.startTransaction();
            in >> usr;
            if(in.commitTransaction())
                process(usr);
            break;
        case 'd':
            in.startTransaction();
            in >> uid;
            if(in.commitTransaction())
                _users.remove(uid);
            break;
        case 's':               // these are all to be moved in the login/file stage!!!!!
        case 'l':               // these are RESPONSES to login or signup attempts
            in.startTransaction();
            in >> uid;
            if(in.commitTransaction()){
                if(uid !=0 ){
                    _siteId = uid;
                    in >> _files;

                    fakeOpenFile();    // WARNING TO BE REMOVED

                }
                else {
                    qDebug() << "operation '" << char(magic) << "' failed";

                    // fail to login/signup
                    // wrong user/pwd combo in case of login
                    // username already present in signup case
                }
            }
            break;
        case 't':
            in.startTransaction();
            in >> _symbols;
            qDebug("Received symbols, CatchChangeSignal not connected yet");
            if(in.commitTransaction()){
                insertSymbols();
                qDebug() << "Connecting CatchChangeSignal...";
                connect(document(), &QTextDocument::contentsChange,
                    this, &MyQTextEdit::CatchChangeSignal);
            }
            break;
        }
    } while(in.commitTransaction());

}

void MyQTextEdit::insertSymbols(){
    QTextCursor init(document());

    init.beginEditBlock();
    for(auto it = _symbols.begin(); it!=_symbols.end(); it++){
        init.insertText(it->c, it->format);
    }
    init.endEditBlock();
}


/* start of fake functions */
void MyQTextEdit::fakeNewFile(){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    int op = 'n';
    out << op;

    QString filename("fakedoc");

    out << filename;
    tcpSocket->write(block);

}

void MyQTextEdit::fakeOpenFile(){
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_0);

    int op = 'o';
    out << op;

    out << _files.last();
    tcpSocket->write(block);

}
/* end of mockup stuff */
