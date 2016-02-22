/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QLoginDialog.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QLoginDialog.h>
#include <ui_QLoginDialog.h>

// Alder includes
#include <Application.h>
#include <Modality.h>
#include <User.h>
#include <Utilities.h>

// VTK includes
#include <vtkSmartPointer.h>

// Qt includes
#include <QFile>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::QLoginDialog( QWidget* parent )
  : QDialog( parent )
{
  this->ui = new Ui_QLoginDialog;
  this->ui->setupUi( this );

  QObject::connect(
    this->ui->buttonBox, SIGNAL( accepted() ),
    this, SLOT( slotAccepted() ) );

  this->ui->passwordLineEdit->setEchoMode( QLineEdit::Password );
  this->ui->usernameLineEdit->setFocus( Qt::PopupFocusReason ); // make sure username box is focused
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::~QLoginDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QLoginDialog::slotAccepted()
{
  QString password = this->ui->passwordLineEdit->text();

  vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
  if( user->Load( "Name", this->ui->usernameLineEdit->text().toStdString() ) &&
      user->IsPassword( password.toStdString() ) )
  { // login successful
    // if the password matches the default password, force the user to change it
    QString defPassword = Alder::User::GetDefaultPassword().c_str();
    while( defPassword == password )
    {
      // prompt for new password
      QString password1 = QInputDialog::getText(
        this,
        QObject::tr( "Change Password" ),
        QObject::tr( "Please provide a new password (cannot be \"password\") for your account:" ),
        QLineEdit::Password );

      if( !password1.isEmpty() && password1 != defPassword )
      {
        // re-prompt to repeat password
        QString password2 = QInputDialog::getText(
          this,
          QObject::tr( "Re-type Password" ),
          QObject::tr( "Please verify your new password by typing it again:" ),
          QLineEdit::Password );

        if( password1 == password2 )
        {
          // set the replacement password
          password = password1;
          user->Set( "Password", password.toStdString() );
          user->Save();
        }
      }
    }

    // warn the user if they do not have any modalitys assigned to them
    std::vector< vtkSmartPointer< Alder::Modality > > modalityList;
    user->GetList( &modalityList );
    if( modalityList.empty() )
    {
      QMessageBox errorMessage( this );
      errorMessage.setWindowModality( Qt::WindowModal );
      errorMessage.setIcon( QMessageBox::Warning );
      std::string str = "User: " +  user->Get( "Name").ToString();
      str += " has no modalities assigned.\n";
      str += "Please contact the system administrator for modality assignment.";
      errorMessage.setText( tr( str.c_str() ) );
      errorMessage.exec();
    }
    else
    {
      // log in the user and mark login time
      Alder::Application::GetInstance()->SetActiveUser( user );
      user->Set( "LastLogin", Alder::Utilities::getTime( "%Y-%m-%d %H:%M:%S" ) );
      user->Save();
    }
    this->accept();
  }
  else
  { // login failed
    QMessageBox errorMessage( this );
    errorMessage.setWindowModality( Qt::WindowModal );
    errorMessage.setIcon( QMessageBox::Warning );
    errorMessage.setText( tr( "Invalid username or password, please try again." ) );
    errorMessage.exec();
  }
}
