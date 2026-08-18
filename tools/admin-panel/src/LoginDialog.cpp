//------------------------------------------------------------------------------
//
//  Login dialog
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#include    "LoginDialog.h"

#include    <QVBoxLayout>
#include    <QHBoxLayout>
#include    <QFormLayout>
#include    <QPushButton>

//-----------------------------------------------------------------------------
LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
    , m_editUsername(new QLineEdit(this))
    , m_editPassword(new QLineEdit(this))
{
    setWindowTitle("Authorization");
    setModal(true);
    setMinimumWidth(320);

    m_editUsername->setPlaceholderText("Enter username");
    m_editUsername->setMaxLength(64);

    m_editPassword->setEchoMode(QLineEdit::Password);
    m_editPassword->setPlaceholderText("Enter password");
    m_editPassword->setMaxLength(64);

    auto* formLayout = new QFormLayout;
    formLayout->addRow("Username:", m_editUsername);
    formLayout->addRow("Password:", m_editPassword);

    QPushButton* btnOk = new QPushButton("OK", this);
    QPushButton* btnCancel = new QPushButton("Cancel", this);

    connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

    auto* buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnCancel);
    buttonLayout->addWidget(btnOk);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

//-----------------------------------------------------------------------------
LoginDialog::~LoginDialog()
{
}

//-----------------------------------------------------------------------------
QString LoginDialog::getUsername() const
{
    return m_editUsername->text().trimmed();
}

//-----------------------------------------------------------------------------
QString LoginDialog::getPassword() const
{
    return m_editPassword->text();
}

//-----------------------------------------------------------------------------
bool LoginDialog::getCredentials(QWidget* parent, QString& username, QString& password)
{
    LoginDialog dialog(parent);

    // Наследуем стиль основного окна
    dialog.setStyleSheet(parent ? parent->styleSheet() : QString());

    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }

    username = dialog.getUsername();
    password = dialog.getPassword();

    return true;
}
