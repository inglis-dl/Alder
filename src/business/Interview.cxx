/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Interview.h>

#include <Application.h>
#include <Exam.h>
#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>
#include <User.h>
#include <Utilities.h>

#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

#include <map>
#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Interview );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetNeighbour(
    const bool &forward, const bool &loaded, const bool &unrated )
  {
    this->AssertPrimaryId();

    Application *app = Application::GetInstance();
    std::string interviewId = this->Get( "Id" ).ToString();
    std::string uId = this->Get( "UId" ).ToString();
    std::string userId = app->GetActiveUser()->Get( "Id" ).ToString();
    std::stringstream stream;

    // use a special query to quickly get the next interview
    if( !loaded && !unrated )
    {
      stream << "SELECT Id FROM Interview ";
    }
    else if( !loaded && unrated )
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Rating.Rating IS NOT NULL AS Rated "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "LEFT JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "LEFT JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<   "LEFT JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<   "AND UserHasModality.UserId = User.Id "
             <<   "LEFT JOIN Image ON Exam.Id = Image.ExamId "
             <<   "LEFT JOIN Rating ON Image.Id = Rating.ImageId "
             <<   "AND User.Id = Rating.UserId "
             <<   "WHERE User.Id = " << userId << " "
             <<   "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             << ") AS temp1 "
             << "WHERE Rated = false "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Rating.Rating IS NOT NULL AS Rated "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<     "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<     "AND UserHasModality.UserId = User.Id "
             <<     "JOIN Image ON Exam.Id = Image.ExamId "
             <<     "JOIN Rating ON Image.Id = Rating.ImageId "
             <<     "AND User.Id = Rating.UserId "
             <<     "WHERE User.Id = " << userId << " "
             <<     "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             <<   ") AS temp2 "
             <<   "WHERE Rated = true "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }
    else if ( loaded && !unrated )
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Exam.Downloaded "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<   "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<   "AND UserHasModality.UserId = User.Id "
             <<   "WHERE User.Id = " << userId << " "
             <<   "AND Stage = 'Completed' "
             <<   "GROUP BY Interview.Id, Downloaded "
             << ") AS temp1 "
             << "WHERE Downloaded = true "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Exam.Downloaded "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<     "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<     "AND UserHasModality.UserId = User.Id "
             <<     "WHERE User.Id = " << userId << " "
             <<     "AND Stage = 'Completed' "
             <<     "GROUP BY Interview.Id, Downloaded "
             <<   ") AS temp2 "
             <<   "WHERE Downloaded = false "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }
    else // loaded && unrated
    {
      stream << "SELECT Id, UId FROM ( "
             <<   "SELECT Interview.Id, UId, Rating.Rating IS NOT NULL AS Rated, Exam.Downloaded "
             <<   "FROM User "
             <<   "CROSS JOIN Interview "
             <<   "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<   "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<   "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<   "AND UserHasModality.UserId = User.Id "
             <<   "JOIN Image ON Exam.Id = Image.ExamId "
             <<   "LEFT JOIN Rating ON Image.Id = Rating.ImageId "
             <<   "AND User.Id = Rating.UserId "
             <<   "WHERE User.Id = " << userId << " "
             <<   "AND Stage = 'Completed' "
             <<   "GROUP BY Interview.Id, Rating.Rating IS NOT NULL, Downloaded "
             << ") AS temp1 "
             << "WHERE Rated = false "
             << "AND Downloaded = true "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Rating.Rating IS NOT NULL AS Rated "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<     "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<     "AND UserHasModality.UserId = User.Id "
             <<     "JOIN Image ON Exam.Id = Image.ExamId "
             <<     "JOIN Rating ON Image.Id = Rating.ImageId "
             <<     "AND User.Id = Rating.UserId "
             <<     "WHERE User.Id = " << userId << " "
             <<     "GROUP BY Interview.Id, Rating.Rating IS NOT NULL "
             <<   ") AS temp2 "
             <<   "WHERE Rated = true "
             << ") "
             << "AND Id NOT IN ( "
             <<   "SELECT Id FROM ( "
             <<     "SELECT Interview.Id, Exam.Downloaded "
             <<     "FROM User "
             <<     "CROSS JOIN Interview "
             <<     "JOIN Exam ON Interview.Id = Exam.InterviewId "
             <<     "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
             <<     "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
             <<     "AND UserHasModality.UserId = User.Id "
             <<     "WHERE User.Id = " << userId << " "
             <<     "AND Stage = 'Completed' "
             <<     "GROUP BY Interview.Id, Downloaded "
             <<   ") AS temp2 "
             <<   "WHERE Downloaded = false "
             << ") "
             << "UNION SELECT " << interviewId << ", '" << uId << "' ";
    }

    // order the query by UId (descending if not forward)
    stream << "ORDER BY UId ";
    if( !forward ) stream << "DESC ";

    app->Log( "Querying Database (Interview::GetNeighbour): " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    vtkVariant neighbourId;

    // store the first record in case we need to loop over
    if( query->NextRow() )
    {
      bool found = false;
      vtkVariant currentId = this->Get( "Id" );

      // if the current id is last in the following loop then we need the first id
      neighbourId = query->DataValue( 0 );

      do // keep looping until we find the current Id
      {
        vtkVariant id = query->DataValue( 0 );
        if( found )
        {
          neighbourId = id;
          break;
        }

        if( currentId == id ) found = true;
      }
      while( query->NextRow() );

      // we should always find the current interview id
      if( !found ) throw std::runtime_error( "Cannot find current Interview in database." );
    }

    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    if( neighbourId.IsValid() ) interview->Load( "Id", neighbourId.ToString() );
    return interview;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< std::pair< std::string, std::string > > Interview::GetUIdVisitDateList()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( "SELECT UId, VisitDate FROM Interview ORDER BY UId, VisitDate" );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

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
    std::vector< vtkSmartPointer< Exam > > examList;
    this->GetList( &examList );
    int total = 0;
    for( auto it = examList.cbegin(); it != examList.cend(); ++it )
    {
      total += (*it)->GetCount( "Image" );
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
    this->GetList( &examList );
    for( auto examIt = examList.cbegin(); examIt != examList.cend(); ++examIt )
    {
      Exam *exam = *(examIt);
      if( !exam->IsRatedBy( user ) ) return false;
    }

    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::HasExamData()
  {
    return 0 < this->GetCount( "Exam" );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::HasImageData( QueryModifier *modifier )
  {
    this->AssertPrimaryId();
    Application *app = Application::GetInstance();

    std::vector< vtkSmartPointer< Exam > > examList;
    this->GetList( &examList, modifier );
    if( examList.empty() )
    {
      throw std::runtime_error( "No exams attributed to the interview" );
    }
    for( auto examIt = examList.cbegin(); examIt != examList.cend(); ++examIt )
    {
      if( !(*examIt)->HasImageData() )
      {
        return false;
      }
    }
    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateExamData( const bool& updateMetaData )
  {
    // only update the exams if there are none in the database
    if( !updateMetaData && this->HasExamData() ) return;

    OpalService *opal = Application::GetInstance()->GetOpal();

    // get exam metadata from Opal for this interview
    std::map< std::string, std::string > examData =
      opal->GetRow( "alder", "Exam", this->Get( "UId" ).ToString() );

    // build a map of modalities and exams
    std::map<  std::string, std::vector< std::pair< std::string, std::string > > >
      modalityMap;

     modalityMap["Ultrasound"] = {
       {"CarotidIntima","left"},
       {"CarotidIntima","right"}
     };
     modalityMap["Dexa"] = {
       {"DualHipBoneDensity","left"},
       {"DualHipBoneDensity","right"},
       {"ForearmBoneDensity","left"},
       {"ForearmBoneDensity","right"},
       {"LateralBoneDensity","none"},
       {"WholeBodyBoneDensity","none"}
     };
     modalityMap["Retinal"] = {
       {"RetinalScan","left"},
       {"RetinalScan","right"}
     };

    vtkVariant interviewId = this->Get( "Id" );

    for( auto mapIt = modalityMap.cbegin(); mapIt != modalityMap.cend(); ++mapIt )
    {
      vtkNew<Modality> modality;
      modality->Load( "Name", mapIt->first );
      vtkVariant modalityId = modality->Get( "Id" );
      for( auto vecIt = mapIt->second.cbegin(); vecIt != mapIt->second.cend(); ++vecIt )
      {
        bool create = true;

        vtkSmartPointer< Alder::ScanType > scanType = vtkSmartPointer< Alder::ScanType >::New();
        std::map< std::string, std::string > scanTypeMap;
        scanTypeMap.insert(
          std::pair<std::string, std::string>( "ModalityId", vtkVariant( modalityId ).ToString() ) );
        scanTypeMap.insert(
          std::pair<std::string, std::string>( "Type", vtkVariant( vecIt->first ).ToString() ) );
        scanType->Load( scanTypeMap );

        vtkVariant scanTypeId = scanType->Get( "Id" );

        if( updateMetaData )
        {
          vtkSmartPointer< Alder::QueryModifier > modifier =
            vtkSmartPointer< Alder::QueryModifier >::New();
          modifier->Where( "ScanTypeId", "=", scanTypeId );
          modifier->Where( "Laterality", "=", vtkVariant( vecIt->second ) );

          std::vector< vtkSmartPointer< Exam > > examList;
          this->GetList( &examList, modifier );
          if( !examList.empty() )
          {
            create = false;
            auto it = examList.cbegin();
            Alder::Exam * exam = it->GetPointer();
            // check if the data need to be updated
            std::string value = examData[ vecIt->first + ".Stage" ];
            bool save = false;
            if( exam->Get("Stage").ToString() != value )
            {
              exam->Set( "Stage", value );
              save = true;
            }

            value = examData[ vecIt->first + ".Interviewer"];
            if( exam->Get("Interviewer").ToString() != value )
            {
              exam->Set( "Interviewer", value );
              save = true;
            }

            value = examData[ vecIt->first + ".DatetimeAcquired"];
            if( exam->Get("DatetimeAcquired").ToString() != value )
            {
              exam->Set( "DatetimeAcquired", value );
              save = true;
            }

            if( save ) exam->Save();
          }
        }

        if( create )
        {
          vtkNew<Exam> exam;
          exam->Set( "InterviewId", interviewId );
          exam->Set( "ScanTypeId", scanTypeId );
          exam->Set( "Laterality", vecIt->second );
          exam->Set( "Stage", examData[ vecIt->first + ".Stage" ] );
          exam->Set( "Interviewer", examData[ vecIt->first + ".Interviewer"] );
          exam->Set( "DatetimeAcquired", examData[ vecIt->first + ".DatetimeAcquired"] );
          exam->Save();
        }
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateImageData( ProgressProxy& proxy )
  {
    Application *app = Application::GetInstance();
    User* user = app->GetActiveUser();
    std::vector< vtkSmartPointer< Exam > > examList;
    if( user )
    {
      std::stringstream stream;

      stream << "SELECT Exam.* FROM Exam "
             << "JOIN Interview ON Interview.Id=Exam.InterviewId "
             << "JOIN ScanType ON Exam.ScanTypeId=ScanType.Id "
             << "WHERE ScanType.ModalityId IN ( "
             << "SELECT Modality.Id FROM Modality "
             << "JOIN UserHasModality ON UserHasModality.ModalityId=Modality.Id "
             << "JOIN User on User.Id=UserHasModality.UserId "
             << "WHERE User.Id=" << user->Get( "Id" ).ToString() << " ) "
             << "AND Interview.Id=" << this->Get( "Id" ).ToString();

      Database *db = app->GetDB();
      vtkSmartPointer<vtkAlderMySQLQuery> query = db->GetQuery();

      app->Log( "Querying Database: " + stream.str() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      if( query->HasError() )
      {
        app->Log( query->GetLastErrorText() );
        throw std::runtime_error( "There was an error while trying to query the database." );
      }

      while( query->NextRow() )
      {
        vtkSmartPointer<Exam> exam = vtkSmartPointer<Exam>::New();
        exam->LoadFromQuery( query );
        examList.push_back( exam );
      }
    }
    else
    {
      this->GetList( &examList );
    }

    if( !examList.empty() )
    {
      // only update image data for exams which have not been downloaded
      std::vector< vtkSmartPointer< Exam > > revisedList;
      for( auto examIt = examList.cbegin(); examIt != examList.cend(); ++examIt )
      {
        if( !(*examIt)->HasImageData() )
          revisedList.push_back( *examIt );
      }
      examList.clear();
      if( revisedList.empty() ) return;

      proxy.StartProgress();

      double index = 0;
      double size = revisedList.size();
      for( auto examIt = revisedList.cbegin(); examIt != revisedList.cend(); ++examIt, ++index )
      {
        double progress = index / size;
        proxy.UpdateProgress( progress );
        if( proxy.GetAbortStatus() ) break;
        ( *examIt )->UpdateImageData();
      }

      proxy.EndProgress();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateInterviewData( ProgressProxy& proxy )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();

    // get a list of all interview start dates
    std::vector< std::string > identifierList = opal->GetIdentifiers( "alder", "Interview" );
    std::map< std::string, std::map< std::string, std::string > > list;
    std::map< std::string, std::string > map, key;
    bool done = false;
    int limit = 100;
    double index = 0;
    double size = (double) identifierList.size();

    proxy.StartProgress();

    do
    {
      double progress = index/size;
      proxy.UpdateProgress( progress );
      if( proxy.GetAbortStatus() ) break;

      list = opal->GetRows( "alder", "Interview", index, limit );

      for( auto it = list.cbegin(); it != list.cend(); ++it )
      {
        std::string UId = it->first;
        map = it->second;

        // create a unique value map to load the interview
        key["UId"] = UId;
        key["VisitDate"] = map["VisitDate"].substr( 0, 10 );

        vtkNew< Interview > interview;
        if( !interview->Load( key ) )
        { // only write to the database if the data is missing
          interview->Set( "UId", UId );
          interview->Set( map );
          interview->Save();
        }
      }

      // prepare the next block of start dates
      index += list.size();
    } while ( !list.empty() );

    proxy.EndProgress();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::LoadFromUIDList( std::vector< std::string > const &uidList, ProgressProxy& proxy )
  {
    vtkSmartPointer< QueryModifier > modifier = vtkSmartPointer< QueryModifier >::New();
    for( auto it = uidList.cbegin(); it != uidList.cend(); ++it )
    {
      // create a modifier using the search text
      std::string where = *it;
      where += "%";
      modifier->Where( "UId", "LIKE", vtkVariant( where ), true,
        (it != uidList.cbegin()) );
    }

    // get all the interviews given the search text
    std::vector< vtkSmartPointer< Interview > > interviewList;
    Interview::GetAll( &interviewList, modifier );

    if( interviewList.empty() ) return 0;

    Application *app = Application::GetInstance();
    double size = (double) interviewList.size();
    int index = 0;

    ProgressProxy innerProxy;
    innerProxy.SetProgressTypeLocal();
    innerProxy.SetCurlProgressOff();

    proxy.StartProgress();

    Alder::User *user = app->GetActiveUser();
    if( !user ) return 0;

    user->InitializeExamModifier( modifier );

    for( auto it = interviewList.begin(); it != interviewList.end(); ++it, ++index )
    {
      double progress = index / size;
      proxy.UpdateProgress( progress );
      if( proxy.GetAbortStatus() ) break;

      Interview *interview = *it;
      if( !interview->HasExamData() )
      {
        try
        {
          interview->UpdateExamData();
        }
        catch( std::runtime_error& e )
        {
          std::string err = "There was an error while trying to update exam data ( UId : ";
          err += interview->Get( "UId" ).ToString();
          err += " ). Error: ";
          err += e.what();
          app->Log( err );
          continue;
        }
      }
      if( !interview->HasImageData( modifier ) )
      {
        try
        {
          interview->UpdateImageData( innerProxy );
          if( innerProxy.GetAbortStatus() ) break;
        }
        catch( std::runtime_error& e )
        {
          std::string err = "There was an error while trying to update image data ( UId : ";
          err += interview->Get( "UId" ).ToString();
          err += " ). Error: ";
          err += e.what();
          app->Log( err );
          continue;
        }
      }
    }

    proxy.EndProgress();

    return index;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Interview::GetSimilarImage( std::string const &imageId )
  {
    this->AssertPrimaryId();
    std::string matchId;
    if( imageId.empty() ) return matchId;

    vtkNew<Image> image;
    image->Load( "Id", imageId );
    bool hasParent = image->Get( "ParentImageId" ).IsValid();

    std::stringstream stream;

    // given an image Id, find an image in this record having the same
    // characteristics
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN Exam AS simExam ON Exam.ScanTypeId = simExam.ScanTypeId "
           << "AND Exam.ScanTypeId = simExam.ScanTypeId "
           << "AND Exam.Laterality = simExam.Laterality "
           << "AND Exam.Stage = simExam.Stage "
           << "JOIN Image AS simImage ON simImage.ExamId = simExam.Id "
           << "WHERE Exam.InterviewId = " << this->Get( "Id" ).ToString() << " "
           << "AND simImage.Id = " << imageId << " "
           << "AND simImage.ParentImageId IS " << (hasParent ? "NOT NULL " : "NULL " )
           << "AND Image.ParentImageId IS " << (hasParent ? "NOT NULL " : "NULL " )
           << "LIMIT 1";

    Application *app = Application::GetInstance();
    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    if( query->NextRow() )
    {
      matchId = query->DataValue( 0 ).ToString();
    }
    return matchId;
  }

} // end namespace Alder
