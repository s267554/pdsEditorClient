#include "userlistitem.h"
#include <QLayout>
#include <QColorDialog>

UserListItem::UserListItem(const User& user) : userModel(user)
{
    setToolTip("user");
    setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum));

    // Horizontal layout to contain the grabber and the rest of the UI.
    auto top_layout = new QHBoxLayout(this);
    top_layout->setMargin(0);
    setLayout(top_layout);

    // Layouts for the meaningful UI.
    auto container_layout = new QVBoxLayout;
    container_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    container_layout->setMargin(4);
    top_layout->addLayout(container_layout);

    auto name_layout = new QHBoxLayout;
    name_layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
    name_layout->setMargin(0);
    container_layout->addLayout(name_layout);

    // The UI.
    _nameLabel = new QLabel(userModel.nick);
    name_layout->addWidget(_nameLabel);

    _editButton = new QPushButton("Cambia colore");
    name_layout->addWidget(_editButton);

    connect(_editButton, &QAbstractButton::clicked, this, &UserListItem::setColor);

}

void UserListItem::setColor()
{
    const QColor color = QColorDialog::getColor(this->userModel.color, this);

    if (color.isValid()) {
        emit colorSelected(this->userModel.uid, color);
    }
}
