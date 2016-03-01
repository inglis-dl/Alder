/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QMainAlderWindow.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QMainAlderWindow.h>
#include <ui_QMainAlderWindow.h>

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
QMainAlderWindow::QMainAlderWindow( QWidget* parent )
  : QMainWindow( parent )
{
  this->ui = new Ui_QMainAlderWindow;
  this->ui->setupUi( this );

  // connect the menu items
  QObject::connect(
    this->ui->actionOpenInterview, SIGNAL( triggered() ),
    this, SLOT( slotOpenInterview() ) );
  QObject::connect(
    this->ui->actionShowAtlas, SIGNAL( triggered() ),
    this, SLOT( slotShowAtlas() ) );
  QObject::connect(
    this->ui->actionShowDicomTags, SIGNAL( triggered() ),
    this, SLOT( slotShowDicomTags() ) );
  QObject::connect(
    this->ui->actionLogin, SIGNAL( triggered() ),
    this, SLOT( slotLogin() ) );
  QObject::connect(
    this->ui->actionChangePassword, SIGNAL( triggered() ),
    this, SLOT( slotChangePassword() ) );
  QObject::connect(
    this->ui->actionUserManagement, SIGNAL( triggered() ),
    this, SLOT( slotUserManagement() ) );
  QObject::connect(
    this->ui->actionUpdateDatabase, SIGNAL( triggered() ),
    this, SLOT( slotUpdateDatabase() ) );
  QObject::connect(
    this->ui->actionReports, SIGNAL( triggered() ),
    this, SLOT( slotReports() ) );
  QObject::connect(
    this->ui->actionRatingCodes, SIGNAL( triggered() ),
    this, SLOT( slotRatingCodes() ) );
  QObject::connect(
    this->ui->actionLoadUIDs, SIGNAL( triggered() ),
    this, SLOT( slotLoadUIDs() ) );
  QObject::connect(
    this->ui->actionExit, SIGNAL( triggered() ),
    qApp, SLOT( closeAllWindows() ) );
  QObject::connect(
    this->ui->actionSaveImage, SIGNAL( triggered() ),
    this, SLOT( slotSaveImage() ) );

  // connect the help menu items
  QObject::connect(
    this->ui->actionAbout, SIGNAL( triggered() ),
    this, SLOT( slotAbout() ) );
  QObject::connect(
    this->ui->actionManual, SIGNAL( triggered() ),
    this, SLOT( slotManual() ) );

  QObject::connect(
    this->ui->atlasWidget, SIGNAL( showing( bool ) ),
    this->ui->interviewWidget, SLOT( slotHideControls( bool ) ) );

  this->readSettings();

  QIcon icon(":/icons/alder32x32.png");
  QApplication::setWindowIcon(icon);

  // toggle visibility of the atlas widget
  this->atlasVisible = true;
  this->slotShowAtlas();

  this->DicomTagWidget = new QAlderDicomTagWidget( this );
  this->Connections = vtkSmartPointer<vtkEventQtSlotConnect>::New();
  this->Connections->Connect( Alder::Application::GetInstance(),
    Alder::Common::ActiveImageEvent,
    this, SLOT( updateDicomTagWidget() ) );

  this->dicomTagsVisible = false;
  this->DicomTagWidget->hide();

  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
QMainAlderWindow::~QMainAlderWindow()
{
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::closeEvent( QCloseEvent *event )
{
  //this->ui->framePlayerWidget->setViewer( 0 );
  this->writeSettings();
  event->accept();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotOpenInterview()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user = app->GetActiveUser();

  if( user )
  {
    QSelectInterviewDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Select Interview" ) );
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
void QMainAlderWindow::slotLogin()
{
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  if( loggedIn )
  {
    app->ResetApplication();
  }
  else
  {
    QLoginDialog dialog( this );
    dialog.setModal( true );
    dialog.setWindowTitle( tr( "Login" ) );
    dialog.exec();
  }

  // active user may have changed so update the interface
  this->updateInterface();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotShowAtlas()
{
  bool lastVisible = this->atlasVisible;

  if( !lastVisible && this->dicomTagsVisible ) this->slotShowDicomTags();

  this->atlasVisible = !this->atlasVisible;
  this->ui->actionShowAtlas->setText( tr( this->atlasVisible ? "Hide Atlas" : "Show Atlas" ) );

  if( this->atlasVisible )
  {
    // add the widget to the splitter
    this->ui->splitter->insertWidget( 0, this->ui->atlasWidget );
    this->ui->atlasWidget->show();

    QList<int> sizeList = this->ui->splitter->sizes();
    int total = sizeList[0] + sizeList[1];
    sizeList[0] = floor( total / 2 );
    sizeList[1] = sizeList[0];
    this->ui->splitter->setSizes( sizeList );
  }
  else
  {
    // remove the widget from the splitter
    this->ui->atlasWidget->hide();
    this->ui->atlasWidget->setParent( this );

    Alder::Application::GetInstance()->SetActiveAtlasImage( NULL );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotShowDicomTags()
{
  if( this->atlasVisible ) return;

  this->dicomTagsVisible = !this->dicomTagsVisible;

  this->ui->actionShowDicomTags->setText(
    tr( this->dicomTagsVisible ? "Hide Dicom Tags" : "Show Dicom Tags" ) );

  if( this->dicomTagsVisible )
  {
    this->ui->splitter->insertWidget( 0, qobject_cast<QWidget*>(this->DicomTagWidget));
    this->DicomTagWidget->show();
  }
  else
  {
    this->DicomTagWidget->hide();
    this->DicomTagWidget->setParent( this );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUserManagement()
{
  this->adminLoginDo( &QMainAlderWindow::adminUserManagement );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotUpdateDatabase()
{
  this->adminLoginDo( &QMainAlderWindow::adminUpdateDatabase );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotReports()
{
  this->adminLoginDo( &QMainAlderWindow::adminReports );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotRatingCodes()
{
  this->adminLoginDo( &QMainAlderWindow::adminRatingCodes );
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
      QObject::tr( "User Management" ),
      QObject::tr( attempt > 1 ? "Wrong password, try again:" : "Administrator password:" ),
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
void QMainAlderWindow::adminRatingCodes()
{
  // load the code dialog
  QCodeDialog codeDialog( this );
  codeDialog.setModal( true );
  codeDialog.setWindowTitle( tr( "Rating Codes" ) );
  codeDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::adminReports()
{
  // load the reports dialog
  QReportDialog reportDialog( this );
  reportDialog.setModal( true );
  reportDialog.setWindowTitle( tr( "Rating Reports" ) );
  reportDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotLoadUIDs()
{
  QFileDialog dialog(this, tr("Open File") );
  dialog.setNameFilter( tr("CSV files (*.csv)") );
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
    QMessageBox messageBox( this );
    messageBox.setWindowModality( Qt::WindowModal );
    messageBox.setIcon( QMessageBox::Warning );
    messageBox.setText( errorMsg );
    messageBox.exec();
  }
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
void QMainAlderWindow::adminUserManagement( )
{
  // load the users dialog
  QUserListDialog usersDialog( this );
  usersDialog.setModal( true );
  usersDialog.setWindowTitle( tr( "User Management" ) );

  Alder::Application *app = Alder::Application::GetInstance();
  if(  NULL != app->GetActiveUser() )
  {
    QObject::connect(
      &usersDialog, SIGNAL( userModalityChanged() ),
     this->ui->interviewWidget, SLOT( updateExamTreeWidget() ));
  }
  usersDialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotChangePassword()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::User* user;
  if(  NULL != ( user = app->GetActiveUser() ) )
  {
    QString password = user->Get("Password").ToString().c_str();
    QChangePasswordDialog dialog( this, password );
    dialog.setModal( true );
    QObject::connect(
      &dialog, SIGNAL( passwordChanged( QString ) ),
     this, SLOT( changeActiveUserPassword( QString ) ));
    dialog.exec();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::changeActiveUserPassword( QString password )
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
void QMainAlderWindow::slotAbout()
{
  QAboutDialog dialog( this );
  dialog.setModal( true );
  dialog.setWindowTitle( tr( "About Alder" ) );
  dialog.exec();
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotManual()
{
  // TODO: open link to Alder manual
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::slotSaveImage()
{
  QString fileName = QFileDialog::getSaveFileName( this,
    tr("Save Image to File"), this->lastSavePath,
    tr("Images (*.png *.pnm *.bmp *.jpg *.jpeg *.tif *.tiff)"));

  if( fileName.isEmpty() ) return;
  else
  {
    this->ui->interviewWidget->saveImage( fileName );
    this->lastSavePath = QFileInfo( fileName ).path();
  }
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
void QMainAlderWindow::updateInterface()
{
  Alder::Application *app = Alder::Application::GetInstance();
  bool loggedIn = NULL != app->GetActiveUser();

  // dynamic menu action names
  this->ui->actionLogin->setText( tr( loggedIn ? "Logout" : "Login" ) );

  // set all widget enable states
  this->ui->actionOpenInterview->setEnabled( loggedIn );
  this->ui->actionChangePassword->setEnabled( loggedIn );
  this->ui->actionShowAtlas->setEnabled( loggedIn );
  this->ui->actionSaveImage->setEnabled( loggedIn );
  this->ui->actionLoadUIDs->setEnabled( loggedIn );

  this->ui->splitter->setEnabled( loggedIn );

  this->ui->interviewWidget->setEnabled( loggedIn );
  this->ui->interviewWidget->updateEnabled();
  this->ui->atlasWidget->setEnabled( loggedIn );
  this->ui->atlasWidget->setEnabled( loggedIn );

  this->ui->actionShowDicomTags->setEnabled( loggedIn );
  this->DicomTagWidget->setEnabled( loggedIn );
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void QMainAlderWindow::updateDicomTagWidget()
{
  Alder::Application *app = Alder::Application::GetInstance();
  Alder::Image *image = app->GetActiveImage();
  QString fileName = image ? image->GetFileName().c_str() : "";
  this->DicomTagWidget->load( fileName );
}
