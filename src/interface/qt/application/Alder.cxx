/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Alder.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
//
// .SECTION Description
// The main function which launches the application.
//

// Alder includes
#include <Application.h>
#include <Configuration.h>
#include <User.h>
#include <Utilities.h>
#include <QMainAlderWindow.h>
#include <QAlderApplication.h>

// Qt includes
#include <QSplashScreen>
#include <QInputDialog>
#include <QObject>
#include <QPixmap>
#include <QString>
#include <QMutex>
#include <QWaitCondition>

#include <stdexcept>
#include <sstream>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  int status = EXIT_FAILURE;
  try
  {
    Qt::Alignment topMiddle = Qt::AlignHCenter | Qt::AlignTop;
    std::stringstream error;
    unsigned long msec = 200;
    QMutex dummy;
    QWaitCondition wait;
    QAlderApplication qapp( argc, argv );
    QSplashScreen splash(QPixmap(":/icons/alder_splash.png"));
    splash.show();
    splash.update();
    qapp.processEvents();
    splash.showMessage( QObject::tr("\nReading configuration..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    Application *app = Application::GetInstance();
    if( !app->ReadConfiguration( ALDER_CONFIG_FILE ) )
    {
      error << "ERROR: error while reading configuration file \"" << ALDER_CONFIG_FILE << "\"";
      throw std::runtime_error( error.str() );
    }

    splash.showMessage( QObject::tr("\nOpening log file..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    if( !app->OpenLogFile() )
    {
      std::string logPath = app->GetConfig()->GetValue( "Path", "Log" );
      error << "ERROR: unable to open log file \"" << logPath << "\"";
      throw std::runtime_error( error.str() );
    }

    splash.showMessage( QObject::tr("\nTesting image data path..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    if( !app->TestImageDataPath() )
    {
      std::string imageDataPath = app->GetConfig()->GetValue( "Path", "ImageData" );
      error << "ERROR: no write access to image data directory \"" << imageDataPath << "\"";
      throw std::runtime_error( error.str() );
    }

    splash.showMessage( QObject::tr("\nConnecting to database..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    if( !app->ConnectToDatabase() )
    {
      error << "ERROR: error while connecting to the database";
      throw std::runtime_error( error.str() );
    }

    splash.showMessage( QObject::tr("\nConnecting to Opal..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    app->SetupOpalService();

    splash.showMessage( QObject::tr("\nUpdating database..."), topMiddle, Qt::black );
    dummy.lock();
    wait.wait(&dummy,msec);
    dummy.unlock();

    app->UpdateDatabase();

    // now create the user interface
    QMainAlderWindow mainWindow;

    // check to see if an admin user exists, create if not
    vtkNew< User > user;
    if( !user->Load( "Name", "administrator" ) )
    {
      QString text = QInputDialog::getText(
        &mainWindow,
        QDialog::tr( "Administrator Password" ),
        QDialog::tr( "Please provide a password for the mandatory administrator account:" ),
        QLineEdit::Password );

      if( !text.isEmpty() )
      { // create an administrator with the new password
        user->Set( "Name", "administrator" );
        user->Set( "Password", text.toStdString().c_str() );
        user->Save();
      }
    }

    splash.showMessage( QObject::tr("\nLaunching Alder..."), topMiddle, Qt::black );
    splash.finish(&mainWindow);
    mainWindow.show();

    // execute the application, then delete the application
    int status = qapp.exec();
  }
  catch( std::exception &e )
  {
    cerr << "Exception: " << e.what() << endl;
    status = EXIT_FAILURE;
  }

  Application::DeleteInstance();
  // return the result of the executed application
  return status;
}
