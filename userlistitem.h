#ifndef USERLISTITEM_H
#define USERLISTITEM_H

#include <QObject>
#include <QWidget>
#include <QLabel>
#include <QPushButton>

class UserListItem : public QWidget
{
    Q_OBJECT
public:
    UserListItem(const QString& text);

private:
    QLabel*      _nameLabel  = nullptr;
    QPushButton* _editButton = nullptr;

signals:

};

#endif // USERLISTITEM_H
