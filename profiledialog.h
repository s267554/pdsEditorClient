#ifndef PROFILEDIALOG_H
#define PROFILEDIALOG_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include "user.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QLabel;
class QLineEdit;
QT_END_NAMESPACE

class ProfileDialog : public QDialog
{
    Q_OBJECT
public:
    explicit ProfileDialog(QWidget *parent = nullptr, User* = nullptr);
private:
    QLineEdit *nickEdit = nullptr;
    QPushButton *acceptButton = nullptr;
    User* user = nullptr;
private slots:
    void changesAccepted();
};

#endif // PROFILEDIALOG_H
