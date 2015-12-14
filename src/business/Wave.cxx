/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Wave.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Wave.h>

#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>
#include <Utilities.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( Wave );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Wave::UpdateModalityData()
  {
    std::string source = this->Get( "MetaDataSource" ).ToString();
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    std::vector< std::string > examData = opal->GetVariables( source, "Exam" );
    std::vector< std::string > unique;
    for( auto it = examData.cbegin(); it != examData.cend(); ++it )
    {
      std::vector< std::string > tmp = Alder::Utilities::explode( *it, "." );
      if( 2 != tmp.size() )
        continue;

      std::string examVar = tmp.back();
      if( "Modality" == examVar )
      {
        std::map< std::string, std::string > colMap =
          opal->GetColumn( source, "Exam", *it, 0, 1 );
        if( colMap.empty() )
          continue;

        std::string name = colMap.begin()->second;
        if( name.empty() )
          continue;

        if( std::find( unique.begin(), unique.end(), name ) == unique.end() )
        {
          unique.push_back( name );
          vtkNew< Alder::Modality > modality;
          if( !modality->Load( "Name", name ) )
          {
            modality->Set( "Name", name );
            modality->Save();
          }
        }
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Wave::UpdateScanTypeData()
  {
    std::string source = this->Get( "MetaDataSource" ).ToString();
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    std::vector< std::string > examData = opal->GetVariables( source, "Exam" );
    std::vector< std::string > unique;
    std::string waveId = this->Get( "Id" ).ToString();
    for( auto it = examData.cbegin(); it != examData.cend(); ++it )
    {
      std::vector< std::string > tmp = Alder::Utilities::explode( *it, "." );
      if( 2 != tmp.size() )
        continue;
      std::string examType = tmp.front();
      std::string examVar = tmp.back();
      if( "Modality" == examVar )
      {
        if( std::find( unique.begin(), unique.end(), examType ) == unique.end() )
        {
          std::map< std::string, std::string > colMap =
            opal->GetColumn( source, "Exam", *it, 0, 1 );
          if( colMap.empty() )
            continue;

          std::string name = colMap.begin()->second;
          if( name.empty() )
            continue;

          vtkNew< Alder::Modality > modality;
          if( modality->Load( "Name", name ) )
          {
            unique.push_back( examType );
            std::map< std::string, std::string > loader;
            loader[ "ModalityId" ] = modality->Get( "Id" ).ToString();
            loader[ "Type" ] = examType;
            vtkSmartPointer< Alder::ScanType > scanType = vtkSmartPointer< Alder::ScanType >::New();
            if( !scanType->Load( loader ) )
            {
              scanType->Set( loader );
              scanType->Save();
              scanType->Load( loader );
            }
            if( !this->Has( scanType ) )
              this->AddRecord( scanType );
          }
        }
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Wave::UpdateWaveData( const std::string &source )
  {
    // get the waves from Opal
    // create or update as required
    if( source.empty() ) return;

    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    try
    {
      opal->SustainConnectionOn();
    }
    catch( std::runtime_error& e )
    {
      app->Log( e.what() );
      return;
    }

    // the list of Waves by Name
    std::vector< std::string > vecOpal = opal->GetIdentifiers( source, "Wave" );
    if( vecOpal.empty() )
    {
      std::string err = "Opal data source ";
      err += source;
      err += " is missing Wave identifiers";
      app->Log( err );
      if( !sustain )
        opal->SustainConnectionOff();
      return;
    }

    std::vector< std::string > validate = app->GetDB()->GetColumnNames( "Wave" );

    std::map< std::string, std::map< std::string, std::string > > mapOpal =
      opal->GetRows( source, "Wave", 0, vecOpal.size() );

    for( auto it = mapOpal.cbegin(); it != mapOpal.cend(); ++it )
    {
      std::string name = it->first;
      if( name.empty() )
         continue;

      std::map<std::string,std::string> mapVar = it->second;
      vtkNew< Alder::Wave > wave;
      bool create = true;
      bool update = false;
      if( wave->Load( "Name", name ) )
      {
        create = false;
        update = mapVar.size() <= validate.size();
      }

      if( false == create && false == update )
        continue;

      std::map< std::string, std::string > loader;
      for( auto mit = mapVar.cbegin(); mit != mapVar.cend(); ++mit )
      {
        std::string waveVar = mit->first;
        std::string waveVal = mit->second;
        if( std::find( validate.begin(), validate.end(), waveVar ) == validate.end() )
        {
          std::string warning = "Invalid wave column in Opal source ";
          warning += source;
          app->Log( warning );
          create = false;
          update = false;
          break;
        }
        else
        {
          if( create || ( update && waveVal != wave->Get( waveVar ).ToString() ) )
            loader[waveVar] = waveVal;
        }
      }

      if( create || update && !loader.empty() )
      {
        loader[ "Name" ] = name;
        wave->Set( loader );
        wave->Save();
        if( create )
          wave->Load( loader );
      }

      wave->UpdateModalityData();
      wave->UpdateScanTypeData();
    }

    if( !sustain )
      opal->SustainConnectionOff();
  }
}
