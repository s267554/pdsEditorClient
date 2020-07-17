#include "profiledialog.h"
#include <QLabel>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QDialogButtonBox>

ProfileDialog::ProfileDialog(QWidget *parent, User* user) : QDialog(parent)
{
    auto layout = new QGridLayout(this);
    this->user = user;

    nickEdit = new QLineEdit(user->nick);
    auto nickLabel = new QLabel(tr("User nick:"));
    nickLabel->setBuddy(nickEdit);

    layout->addWidget(nickLabel);
    layout->addWidget(nickEdit);

    auto quitButton = new QPushButton(tr("Cancel"));

    acceptButton = new QPushButton(tr("Ok"));

    auto buttonBox = new QDialogButtonBox;
    buttonBox->addButton(acceptButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(quitButton, QDialogButtonBox::RejectRole);

    layout->addWidget(buttonBox);

    connect(acceptButton, &QAbstractButton::clicked,
            this, &ProfileDialog::changesAccepted);
    connect(quitButton, &QAbstractButton::clicked, this, &QWidget::close);

}

void ProfileDialog::changesAccepted()
{
    user->nick = nickEdit->text();
    this->done(Accepted);
}
