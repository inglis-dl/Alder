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
#include <Site.h>
#include <User.h>

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
  int Interview::GetImageCount()
  {
    this->AssertPrimaryId();

    std::string interviewId = this->Get( "Id" ).ToString();
    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM Image "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "JOIN Interview ON Interview.Id=Exam.InterviewId "
           << "WHERE Interview.Id=" << interviewId;

    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    // only one row
    query->NextRow();
    return query->DataValue(0).ToInt();
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
      std::string err = "No exams attributed to interview ";
      err += this->Get("Id").ToString();
      err += " for UID ";
      err += this->Get("UId").ToString();
      app->Log( err.c_str() );
      return false;
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
  void Interview::UpdateExamData( Alder::Wave *aWave,
    const std::string &aMetaSource )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if( !sustain )
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch( std::runtime_error& e )
      {
        app->Log( e.what() );
        return;
      }
    }

    // get the Wave associated with this Interview
    vtkSmartPointer< Alder::Wave > wave;
    if( NULL == aWave )
    {
      if( !this->GetRecord( wave ) )
        throw std::runtime_error( "Interview has no Wave record!" );
    }
    else
      wave = aWave;

    std::string source = aMetaSource;
    if( source.empty() )
      source = wave->Get( "MetaDataSource" ).ToString();

    std::string identifier = this->Get( "UId" ).ToString();
    std::map< std::string, std::string > opalData = opal->GetRow( source, "Exam", identifier );

    if( !sustain )
      opal->SustainConnectionOff();

    if( opalData.empty() ) return;

    std::string interviewId = this->Get( "Id" ).ToString();

    std::map< std::string, std::map< std::string, std::string > > examData;
    if( examData.empty() )
    {
      std::vector< vtkSmartPointer< Alder::ScanType > > scanTypeList;
      wave->GetList( &scanTypeList );
      for( auto it = scanTypeList.cbegin(); it != scanTypeList.cend(); ++it )
      {
        std::string typeStr = (*it)->Get( "Type" ).ToString();
        std::string idStr = (*it)->Get( "Id" ).ToString();
        examData[ typeStr ][ "ScanTypeId" ] = idStr;
      }
    }

    // parse the opal data
    for( auto it = opalData.cbegin(); it != opalData.cend(); ++it )
    {
      std::string opalVar = it->first;
      std::string opalVal = it->second;

      std::vector< std::string > tmp = Alder::Utilities::explode( opalVar, "." );
      if( 2 != tmp.size() ) continue;
      std::string examType = tmp.front();
      std::string examVar  = tmp.back();

      if( examData.find( examType ) == examData.end() ) continue;
      examData[examType][examVar] = opalVal;
    }

    std::map< std::string, std::string > loader;
    vtkSmartPointer< Exam > exam = vtkSmartPointer< Exam >::New();
    for( auto it = examData.cbegin(); it != examData.cend(); ++it )
    {
      std::string type = it->first;
      std::map<std::string, std::string> columns = it->second;

      if( columns.find("Side") == columns.end() ||
          columns.find("ScanTypeId") == columns.end() )
        continue;

      if( columns["Side"].empty() )
        continue;

      std::vector< std::string > vecSide = Alder::Utilities::explode( columns["Side"], "," );
      if( vecSide.empty() ) continue;

      loader["ScanTypeId"] = columns["ScanTypeId"];
      loader["InterviewId"] = interviewId;
      for( auto vit = vecSide.cbegin(); vit != vecSide.cend(); ++vit )
      {
        std::string sideStr = *vit;
        loader["Side"] = sideStr;
        if( exam->Load( loader ) )
        {
          // check and update data as required
          bool save = false;
          std::string value = columns["Stage"];
          if( exam->Get("Stage").ToString() != value )
          {
            exam->Set( "Stage", value );
            save = true;
          }

          value = columns["Interviewer"];
          if( exam->Get("Interviewer").ToString() != value )
          {
            exam->Set( "Interviewer", value );
            save = true;
          }

          value = columns["DatetimeAcquired"];
          if( exam->Get("DatetimeAcquired").ToString() != value )
          {
            exam->Set( "DatetimeAcquired", value );
            save = true;
          }

          if( save )
          {
            exam->Save();
          }
        }
        else
        {
          vtkNew<Exam> exam;
          exam->Set( "InterviewId", interviewId );
          exam->Set( "ScanTypeId", columns["ScanTypeId"] );
          exam->Set( "Side", sideStr );
          exam->Set( "Stage", columns["Stage"] );
          exam->Set( "Interviewer", columns["Interviewer"] );
          exam->Set( "DatetimeAcquired", columns["DatetimeAcquired"] );
          exam->Save();
        }
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateImageData()
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
             << "JOIN User ON User.Id=UserHasModality.UserId "
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

      double index = 0;
      double size = revisedList.size();
      for( auto examIt = revisedList.cbegin(); examIt != revisedList.cend(); ++examIt, ++index )
      {
        ( *examIt )->UpdateImageData();
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateInterviewData( const std::vector< std::pair< int ,bool > > &waveList )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if( !sustain )
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch( std::runtime_error& e )
      {
        app->Log( e.what() );
        return;
      }
    }

    double size = 0.;
    std::map< std::string,           // key Wave.Id
      std::vector< std::string > >   // value vector of Interview.UId
        mapWave;
    for( auto it = waveList.cbegin(); it != waveList.cend(); ++it )
    {
      std::string waveId = vtkVariant( it->first ).ToString();
      bool fullUpdate = it->second; 
      vtkNew< Wave > wave;
      if( !wave->Load( "Id", waveId ) ) continue;

      // get the list of UId's available in Opal
      std::string source = wave->Get( "MetaDataSource" ).ToString();
      std::vector< std::string > vecOpal = opal->GetIdentifiers( source, "Interview" );

      if( vecOpal.empty() ) continue;
      std::sort( vecOpal.begin(), vecOpal.end() );
      std::vector< std::string > vecUId;

      if( !fullUpdate )
      { 
        // get the list of UIds to exclude from the update
        int maxExam = wave->GetMaximumExamCount();
        std::stringstream stream;
        stream << "SELECT UId, COUNT(*) FROM Interview "
               << "JOIN Exam ON Exam.InterviewId=Interview.Id "
               << "WHERE WaveId=" << waveId << " "
               << "GROUP BY UId "
               << "HAVING COUNT(*)=" << maxExam << " "
               << "ORDER BY UId ";
      
        app->Log( "Querying Database: " + stream.str() );
        vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
        query->SetQuery( stream.str().c_str() );
        query->Execute();      

        if( query->HasError() )
        {
          app->Log( query->GetLastErrorText() );
          throw std::runtime_error( "There was an error while trying to query the database." );
        }

        while( query->NextRow() )
        {
          std::string uidStr  = query->DataValue( 0 ).ToString();
          vecUId.push_back( uidStr );
        }
      }

      if( vecUId.empty() )
      {
        std::cout << "downloading all opal interview data for wave " << waveId << std::endl;
        mapWave[ waveId ] = vecOpal;
      }
      else
      {
        std::vector< std::string > vecWave( vecOpal.size() );
        std::vector< std::string >::iterator vit =
         std::set_difference( vecOpal.begin(), vecOpal.end(), vecUId.begin(), vecUId.end(), vecWave.begin() );
        vecWave.resize( vit - vecWave.begin() );

        mapWave[ waveId ] = vecWave;

        std::cout << "downloading partial opal interview data for wave " << waveId << 
                  vecOpal.size() << " ids in opal with " << vecWave.size() << " remaining to pull" << std::endl;
      }

      size += mapWave[ waveId ].size();
    }

    if( 0. == size )
    {
      if( !sustain )
        opal->SustainConnectionOff();
      return;
    }

    std::cout << "start looping on waves getting " << size << " opal identifiers" << std::endl;

    // get a map of site ids, names and aliases
    std::vector< vtkSmartPointer< Site > > vecSite;
    Alder::Site::GetAll( &vecSite );
    std::map< std::string, std::string > mapSite;
    for( auto it = vecSite.cbegin(); it != vecSite.cend(); ++it )
    {
      Alder::Site *site = *it;
      std::string siteId = site->Get( "Id" ).ToString();
      mapSite[ site->Get( "Name" ).ToString() ] = siteId;
      std::string alias = site->Get( "Name" ).ToString();
      if( !alias.empty() )
        mapSite[ alias ] = siteId;
    }

    // determine number of identifiers to pull per Opal curl call
    int limit = static_cast<int>(0.1 * size);
    limit = limit > 500 ? 500 : ( limit < 1 ? 1 : limit );
    int index = 0;
    double progress = 0.;
    vtkSmartPointer< Alder::Wave > wave = vtkSmartPointer< Alder::Wave >::New();
    for( auto it = mapWave.cbegin(); it != mapWave.cend(); ++it )
    {
      std::string waveId = it->first;
      wave->Load( "Id", waveId );
      std::string source = wave->Get( "MetaDataSource" ).ToString();
      std::vector< std::string > vecUId = it->second;
      std::vector< std::string >::iterator ibegin = vecUId.begin();
      std::vector< std::string >::iterator iend = vecUId.end();
      std::map< std::string, std::map< std::string, std::string > > mapOpal;
      bool done = false;
      int localIndex = 0;
      do
      {
        progress = index/size;
        std::cout << "progress " << progress << std::endl;

        mapOpal = opal->GetRows( source, "Interview", localIndex, limit );
        for( auto mit = mapOpal.cbegin(); mit != mapOpal.cend(); ++mit )
        {
          // skip identifiers that we already have in the Alder db
          std::string uidStr = mit->first;
          if( std::find( ibegin, iend, uidStr ) == iend )
            continue;

          // if found, try to load the interview
          std::map< std::string, std::string > mapVar = mit->second;

          // create a unique value map to load the interview
          std::map< std::string, std::string > loader;
          loader["WaveId"] = waveId;
          loader["UId"] = uidStr;
          loader["VisitDate"] = mapVar["VisitDate"];

          vtkNew< Interview > interview;
          if( !interview->Load( loader ) )
          {
            // create the new interview
            std::string siteStr = mapVar["Site"];
            std::string siteId;
            auto sit = mapSite.find( siteStr );
            if( sit == mapSite.end() )
            {
              vtkNew< Site > site;
              if( site->Load( "Name", siteStr ) )
                siteId = site->Get( "Id" ).ToString();
              else if( site->Load( "Alias", siteStr ) )
                siteId = site->Get( "Id" ).ToString();
              else
              {
                site->Set( "Name", siteStr );
                site->Save();
                siteId = site->Get( "Id" ).ToString();
              }
              mapSite[siteStr] = siteId;
            }
            else
            {
              siteId = sit->second;
            }
            loader["SiteId"] = siteId;

            interview->Set( loader );
            interview->Save();
            interview->Load( loader );
          }

          interview->UpdateExamData( wave, source );
          index++;
        }

        localIndex += mapOpal.size();
      } while ( !mapOpal.empty() );
    }

    if( !sustain )
      opal->SustainConnectionOff();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::LoadFromList( const std::vector< std::pair< std::string, std::string > > &list )
  {
    std::vector< vtkSmartPointer< Interview > > interviewList;
    vtkNew< Alder::Wave > wave;
    for( auto it = list.cbegin(); it != list.cend(); ++it )
    {
      std::string uidStr = (*it).first;
      std::string rankStr = (*it).second;
      if( wave->Load( "Rank", rankStr ) )
      {
        vtkSmartPointer< Interview > interview = vtkSmartPointer< Interview >::New();
        std::map< std::string, std::string > loader;
        loader[ "UId" ] = uidStr;
        loader[ "WaveId" ] = wave->Get( "Id" ).ToString();
        if( interview->Load( loader ) )
        {
          interviewList.push_back( interview );
        }
      }
    }

    if( interviewList.empty() ) return 0;

    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if( !sustain )
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch( std::runtime_error& e )
      {
        app->Log( e.what() );
        return 0;
      }
    }

    Alder::User *user = app->GetActiveUser();
    if( !user ) return 0;

    vtkSmartPointer< Alder::QueryModifier > modifier =
      vtkSmartPointer< Alder::QueryModifier >::New();
    user->InitializeExamModifier( modifier );

    double size = (double)interviewList.size();
    int index = 0;
    for( auto it = interviewList.cbegin(); it != interviewList.cend(); ++it, ++index )
    {
      double progress = index / size;
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
          interview->UpdateImageData();
          //interview->UpdateImageData( innerProxy );
          //if( innerProxy.GetAbortStatus() ) break;
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

    if( !sustain )
      opal->SustainConnectionOff();

    return index;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Interview::GetSimilarImageId( const std::string &imageId )
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
           << "AND Exam.Side = simExam.Side "
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
