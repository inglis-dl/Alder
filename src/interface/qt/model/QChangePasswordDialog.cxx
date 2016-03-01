/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QChangePasswordDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QChangePasswordDialog.h>
#include <ui_QChangePasswordDialog.h>

// Alder includes
#include <Utilities.h>

// Qt includes
#include <QKeyEvent>
#include <QPushButton>
#include <QMessageBox>

#include <iostream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
class QChangePasswordDialogPrivate : public Ui_QChangePasswordDialog
{
  Q_DECLARE_PUBLIC(QChangePasswordDialog);
protected:
  QChangePasswordDialog* const q_ptr;

public:
  explicit QChangePasswordDialogPrivate(QChangePasswordDialog& object, const QString& pwd = "");
  virtual ~QChangePasswordDialogPrivate();

  void init();
  void setupUi(QDialog*);
  void updateUi();
  bool validate();
  bool confirmChange();

private:
  QString originalPassword;
  QString newPassword;
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QChangePasswordDialogPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialogPrivate::QChangePasswordDialogPrivate
(QChangePasswordDialog& object, const QString& pwd)
  : q_ptr(&object), originalPassword( pwd )
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialogPrivate::~QChangePasswordDialogPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialogPrivate::init()
{
  Q_Q(QChangePasswordDialog);
  this->setupUi(q);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialogPrivate::setupUi( QDialog* widget )
{
  Q_Q(QChangePasswordDialog);

  this->Ui_QChangePasswordDialog::setupUi( widget );
  this->newPassword = "";
  QObject::connect(
    this->okButton, SIGNAL( clicked() ),
    q, SLOT( accepted() ) );

  QObject::connect(
    this->cancelButton, SIGNAL( clicked() ),
    q, SLOT( reject() ) );

  this->newPasswordLineEdit->installEventFilter( q );
  this->confirmPasswordLineEdit->installEventFilter( q );

  this->newPasswordLineEdit->setFocus( Qt::PopupFocusReason );
  this->newPasswordLineEdit->setEchoMode( QLineEdit::Password );
  this->confirmPasswordLineEdit->setEchoMode( QLineEdit::Password );
  this->confirmPasswordLineEdit->setDisabled( true );
  q->setWindowTitle( QString("Set Password") );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QChangePasswordDialogPrivate::validate()
{
  Q_Q(QChangePasswordDialog);
  QString password = this->newPasswordLineEdit->text();
  bool noError = true;
  QString errStr;
  if( password.isEmpty() )
  {
    errStr = "empty";
    noError = false;
  }
  else if( 6 > password.size() )
  {
    errStr = "too short";
    noError = false;
  }
  else if( "password" == password )
  {
    errStr = "illegal";
    noError = false;
  }
  else
  {
    std::string hashed;
    Alder::Utilities::hashString( password.toStdString(), hashed );

    if( this->originalPassword.toStdString() == hashed )
    {
      errStr = "no change";
      noError = false;
    }
  }
  if( !noError )
  {
    QMessageBox errorMessage( q );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    QString msg = "Ivalid password (";
    msg += errStr;
    msg += "), please try again.";
    errorMessage.setText( QDialog::tr( msg.toStdString().c_str() ) );
    errorMessage.exec();
  }
  return noError;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QChangePasswordDialogPrivate::confirmChange()
{
  Q_Q(QChangePasswordDialog);
  QString password1 = this->newPasswordLineEdit->text();
  QString password2 = this->confirmPasswordLineEdit->text();
  if( password1 != password2 )
  {
    QMessageBox errorMessage( q );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    errorMessage.setText( QDialog::tr( "Confirmation does not match password, please try again." ) );
    errorMessage.exec();
    this->confirmPasswordLineEdit->setFocus();
    this->newPassword = "";
    return false;
  }
  else
  {
    this->newPassword = password1;
    return true;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialog::QChangePasswordDialog( QWidget* parent, const QString& pwd )
  : Superclass( parent )
  , d_ptr(new QChangePasswordDialogPrivate(*this, pwd) )
{
  Q_D(QChangePasswordDialog);
  d->init();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QChangePasswordDialog::~QChangePasswordDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool QChangePasswordDialog::eventFilter( QObject* watched, QEvent* event )
{
  Q_D(QChangePasswordDialog);
  if( watched == d->newPasswordLineEdit )
  {
    if( event->type() == QEvent::KeyPress )
    {
      QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
      if( keyEvent->key() == Qt::Key_Return ||
          keyEvent->key() == Qt::Key_Enter ||
          keyEvent->key() == Qt::Key_Tab )
      {
        if( d->validate() )
        {
          d->confirmPasswordLineEdit->setDisabled( false );
          d->confirmPasswordLineEdit->setFocus();
          return true;
        }
        else
        {
          d->confirmPasswordLineEdit->setDisabled( true );
          return true;
        }
      }
      else
      {
        d->confirmPasswordLineEdit->setDisabled( true );
        return false;
      }
    }
    else if( event->type() == QEvent::FocusIn )
    {
      d->confirmPasswordLineEdit->setDisabled( true );
      return false;
    }
    else
    {
      return false;
    }
  }
  else if( watched == d->confirmPasswordLineEdit &&
           event->type() == QEvent::KeyPress )
  {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
    if( keyEvent->key() == Qt::Key_Return ||
        keyEvent->key() == Qt::Key_Enter ||
        keyEvent->key() == Qt::Key_Tab )
    {
      if( d->confirmChange() )
      {
        this->accepted();
      }
      return true;
    }
    return false;
  }
  else
    return false;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QChangePasswordDialog::accepted()
{
  Q_D(QChangePasswordDialog);
  if( !d->newPassword.isEmpty() )
  {
    emit passwordChanged( d->newPassword  );
  }
  this->accept();
}
