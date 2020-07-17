#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include "user.h"

class UserListItem : public QWidget
{
    Q_OBJECT
public:
    UserListItem(const QString& text);
    UserListItem(const User&);
    User userModel;

public slots:
    void setColor();
private:
    QLabel*      _nameLabel  = nullptr;
    QPushButton* _editButton = nullptr;

signals:
    void colorSelected(quint32, QColor);

};

#endif // USERLISTITEM_H
