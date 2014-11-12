/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   DexaClean.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Application.h>
#include <Configuration.h>
#include <Exam.h>
#include <Interview.h>
#include <User.h>

#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <stdexcept>

#include <termios.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <fstream>

bool ApplicationInit();

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int getch()
{
  int ch;
  struct termios t_old, t_new;

  tcgetattr( STDIN_FILENO, &t_old );
  t_new = t_old;
  t_new.c_lflag &= ~(ICANON | ECHO);
  tcsetattr( STDIN_FILENO, TCSANOW, &t_new );

  ch = getchar();

  tcsetattr( STDIN_FILENO, TCSANOW, &t_old );
  return ch;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
std::string getpass( const char *prompt, bool show_asterisk = true )
{
  const char BACKSPACE = 127;
  const char RETURN = 10;

  std::string password;
  unsigned char ch = 0;

  std::cout << prompt << std::endl;

  while( ( ch = getch() ) != RETURN )
  {
    if( ch == BACKSPACE )
    {
      if( !password.empty() )
      {
        if( show_asterisk )
          std::cout << "\b \b";
        password.resize( password.size() - 1 );
      }
    }
    else
    {
      password += ch;
      if( show_asterisk )
        std::cout <<'*';
    }
  }
  std::cout << std::endl;
  return password;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int main( int argc, char** argv )
{
  bool query_site = false;
  bool query_uid = false;
  bool query_uid_list = false;
  std::string site_str = "";
  std::string uid_str = "";
  std::string uid_file_name = "";
  if( argc > 1 )
  {
    int i = 1;
    do{
      if( std::string( argv[i] ) == "-l" && i+1 < argc)
      {
        query_uid_list = true;
        uid_file_name = argv[i+1];
      }
      else if( std::string( argv[i] ) == "-u" && i+1 < argc)
      {
        query_uid = true;
        uid_str = argv[i+1];
      }
      else if( std::string( argv[i] ) == "-s" && i+1 < argc)
      {
        query_site = true;
        site_str = argv[i+1];
        site_str.erase(
          remove( site_str.begin(), site_str.end(), '\"' ), site_str.end() );
      }
      i+=2;
    } while( i+1 < argc );
  }

  // stand alone console application that makes use of the
  // Alder business logic

  if( !ApplicationInit() )
  {
    return EXIT_FAILURE;
  }

  vtkNew< Alder::User > user;
  user->Load( "Name", "administrator" );
  std::string pwd = getpass( "Please enter the Alder admin password: ", true );
  if( !user->IsPassword( pwd ) )
  {
    std::cout << "Error: wrong administrator password" << std::endl;
    Alder::Application::DeleteInstance();
    return EXIT_FAILURE;
  }

  Alder::Application *app = Alder::Application::GetInstance();
  app->SetupOpalService();

  // search the Alder db for dexa exams of type Hip, Forearm and WholeBody
  // process the images report which ones had the dicom Patient Name tag populated

  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;

  if( query_uid || query_site || query_uid_list )
  {
    vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();

    if( query_uid )
    {
      modifier->Where( "UId", "=", vtkVariant( uid_str ) );
    }
    if( query_uid_list )
    {
      ifstream infile( uid_file_name.c_str() );
      std::string uid_list_str = "";
      while( infile )
      {
        std::string line;
        if( !getline( infile, line ) ) break;
        std::istringstream str_str( line );
        while( str_str )
        {
          std::string str;
          if( !getline( str_str, str, ',' ) ) break;
          str.erase( remove_if( str.begin(), str.end(), ::isspace ), str.end() );
          uid_list_str += '"' + str + '"';
          uid_list_str += ',';
        }
      }
      uid_list_str.erase( uid_list_str.end() - 1 );
      modifier->Where( "UId", "IN", vtkVariant( uid_list_str ), false );
    }
    if( query_site )
    {
      modifier->Where( "Site", "=", vtkVariant( site_str ) );
    }

    std::cout << modifier->GetSql() << std::endl;

    Alder::Interview::GetAll( &interviewList, modifier );
  }
  else
  {
    Alder::Interview::GetAll( &interviewList );
  }

  if( !interviewList.empty() )
  {
    int i = 1;
    int count = interviewList.size();
    for( auto interviewIt = interviewList.begin(); interviewIt != interviewList.end(); ++interviewIt )
    {
      Alder::Interview *interview = interviewIt->GetPointer();
      interview->UpdateExamData( true );
      std::cout << "-------------- PROCESSING INTERVIEW " << i++ << " of " << count << " --------------" << std::endl;
      std::cout << "UID: " << interview->Get( "UId" ) << std::endl;
      std::cout << "VisitDate: " << interview->Get( "VisitDate" ) << std::endl;
      std::cout << "Site: " << interview->Get( "Site" ) << std::endl;
    }
  }
  else
  {
    std::cout << "No interviews found" << std::endl;
  }

  Alder::Application::DeleteInstance();
  return EXIT_SUCCESS;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool ApplicationInit()
{
  bool status = true;
  Alder::Application *app = Alder::Application::GetInstance();
  std::cout << "Initializing application using file: " << ALDER_CONFIG_FILE << std::endl;
  if( !app->ReadConfiguration( ALDER_CONFIG_FILE ) )
  {
    std::cout << "ERROR: error while reading configuration file \""
              << ALDER_CONFIG_FILE << "\"" << std::endl;
    status = false;
  }
  else if( !app->OpenLogFile() )
  {
    std::string logPath = app->GetConfig()->GetValue( "Path", "Log" );
    std::cout << "ERROR: unable to open log file \""
              << logPath << "\"" << std::endl;
    status = false;
  }
  else if( !app->TestImageDataPath() )
  {
    std::string imageDataPath = app->GetConfig()->GetValue( "Path", "ImageData" );
    std::cout << "ERROR: no write access to image data directory \""
              << imageDataPath << "\"" << std::endl;
    status = false;
  }
  else if( !app->ConnectToDatabase() )
  {
    std::cout << "ERROR: error while connecting to the database" << std::endl;
    status = false;
  }
  if( !status )
  {
    Alder::Application::DeleteInstance();
  }
  return status;
}
