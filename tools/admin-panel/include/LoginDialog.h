//------------------------------------------------------------------------------
//
//  Login dialog
//  (c) SimulatorClient 2026
//
//------------------------------------------------------------------------------

#ifndef     LOGINDIALOG_H
#define     LOGINDIALOG_H

#include    <QDialog>
#include    <QLineEdit>

//-----------------------------------------------------------------------------
class LoginDialog : public QDialog
{
    Q_OBJECT

public:

    explicit LoginDialog(QWidget* parent = nullptr);
    ~LoginDialog();

    QString getUsername() const;
    QString getPassword() const;

    static bool getCredentials(QWidget* parent, QString& username, QString& password);

private:

    QLineEdit*  m_editUsername;
    QLineEdit*  m_editPassword;
};

#endif // LOGINDIALOG_H
