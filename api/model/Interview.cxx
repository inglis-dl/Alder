/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.cxx
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "Interview.h"

#include "Application.h"
#include "Exam.h"
#include "OpalService.h"
#include "User.h"
#include "Utilities.h"

#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkSmartPointer.h"

#include <map>
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Interview );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetNext()
  {
    std::string currentUId = this->Get( "UId" ).ToString();
    std::string currentVisitDate = this->Get( "VisitDate" ).ToString();
    std::vector< std::pair< std::string, std::string > > list = Interview::GetUIdVisitDateList();
    std::vector< std::pair< std::string, std::string > >::reverse_iterator pair;

    // the list should never be empty (since we are already an instance of interview)
    if( list.empty() )
      throw std::runtime_error( "Interview list is empty while trying to get next interview." );
    
    // find this record's UId in the list, return the next one
    for( pair = list.rbegin(); pair != list.rend(); pair++ )
    {
      if( currentUId == pair->first && currentVisitDate == pair->second )
      {
        if( list.rbegin() == pair )
        { // first UId matches, get the last pair
          pair = list.rbegin();
        }
        else
        { // move the iterator to the previous address
          pair--;
        }
        break;
      }
    }

    if( list.rend() == pair )
      throw std::runtime_error( "Interview list does not include current UId/VisitDate pair." );

    std::map< std::string, std::string > map;
    map["UId"] = pair->first;
    map["VisitDate"] = pair->second;
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    interview->Load( map );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::Next()
  {
    this->Load( "Id", this->GetNext()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetPrevious()
  {
    std::string currentUId = this->Get( "UId" ).ToString();
    std::string currentVisitDate = this->Get( "VisitDate" ).ToString();
    std::vector< std::pair< std::string, std::string > > list = Interview::GetUIdVisitDateList();
    std::vector< std::pair< std::string, std::string > >::iterator pair;

    // the list should never be empty (since we are already an instance of interview)
    if( list.empty() )
      throw std::runtime_error( "Interview list is empty while trying to get next interview." );
    
    // find this record's UId in the list, return the next one
    std::string UId;
    for( pair = list.begin(); pair != list.end(); pair++ )
    {
      if( currentUId == pair->first && currentVisitDate == pair->second )
      {
        if( list.begin() == pair )
        { // first UId matches, get the last UId
          pair = list.end();
        }
        else
        { // move the iterator to the previous address, get its value
          pair--;
        }
        break;
      }
    }

    if( list.end() == pair )
      throw std::runtime_error( "Interview list does not include current UId/VisitDate pair." );

    std::map< std::string, std::string > map;
    map["UId"] = pair->first;
    map["VisitDate"] = pair->second;
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    interview->Load( map );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::Previous()
  {
    this->Load( "Id", this->GetPrevious()->Get( "Id" ).ToString() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::pair< std::string, std::string > > Interview::GetUIdVisitDateList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT UId, VisitDate FROM Interview ORDER BY UId, VisitDate" );
    query->Execute();

    std::vector< std::pair< std::string, std::string > > list;
    while( query->NextRow() )
      list.push_back( std::pair< std::string, std::string >(
        query->DataValue( 0 ).ToString(), query->DataValue( 1 ).ToString() ) );
    return list;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::GetImageCount()
  {
    // loop through all exams and sum the image count
    int total = 0;

    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      total += exam->GetCount( "Image" );
    }

    return total;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::vector< vtkSmartPointer< Exam > > examList;
    std::vector< vtkSmartPointer< Exam > >::iterator examIt;
    this->GetList( &examList );
    for( examIt = examList.begin(); examIt != examList.end(); ++examIt )
    {
      Exam *exam = *(examIt);
      if( !exam->IsRatedBy( user ) ) return false;
    }

    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::Update()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    std::map< std::string, std::string > examData;
    vtkSmartPointer<Exam> exam;

    // make sure all stages exist

    // only update the exams if there are none in the database
    if( 0 == this->GetCount( "Exam" ) )
    {
      // get exam metadata from Opal for this interview
      examData = opal->GetRow( "alder", "Exam", this->Get( "UId" ).ToString() );

      // CarotidIntima
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "CarotidIntima" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["CarotidIntimaStage"] );
      exam->Set( "Interviewer", examData["CarotidIntimaStage"] );
      exam->Set( "DatetimeAcquired", examData["CarotidIntimaDatetimeAcquired"] );

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "CarotidIntima" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["CarotidIntimaStage"] );
      exam->Set( "Interviewer", examData["CarotidIntimaStage"] );
      exam->Set( "DatetimeAcquired", examData["CarotidIntimaDatetimeAcquired"] );

      // DualHipBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "DualHipBoneDensity" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["DualHipBoneDensityStage"] );
      exam->Set( "Interviewer", examData["DualHipBoneDensityStage"] );
      exam->Set( "DatetimeAcquired", examData["DualHipBoneDensityDatetimeAcquired"] );

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "DualHipBoneDensity" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["DualHipBoneDensityStage"] );
      exam->Set( "Interviewer", examData["DualHipBoneDensityStage"] );
      exam->Set( "DatetimeAcquired", examData["DualHipBoneDensityDatetimeAcquired"] );

      // ForearmBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "ForearmBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["ForearmBoneDensityStage"] );
      exam->Set( "Interviewer", examData["ForearmBoneDensityStage"] );
      exam->Set( "DatetimeAcquired", examData["ForearmBoneDensityDatetimeAcquired"] );

      // LateralBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "LateralBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["LateralBoneDensityStage"] );
      exam->Set( "Interviewer", examData["LateralBoneDensityStage"] );
      exam->Set( "DatetimeAcquired", examData["LateralBoneDensityDatetimeAcquired"] );

      // Plaque
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "Plaque" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["PlaqueStage"] );
      exam->Set( "Interviewer", examData["PlaqueStage"] );
      exam->Set( "DatetimeAcquired", examData["PlaqueDatetimeAcquired"] );

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Ultrasound" );
      exam->Set( "Type", "Plaque" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["PlaqueStage"] );
      exam->Set( "Interviewer", examData["PlaqueStage"] );
      exam->Set( "DatetimeAcquired", examData["PlaqueDatetimeAcquired"] );

      // RetinalScan
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Retinal" );
      exam->Set( "Type", "RetinalScan" );
      exam->Set( "Laterality", "left" );
      exam->Set( "Stage", examData["RetinalScanStage"] );
      exam->Set( "Interviewer", examData["RetinalScanStage"] );
      exam->Set( "DatetimeAcquired", examData["RetinalScanDatetimeAcquired"] );

      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Retinal" );
      exam->Set( "Type", "RetinalScan" );
      exam->Set( "Laterality", "right" );
      exam->Set( "Stage", examData["RetinalScanStage"] );
      exam->Set( "Interviewer", examData["RetinalScanStage"] );
      exam->Set( "DatetimeAcquired", examData["RetinalScanDatetimeAcquired"] );

      // WholeBodyBoneDensity
      exam = vtkSmartPointer<Exam>::New();
      exam->Set( "InterviewId", this->Get( "Id" ) );
      exam->Set( "Modality", "Dexa" );
      exam->Set( "Type", "WholeBodyBoneDensity" );
      exam->Set( "Laterality", "none" );
      exam->Set( "Stage", examData["WholeBodyBoneDensityStage"] );
      exam->Set( "Interviewer", examData["WholeBodyBoneDensityStage"] );
      exam->Set( "DatetimeAcquired", examData["WholeBodyBoneDensityDatetimeAcquired"] );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateData()
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    double progress = 0.0;
    app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );

    // get a list of all interview start dates
    std::map< std::string, std::map< std::string, std::string > > list;
    std::map< std::string, std::string > map, key;
    std::map< std::string, std::map< std::string, std::string > >::iterator it;
    bool done = false;
    int limit = 100;
    int index = 0;

    std::vector< std::string > identifierList = opal->GetIdentifiers( "alder", "Interview" );
    double size = (double) identifierList.size();

    do
    {
      list = opal->GetRows( "alder", "Interview", index, limit );

      for( it = list.begin(); it != list.end(); ++it )
      {
        std::string UId = it->first;
        map = it->second;

        // create a unique value map to load the interview
        key["UId"] = UId;
        key["VisitDate"] = map["VisitDate"].substr( 0, 10 );

        vtkSmartPointer< Interview > interview = vtkSmartPointer< Interview >::New();
        if( !interview->Load( key ) )
        { // only write to the database if the data is missing
          interview->Set( "UId", UId );
          interview->Set( map );
          interview->Save();
        }
      }

      // prepare the next block of start dates
      index += list.size();
      progress = index / size;
      app->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &progress ) );
    } while ( !list.empty() );
  }
}
