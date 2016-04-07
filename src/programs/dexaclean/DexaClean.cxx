/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   DexaClean.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

// Alder includes
#include <AlderConfig.h>
#include <Application.h>
#include <Configuration.h>
#include <Exam.h>
#include <Interview.h>
#include <User.h>

// VTK includes
#include <vtkNew.h>
#include <vtkSmartPointer.h>

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>

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

  // search the Alder db for dexa exams of type Hip, Forearm and WholeBody
  // process the images report which ones had the dicom Patient Name tag populated

  std::vector< vtkSmartPointer< Alder::Interview > > interviewList;
  Alder::Interview::GetAll( &interviewList );
  if( !interviewList.empty() )
  {
    for( auto interviewIt = interviewList.begin(); interviewIt != interviewList.end(); ++interviewIt )
    {
      Alder::Interview *interview = interviewIt->GetPointer();
      std::vector< vtkSmartPointer< Alder::Exam > > examList;
      interview->GetList( &examList );

      std::string siteStr = interview->Get( "Site" ).ToString();

      for( auto examIt = examList.begin(); examIt != examList.end(); ++examIt )
      {
        Alder::Exam *exam = examIt->GetPointer();
        std::string modStr = exam->GetModalityName();

        if( modStr != "Dexa" ) continue;

        std::string typeStr = exam->GetScanType();

        std::vector< vtkSmartPointer< Alder::Image > > imageList;
        exam->GetList( &imageList );
        for( auto imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
        {
          Alder::Image *image = imageIt->GetPointer();

          //check and set the laterality correctly from the dicom tag
          image->SwapExamSideFromDICOM();
          std::string sideStr = exam->Get( "Side" ).ToString();

          std::string fileName = image->GetFileName();
          std::cout << "-------------- PROCESSING IMAGE --------------" << std::endl
           << "Path: "        << fileName << std::endl
           << "Interviewer: " << exam->Get( "Interviewer" ).ToString() << std::endl
           << "Site: "        << siteStr << std::endl
           << "Datetime: "    << exam->Get( "DatetimeAcquired" ).ToString() << std::endl
           << "Modality: "    << modStr << std::endl
           << "Type: "        << typeStr << std::endl
           << "Side: "        << sideStr << std::endl
           << "Downloaded: "  << exam->Get( "Downloaded" ).ToInt() << std::endl;

          //check the dicom tag for a patient name
          std::string nameStr = image->GetDICOMTag( "PatientName" );
          if( !nameStr.empty() )
          {
            std::cout << "CONFIDENTIALITY BREECH! " << nameStr << std::endl;
            try{
              image->CleanHologicDICOM();
            }
            catch(...)
            {
            }
          }
        }
      }
    }
  }

  Alder::Application::DeleteInstance();
  return EXIT_SUCCESS;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool ApplicationInit()
{
  bool status = true;
  Alder::Application *app = Alder::Application::GetInstance();
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
