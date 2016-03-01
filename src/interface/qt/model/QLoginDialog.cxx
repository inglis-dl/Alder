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
class QLoginDialogPrivate : public Ui_QLoginDialog
{
  Q_DECLARE_PUBLIC(QLoginDialog);
protected:
  QLoginDialog* const q_ptr;

public:
  explicit QLoginDialogPrivate(QLoginDialog& object);
  virtual ~QLoginDialogPrivate();

  void init();
  void setupUi(QDialog*);
};

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QLoginDialogPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialogPrivate::QLoginDialogPrivate(QLoginDialog& object)
  : q_ptr(&object)
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialogPrivate::~QLoginDialogPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QLoginDialogPrivate::init()
{
  Q_Q(QLoginDialog);
  this->setupUi(q);
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QLoginDialogPrivate::setupUi( QDialog* widget )
{
  Q_Q(QLoginDialog);

  this->Ui_QLoginDialog::setupUi( widget );

  QObject::connect(
    this->buttonBox, SIGNAL( accepted() ),
    q, SLOT( accepted() ) );

  this->passwordLineEdit->setEchoMode( QLineEdit::Password );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::QLoginDialog( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QLoginDialogPrivate(*this))
{
  Q_D(QLoginDialog);
  d->init();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QLoginDialog::~QLoginDialog()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QLoginDialog::accepted()
{
  Q_D(QLoginDialog);
  QString password = d->passwordLineEdit->text();

  vtkSmartPointer< Alder::User > user = vtkSmartPointer< Alder::User >::New();
  if( user->Load( "Name", d->usernameLineEdit->text().toStdString() ) &&
      user->IsPassword( password.toStdString() ) )
  { // login successful
    // if the password matches the default password, force the user to change it
    QString defPassword = Alder::User::GetDefaultPassword().c_str();
    while( defPassword == password )
    {
      // prompt for new password
      QString password1 = QInputDialog::getText(
        this,
        QDialog::tr( "Change Password" ),
        QDialog::tr( "Please provide a new password (cannot be \"password\") for your account:" ),
        QLineEdit::Password );

      if( !password1.isEmpty() && password1 != defPassword )
      {
        // re-prompt to repeat password
        QString password2 = QInputDialog::getText(
          this,
          QDialog::tr( "Re-type Password" ),
          QDialog::tr( "Please verify your new password by typing it again:" ),
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
