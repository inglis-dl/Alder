/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMainAlderWindow.h>
#include <QMainAlderWindow_p.h>

// Alder includes
#include <Application.h>
#include <Common.h>
#include <Interview.h>
#include <User.h>
#include <Utilities.h>

#include <QAboutDialog.h>
#include <QAlderDicomTagWidget.h>
#include <QChangePasswordDialog.h>
#include <QLoginDialog.h>
#include <QCodeDialog.h>
#include <QReportDialog.h>
#include <QSelectInterviewDialog.h>
#include <QSelectWaveDialog.h>
#include <QUserListDialog.h>
#include <QVTKProgressDialog.h>

// VTK includes
#include <vtkEventQtSlotConnect.h>
#include <vtkNew.h>

// Qt includes
#include <QCloseEvent>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>

#include <stdexcept>

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QMainAlderWindowPrivate methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindowPrivate::QMainAlderWindowPrivate(QMainAlderWindow& object)
  : QObject(&object), q_ptr(&object)
{
  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindowPrivate::~QMainAlderWindowPrivate()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::setupUi( QMainWindow* window )
{
  Q_Q(QMainAlderWindow);

  this->Ui_QMainAlderWindow::setupUi( window );

  // connect the menu items
  QObject::connect(
    this->actionOpenInterview, SIGNAL( triggered() ),
    this, SLOT( openInterview() ) );
  QObject::connect(
    this->actionShowAtlas, SIGNAL( triggered() ),
    this, SLOT( showAtlas() ) );
  QObject::connect(
    this->actionShowDicomTags, SIGNAL( triggered() ),
    this, SLOT( showDicomTags() ) );
  QObject::connect(
    this->actionLogin, SIGNAL( triggered() ),
    this, SLOT( login() ) );
  QObject::connect(
    this->actionChangePassword, SIGNAL( triggered() ),
    this, SLOT( changePassword() ) );
  QObject::connect(
    this->actionUserManagement, SIGNAL( triggered() ),
    this, SLOT( userManagement() ) );
  QObject::connect(
    this->actionUpdateDatabase, SIGNAL( triggered() ),
    this, SLOT( updateDatabase() ) );
  QObject::connect(
    this->actionReports, SIGNAL( triggered() ),
    this, SLOT( reports() ) );
  QObject::connect(
    this->actionRatingCodes, SIGNAL( triggered() ),
    this, SLOT( ratingCodes() ) );
  QObject::connect(
    this->actionLoadUIDs, SIGNAL( triggered() ),
    this, SLOT( loadUIDs() ) );
  QObject::connect(
    this->actionExit, SIGNAL( triggered() ),
    qApp, SLOT( closeAllWindows() ) );
  QObject::connect(
    this->actionSaveImage, SIGNAL( triggered() ),
    this, SLOT( saveImage() ) );

  // connect the help menu items
  QObject::connect(
    this->actionAbout, SIGNAL( triggered() ),
    this, SLOT( about() ) );
  QObject::connect(
    this->actionManual, SIGNAL( triggered() ),
    this, SLOT( manual() ) );

  QObject::connect(
    this->atlasWidget, SIGNAL( showing( bool ) ),
    this->interviewWidget, SLOT( hideControls( bool ) ) );

  QIcon icon(":/icons/alder32x32.png");
  QApplication::setWindowIcon(icon);

  this->qvtkConnection = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->qvtkConnection->Connect( Alder::Application::GetInstance(),
    Alder::Common::ActiveImageEvent,
    this, SLOT( updateDicomTagWidget() ) );

  this->atlasWidget->setParent( q );
  this->atlasVisible = false;
  this->atlasWidget->hide();
  this->dicomTagWidget->setParent( q );
  this->dicomTagsVisible = false;
  this->dicomTagWidget->hide();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::openInterview()
{
  Q_Q(QMainAlderWindow);
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user = app->GetActiveUser();
  if( user )
  {
    QSelectInterviewDialog dialog( q );
    dialog.setModal( true );
    dialog.setWindowTitle( QDialog::tr( "Select Interview" ) );
    dialog.exec();

    /*
    // THE DIALOG SHOULD HANDLE THE SELECTION AND UPDATING
    vtkSmartPointer< Alder::QueryModifier > modifier =
      vtkSmartPointer< Alder::QueryModifier >::New();
    user->InitializeExamModifier( modifier );

    // update the interview's exams and images
    Alder::Interview *activeInterview = app->GetActiveInterview();
    if( activeInterview && !activeInterview->HasImageData( modifier ) )
    {
      // create a progress dialog to observe the progress of the update
      QVTKProgressDialog dialog( this );
      Alder::SingleInterviewProgressFunc func( activeInterview );
      dialog.Run(
        "Downloading Exam Images",
        "Please wait while the interview's images are downloaded.",
        func );
      app->InvokeEvent( Alder::Common::ActiveInterviewUpdateImageDataEvent );
    }
    */
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::login()
{
  Q_Q(QMainAlderWindow);
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();
  if( loggedIn )
  {
    app->ResetApplication();
  }
  else
  {
    QLoginDialog dialog( q );
    dialog.setModal( true );
    dialog.setWindowTitle( QDialog::tr( "Login" ) );
    dialog.exec();
  }

  // active user may have changed so update the interface
  this->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::showAtlas()
{
  Q_Q(QMainAlderWindow);
  bool lastVisible = this->atlasVisible;

  if( !lastVisible && this->dicomTagsVisible ) this->showDicomTags();

  this->atlasVisible = !this->atlasVisible;
  this->actionShowAtlas->setText(( this->atlasVisible ? "Hide Atlas" : "Show Atlas" ));

  if( this->atlasVisible )
  {
    // add the widget to the splitter
    this->splitter->insertWidget( 0, this->atlasWidget );
    this->atlasWidget->show();

    QList<int> sizeList = this->splitter->sizes();
    int total = sizeList[0] + sizeList[1];
    sizeList[0] = floor( total / 2 );
    sizeList[1] = sizeList[0];
    this->splitter->setSizes( sizeList );
  }
  else
  {
    // remove the widget from the splitter
    this->atlasWidget->hide();
    this->atlasWidget->setParent( q );
    Alder::Application::GetInstance()->SetActiveAtlasImage( NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::showDicomTags()
{
  Q_Q(QMainAlderWindow);
  if( this->atlasVisible ) return;

  this->dicomTagsVisible = !this->dicomTagsVisible;

  this->actionShowDicomTags->setText(
    ( this->dicomTagsVisible ? "Hide Dicom Tags" : "Show Dicom Tags" ));

  if( this->dicomTagsVisible )
  {
    this->splitter->insertWidget( 0, this->dicomTagWidget );
    this->dicomTagWidget->show();
  }
  else
  {
    this->dicomTagWidget->hide();
    this->dicomTagWidget->setParent( q );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::userManagement()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo( &QMainAlderWindow::adminUserManagement );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateDatabase()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo( &QMainAlderWindow::adminUpdateDatabase );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::reports()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo( &QMainAlderWindow::adminReports );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::ratingCodes()
{
  Q_Q(QMainAlderWindow);
  q->adminLoginDo( &QMainAlderWindow::adminRatingCodes );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::loadUIDs()
{
  Q_Q(QMainAlderWindow);
  QFileDialog dialog( q, QDialog::tr("Open File") );
  dialog.setNameFilter( QDialog::tr("CSV files (*.csv)") );
  dialog.setFileMode( QFileDialog::ExistingFile );

  QStringList selectedFiles;
  if( dialog.exec() )
    selectedFiles = dialog.selectedFiles();
  if( selectedFiles.isEmpty() ) return;
  QString fileName = selectedFiles.front();

  bool error = false;
  QString errorMsg;

  QFile file(fileName);
  if( !file.open( QIODevice::ReadOnly|QIODevice::Text ) )
  {
    error = true;
    errorMsg = "Failed to open file";
  }
  else
  {
    // csv file must contain UId and Wave rank
    std::vector< std::pair< std::string, std::string > > list;
    QTextStream qstream( &file );
    std::string sep = "\",\"";
    while( !file.atEnd() )
    {
      QString line = qstream.readLine();
      std::string str = line.toStdString();
      str = Alder::Utilities::trim( str );
      str = Alder::Utilities::removeLeadingTrailing( str, '"' );
      std::vector< std::string > parts = Alder::Utilities::explode( str, sep );
      if( 2 == parts.size() )
      {
        list.push_back(
          std::make_pair(
            Alder::Utilities::trim( parts[0] ),
            Alder::Utilities::trim( parts[1] ) ) );
      }
    }
    file.close();
    if( list.empty() )
    {
      error = true;
      errorMsg  = "Failed to parse identifiers from csv file: ";
      errorMsg += "expecting UId / wave rank pairs";
    }
    else
    {
      // create a progress dialog to observe the progress of the update
      /*
      QVTKProgressDialog dialog( this );
      Alder::ListInterviewProgressFunc func( uidList );
      dialog.Run(
        "Downloading Images",
        "Please wait while the images are downloaded.",
        func );

      int numLoadedUIDs = func.GetNumLoaded();
      */
      int numLoaded = Alder::Interview::LoadFromList( list );

      Alder::Application *app = Alder::Application::GetInstance();
      std::stringstream log;
      log << "Loaded "
          << numLoaded
          << " of "
          << list.size()
          << " requested identifiers from file "
          << fileName.toStdString();
      app->Log( log.str() );
      if( numLoaded != list.size() )
      {
        error = true;
        errorMsg = log.str().c_str();
      }
    }
  }

  if( error )
  {
    QMessageBox messageBox( q );
    messageBox.setWindowModality( Qt::WindowModal );
    messageBox.setIcon( QMessageBox::Warning );
    messageBox.setText( errorMsg );
    messageBox.exec();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::changePassword()
{
  Q_Q(QMainAlderWindow);
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user;
  if(  NULL != ( user = app->GetActiveUser() ) )
  {
    QString password = user->Get("Password").ToString().c_str();
    QChangePasswordDialog dialog( q, password );
    dialog.setModal( true );
    QObject::connect(
      &dialog, SIGNAL( passwordChanged( QString ) ),
     this, SLOT( changeActiveUserPassword( QString ) ));
    dialog.exec();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::changeActiveUserPassword( QString password )
{
  if( password.isEmpty() ) return;
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user;
  if(  NULL != ( user = app->GetActiveUser() ) )
  {
    user->Set( "Password", vtkVariant( password.toStdString() ) );
    user->Save();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::about()
{
  Q_Q(QMainAlderWindow);
  QAboutDialog dialog( q );
  dialog.setModal( true );
  dialog.setWindowTitle( QDialog::tr( "About Alder" ) );
  dialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::manual()
{
  // TODO: open link to Alder manual
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::saveImage()
{
  Q_Q(QMainAlderWindow);
  QString fileName = QFileDialog::getSaveFileName( q,
    QDialog::tr("Save Image to File"), this->lastSavePath,
    QDialog::tr("Images (*.png *.pnm *.bmp *.jpg *.jpeg *.tif *.tiff)"));

  if( fileName.isEmpty() ) return;
  else
  {
    this->interviewWidget->saveImage( fileName );
    this->lastSavePath = QFileInfo( fileName ).path();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateUi()
{
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->actionLogin->setText((loggedIn ? "Logout" : "Login"));

  // set all widget enable states
  this->actionOpenInterview->setEnabled( loggedIn );
  this->actionChangePassword->setEnabled( loggedIn );
  this->actionShowAtlas->setEnabled( loggedIn );
  this->actionShowDicomTags->setEnabled( loggedIn );
  this->actionSaveImage->setEnabled( loggedIn );
  this->actionLoadUIDs->setEnabled( loggedIn );

  this->splitter->setEnabled( loggedIn );

  this->interviewWidget->setEnabled( loggedIn );
  this->atlasWidget->setEnabled( loggedIn );
  this->dicomTagWidget->setEnabled( loggedIn );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindowPrivate::updateDicomTagWidget()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();
  QString fileName = image ? image->GetFileName().c_str() : "";
  this->dicomTagWidget->load( fileName );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
//
// QMainAlderWindow methods
//
//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::QMainAlderWindow( QWidget* parent )
  : Superclass( parent )
  , d_ptr(new QMainAlderWindowPrivate(*this))
{
  Q_D(QMainAlderWindow);
  d->setupUi(this);
  this->readSettings();
  d->updateUi();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::closeEvent( QCloseEvent *event )
{
  this->writeSettings();
  event->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::readSettings()
{
  QSettings settings( "CLSA", "Alder" );
  settings.beginGroup( "MainAlderWindow" );
  if( settings.contains( "size" ) ) this->resize( settings.value( "size" ).toSize() );
  if( settings.contains( "pos" ) ) this->move( settings.value( "pos" ).toPoint() );
  if( settings.contains( "maximized" ) && settings.value( "maximized" ).toBool() )
    this->showMaximized();
  settings.endGroup();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::writeSettings()
{
  QSettings settings( "CLSA", "Alder" );
  settings.beginGroup( "MainAlderWindow" );
  settings.setValue( "size", this->size() );
  settings.setValue( "pos", this->pos() );
  settings.setValue( "maximized", this->isMaximized() );
  settings.endGroup();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminLoginDo( void (QMainAlderWindow::*fn)() )
{
  int attempt = 1;
  while( attempt < 4 )
  {
    // check for admin password
    QString text = QInputDialog::getText(
      this,
      QDialog::tr( "User Management" ),
      QDialog::tr( attempt > 1 ? "Wrong password, try again:" : "Administrator password:" ),
      QLineEdit::Password );

    // do nothing if the user hit the cancel button
    if( text.isEmpty() ) break;

    vtkNew< Alder::User > user;
    user->Load( "Name", "administrator" );
    if( user->IsPassword( text.toStdString().c_str() ) )
    {
      user->Set( "LastLogin", Alder::Utilities::getTime( "%Y-%m-%d %H:%M:%S" ) );
      user->Save();
      (this->*fn)();
      break;
    }
    attempt++;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUserManagement( )
{
  Q_D(QMainAlderWindow);
  // load the users dialog
  QUserListDialog usersDialog( this );
  usersDialog.setModal( true );
  usersDialog.setWindowTitle( QDialog::tr( "User Management" ) );

  Alder::Application *app = Alder::Application::GetInstance();
  if(  NULL != app->GetActiveUser() )
  {
    QObject::connect(
      &usersDialog, SIGNAL( userModalityChanged() ),
     d->interviewWidget, SLOT( activeInterviewChanged() ));
  }
  usersDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminUpdateDatabase()
{
  QSelectWaveDialog waveDialog( this );
  waveDialog.setModal( true );
  waveDialog.exec();
  std::vector< std::pair< int, bool > > waveVec = waveDialog.selection();
  if( !waveVec.empty() )
  {
    Alder::Interview::UpdateInterviewData( waveVec );
  }
  // create a progress dialog to observe the progress of the update
  /*
  QVTKProgressDialog dialog( this );
  Alder::MultiInterviewProgressFunc func;
  dialog.Run(
    "Updating Database",
    "Please wait while the database is updated.",
    func );
  */
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminRatingCodes()
{
  // load the code dialog
  QCodeDialog codeDialog( this );
  codeDialog.setModal( true );
  codeDialog.setWindowTitle( QDialog::tr( "Rating Codes" ) );
  codeDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminReports()
{
  // load the reports dialog
  QReportDialog reportDialog( this );
  reportDialog.setModal( true );
  reportDialog.setWindowTitle( QDialog::tr( "Rating Reports" ) );
  reportDialog.exec();
}


