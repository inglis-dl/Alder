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

// Qt includes
#include <QMainAlderWindow.h>
#include <QAlderApplication.h>
#include <QInputDialog>
#include <QObject>
#include <QString>

#include <stdexcept>
#include <sstream>

using namespace Alder;

// main function
int main( int argc, char** argv )
{
  int status = EXIT_FAILURE;
  try
  {
    std::stringstream error;
    // start by reading the configuration, connecting to the database and setting up the Opal service
    Application *app = Application::GetInstance();
    if( !app->ReadConfiguration( ALDER_CONFIG_FILE ) )
    {
      error << "ERROR: error while reading configuration file \"" << ALDER_CONFIG_FILE << "\"";
      throw std::runtime_error( error.str() );
    }

    if( !app->OpenLogFile() )
    {
      std::string logPath = app->GetConfig()->GetValue( "Path", "Log" );
      error << "ERROR: unable to open log file \"" << logPath << "\"";
      throw std::runtime_error( error.str() );
    }

    if( !app->TestImageDataPath() )
    {
      std::string imageDataPath = app->GetConfig()->GetValue( "Path", "ImageData" );
      error << "ERROR: no write access to image data directory \"" << imageDataPath << "\"";
      throw std::runtime_error( error.str() );
    }

    if( !app->ConnectToDatabase() )
    {
      error << "ERROR: error while connecting to the database";
      throw std::runtime_error( error.str() );
    }

    app->SetupOpalService();
    app->UpdateDatabase();

    // now create the user interface
    QAlderApplication qapp( argc, argv );
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
