#include "profiledialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>

ProfileDialog::ProfileDialog(QWidget *parent, User* user) : QDialog(parent)
{
    auto layout = new QGridLayout(this);
    this->user = user;

    nickEdit = new QLineEdit(user->nick);
    auto nickLabel = new QLabel(tr("User nick:"));
    nickLabel->setBuddy(nickEdit);

    auto picButton = new QPushButton(tr("Select propic from file..."));

    layout->addWidget(nickLabel);
    layout->addWidget(nickEdit);
    layout->addWidget(picButton);

    auto quitButton = new QPushButton(tr("Cancel"));

    acceptButton = new QPushButton(tr("Ok"));

    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(acceptButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    layout->addWidget(buttonBox);

    connect(picButton, &QAbstractButton::clicked,
            this, &ProfileDialog::openImageFromFile);
    connect(acceptButton, &QAbstractButton::clicked,
            this, &ProfileDialog::changesAccepted);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

}

void ProfileDialog::changesAccepted()
{
    user->nick = nickEdit->text();
    user->icon = QImage(fileName).scaled(32, 32, Qt::IgnoreAspectRatio);
    this->done(Accepted);
}

void ProfileDialog::openImageFromFile()
{
    fileName = QFileDialog::getOpenFileName(this,
        tr("Open Image"), "/home/", tr("Image Files (*.png *.jpg *.bmp)"));
}
