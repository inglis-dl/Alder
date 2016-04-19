/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeType.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <CodeType.h>
#include <Rating.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( CodeType );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int CodeType::GetUsage()
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "JOIN Code ON Code.CodeTypeId=CodeType.Id "
           << "JOIN Image ON Image.Id=Code.ImageId "
           << "JOIN User ON User.Id=Code.UserId "
           << "JOIN Rating ON Rating.ImageId=Image.Id "
           << "AND Rating.UserId=User.Id "
           << "WHERE CodeType.Id=" << this->Get("Id").ToString();

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
    // only has one row
    query->NextRow();
    return query->DataValue( 0 ).ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void CodeType::GetUsageById( std::map<int,int>& codeIdUsageMap )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT CodeType.Id, SUM(IF(Code.Id IS NULL, 0, 1)) AS Count "
           << "FROM CodeType "
           << "LEFT JOIN Code ON Code.CodeTypeId=CodeType.Id "
           << "LEFT JOIN Image ON Image.Id=Code.ImageId "
           << "LEFT JOIN User ON User.Id=Code.UserId "
           << "LEFT JOIN Rating ON Rating.ImageId=Image.Id "
           << "AND Rating.UserId=User.Id "
           << "GROUP BY CodeType.Id";

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    codeIdUsageMap.clear();
    while( query->NextRow() )
    {
      codeIdUsageMap[query->DataValue(0).ToInt()] = query->DataValue(1).ToInt();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool CodeType::IsUnique( const std::string& code, const int& value, const int& groupId )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "WHERE Code='" << code << "' "
           << "AND Value=" << vtkVariant(value).ToString() << " ";

    if( -1 == groupId )
      stream << "AND CodeGroupId IS NULL";
    else
      stream << "AND CodeGroupId=" << vtkVariant(groupId).ToString();

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
    // only has one row
    query->NextRow();
    return (0 == query->DataValue( 0 ).ToInt());
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void CodeType::UpdateRatings()
  {
    std::string Id = this->Get( "Id" ).ToString();
    Application *app = Application::GetInstance();
    int threshold = Rating::MinimumRating - Rating::MaximumRating;
    std::stringstream stream;
    stream  << "UPDATE Rating "
            << "JOIN ( "
            << "  SELECT RatingId, "
            << "  IF(SUM(Value)<"
            <<    threshold << "," << Rating::MinimumRating << "," << Rating::MaximumRating
            << "  +SUM(Value)) AS NewDerived, "
            << "  DerivedRating, "
            << "  MatchedRating "
            << "  FROM ( "
            << "    ( SELECT "
            << "      r.Id AS RatingId, "
            << "      SUM(t.Value) AS Value, "
            << "      r.DerivedRating, "
            << "      IF(r.DerivedRating=r.Rating,1,0) AS MatchedRating "
            << "      FROM Rating r "
            << "      JOIN Image i ON i.Id=r.ImageId "
            << "      JOIN Code c ON c.ImageId=r.ImageId "
            << "      JOIN CodeType t ON t.Id=c.CodeTypeId "
            << "      JOIN ( "
            << "        SELECT DISTINCT r.Id "
            << "        FROM Rating r "
            << "        JOIN Code c ON c.ImageId=r.ImageId "
            << "        WHERE c.CodeTypeId=" << Id
            << "      ) s ON s.Id=r.Id "
            << "      WHERE t.CodeGroupId IS NULL "
            << "      GROUP BY r.Id "
            << "    ) "
            << "    UNION ALL "
            << "    ( SELECT RatingId, "
            << "      SUM(valueG) AS Value, "
            << "      DerivedRating, "
            << "      MatchedRating "
            << "      FROM ( "
            << "        SELECT "
            << "        r.Id AS RatingId, "
            << "        g.Id AS GroupId, "
            << "        g.Value AS ValueG, "
            << "        r.DerivedRating, "
            << "        IF(r.DerivedRating=r.Rating,1,0) AS MatchedRating "
            << "        FROM Rating r "
            << "        JOIN Image i ON i.Id=r.ImageId "
            << "        JOIN Code c ON c.ImageId=r.ImageId "
            << "        JOIN CodeType t ON t.Id=c.CodeTypeId "
            << "        JOIN CodeGroup g ON g.Id=t.CodeGroupId "
            << "        JOIN ( "
            << "          SELECT DISTINCT r.Id "
            << "          FROM Rating r "
            << "          JOIN Code c ON c.ImageId=r.ImageId "
            << "          WHERE c.CodeTypeId=" << Id
            << "        ) s ON s.Id=r.Id "
            << "        GROUP BY r.Id, g.Id "
            << "      ) AS x1 GROUP BY ratingId "
            << "    ) ORDER BY RatingId "
            << "  ) AS x2 GROUP BY RatingId "
            << ") AS x3 ON x3.RatingId=Rating.Id "
            << "SET Rating.DerivedRating=x3.Newderived, "
            << "Rating.Rating=IF(x3.MatchedRating,x3.NewDerived,Rating.Rating) "
            << "WHERE x3.Newderived!=Rating.DerivedRating";

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
  }
}
