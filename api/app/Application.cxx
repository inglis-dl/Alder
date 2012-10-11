/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Application.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#include "Application.h"

#include "Configuration.h"
#include "Database.h"

#include "vtkObjectFactory.h"
#include "vtkVariant.h"
#include "vtkView.h"

namespace Alder
{
  Application* Application::Instance = NULL; // set the initial application

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::Application()
  {
    this->View = vtkView::New();
    this->Config = Configuration::New();
    this->DB = Database::New();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::~Application()
  {
    if( NULL != this->View )
    {
      this->View->Delete();
      this->View = NULL;
    }

    if( NULL != this->Config )
    {
      this->Config->Delete();
      this->Config = NULL;
    }

    if( NULL != this->DB )
    {
      this->DB->Delete();
      this->DB = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  // this must be implemented instead of the standard new macro since the New()
  // method is protected (ie: we do not want an instantiator new function)
  Application* Application::New()
  {
    vtkObject* ret = vtkObjectFactory::CreateInstance( "Application" );
    return ret ? static_cast< Application* >( ret ) : new Application;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application* Application::GetInstance()
  {
    if( NULL == Application::Instance ) Application::Instance = Application::New();
    return Application::Instance;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::DeleteInstance()
  {
    if( NULL != Application::Instance )
    {
      Application::Instance->Delete();
      Application::Instance = NULL;
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ReadConfiguration( std::string filename )
  {
    // make sure the file exists
    ifstream ifile( filename.c_str() );
    if( !ifile ) return false;
    return this->Config->Read( filename );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ConnectToDatabase()
  {
    std::string name = this->Config->GetValue( "Database", "Name" );
    std::string user = this->Config->GetValue( "Database", "Username" );
    std::string pass = this->Config->GetValue( "Database", "Password" );
    std::string host = this->Config->GetValue( "Database", "Server" );
    std::string port = this->Config->GetValue( "Database", "Port" );

    // make sure the database name, user and password are provided
    if( 0 == name.length() || 0 == user.length() || 0 == pass.length() )
    {
      cout << "ERROR: database name, user name and password must be included in "
           << "configuration file" << endl;
      return false;
    }

    // defaint host and port
    if( 0 == host.length() ) host = "localhost";
    if( 0 == port.length() ) port = "3306";

    return this->DB->Connect( name, user, pass, host, vtkVariant( port ).ToInt() );
  }
  
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::HasAdministrator()
  {
    return this->DB->HasAdministrator();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::IsAdministratorPassword( std::string password )
  {
    return this->DB->IsAdministratorPassword( password );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetAdministratorPassword( std::string password )
  {
    this->DB->SetAdministratorPassword( password );
  }
}
