/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Rating.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Rating.h>

#include <Code.h>
#include <CodeType.h>
#include <CodeGroup.h>
#include <User.h>
#include <Utilities.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( Rating );

  const int Rating::MaximumRating = 5;
  const int Rating::MinimumRating = 1;

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Rating::UpdateDerivedRating( const bool derived )
  {
    this->AssertPrimaryId();

    vtkVariant userId = this->Get( "UserId" );
    vtkVariant imageId = this->Get( "ImageId" );
    if( userId.IsValid() && imageId.IsValid() );
    {
      int derivedRating = Rating::MaximumRating;
      //loop over all selected codes

      // get the ungrouped codes
      std::vector< vtkSmartPointer< Alder::Code > > codeList;
      vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
      modifier->Join( "CodeType", "CodeType.Id", "Code.CodeTypeId" );
      modifier->Where( "UserId", "=", userId.ToInt() );
      modifier->Where( "ImageId", "=", imageId.ToInt() );
      modifier->Where( "CodeType.CodeGroupId", "=", vtkVariant() );

      Alder::Code::GetAll( &codeList, modifier );
      for( auto it = codeList.begin(); it != codeList.end(); ++it )
      {
        vtkSmartPointer< Alder::CodeType > type;
        (*it)->GetRecord( type );
        derivedRating += type->Get("Value").ToInt();
      }

      codeList.clear();

      // get the grouped codes
      modifier->Reset();
      modifier->Join( "CodeType", "CodeType.Id", "Code.CodeTypeId" );
      modifier->Where( "UserId", "=", userId.ToInt() );
      modifier->Where( "ImageId", "=", imageId.ToInt() );
      modifier->Where( "CodeType.CodeGroupId", "!=", vtkVariant() );
      modifier->Group( "CodeType.CodeGroupId" );

      Alder::Code::GetAll( &codeList, modifier );
      for( auto it = codeList.begin(); it != codeList.end(); ++it )
      {
        vtkSmartPointer< Alder::CodeType > type;
        (*it)->GetRecord( type );
        vtkSmartPointer< Alder::CodeGroup > group;
        if( type->GetRecord( group ) )
        {
          derivedRating += group->Get("Value").ToInt();
        }
      }

      if( Rating::MinimumRating > derivedRating ) derivedRating = Rating::MinimumRating;
      if( Rating::MaximumRating < derivedRating ) derivedRating = Rating::MaximumRating;

      this->Set( "DerivedRating", derivedRating );
      if( derived )
        this->Set( "Rating", derivedRating );
      this->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<std::string, int> Rating::GetNumberOfRatings( User* user )
  {
    if( !user ) throw std::runtime_error( "Tried to get rating counts for null user" );

    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT Modality.Name, COUNT(*) AS Count "
           << "FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "JOIN ScanType ON ScanType.Id=Exam.ScanTypeId "
           << "JOIN Modality ON Modality.Id=ScanType.ModalityId "
           << "WHERE Rating.UserId = " << user->Get("Id").ToString() << " "
           << "GROUP BY Modality.Name";
    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    std::map<std::string, int> ratings;
    while( query->NextRow() )
    {
      std::string modalityName = query->DataValue(0).ToString();
      ratings[modalityName] = query->DataValue(1).ToInt();
    }
    return ratings;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::map<std::string,std::string>> Rating::GetRatingReportData( User* user,
    const std::string modality )
  {
    if( !user ) throw std::runtime_error( "Tried to get rating data for null user" );

    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT "
           << "User.Name AS RATER, "
           << "IFNULL(Rating.Rating,'NA') AS RATING, "
           << "IFNULL(Rating.DerivedRating,'NA') AS DERIVED, "
           << "IFNULL(x.Code,'NONE') AS CODE, "
           << "ScanType.Type AS TYPE, "
           << "Exam.Laterality AS SIDE, "
           << "Interview.UId AS UID, "
           << "Interview.VisitDate AS VISITDATE, "
           << "Interview.Site AS SITE, "
           << "Exam.Interviewer AS INTERVIEWER "
           << "FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "JOIN ScanType ON ScanType.Id=Exam.ScanTypeId "
           << "JOIN Modality ON Modality.Id=ScanType.ModalityId "
           << "JOIN Interview ON Interview.Id=Exam.InterviewId "
           << "JOIN User ON User.Id=Rating.UserId "
           << "LEFT JOIN ( "
           << "  SELECT "
           << "  GROUP_CONCAT( Code SEPARATOR ',' ) AS Code, "
           << "  ImageId "
           << "  FROM Code "
           << "  JOIN CodeType ON CodeType.Id=Code.CodeTypeId "
           << "  WHERE Code.UserId=" << user->Get("Id").ToString() << " "
           << "  ) x ON x.ImageId=Rating.ImageId "
           << "WHERE User.Id=" << user->Get("Id").ToString() << " ";

    if( !modality.empty() )
    {
      stream << "AND Modality.Name='" << modality << "' ";
    }
    stream << "ORDER BY SITE, VISITDATE, UID";

    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    std::vector<std::map<std::string,std::string>> ratings;
    std::map<int,std::string> queryMap = {
      {0,"RATER"},
      {1,"RATING"},
      {2,"DERIVED"},
      {3,"CODE"},
      {4,"TYPE"},
      {5,"SIDE"},
      {6,"UID"},
      {7,"VISITDATE"},
      {8,"SITE"},
      {9,"INTERVIEWER"}
    };

    while( query->NextRow() )
    {
      std::map<std::string,std::string> tmp;
      for( auto it = queryMap.cbegin(); it != queryMap.cend(); ++it )
      {
        tmp[it->second] = query->DataValue(it->first).ToString();
      }
      ratings.push_back(tmp);
    }

    return ratings;
  }
}
