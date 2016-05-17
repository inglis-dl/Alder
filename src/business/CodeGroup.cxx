/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeGroup.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <CodeGroup.h>

// Alder includes
#include <Rating.h>

// VTK includes
#include <vtkObjectFactory.h>

// C++ includes
#include <string>

namespace Alder
{
  vtkStandardNewMacro(CodeGroup);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int CodeGroup::GetUsage()
  {
    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "WHERE CodeGroupId=" << this->Get("Id").ToString();

    app->Log("Querying Database: " + stream.str());
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();
    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }
    // only has one row
    query->NextRow();
    return query->DataValue(0).ToInt();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool CodeGroup::IsUnique(const std::string& name, const int& value)
  {
    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeGroup "
           << "WHERE Name='" << name << "' "
           << "AND Value=" << vtkVariant(value).ToString();

    app->Log("Querying Database: " + stream.str());
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();
    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }
    // only has one row
    query->NextRow();
    return (0 == query->DataValue(0).ToInt());
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void CodeGroup::UpdateRatings()
  {
    std::string Id = this->Get("Id").ToString();
    Application* app = Application::GetInstance();
    int threshold = Rating::MinimumRating - Rating::MaximumRating;
    std::stringstream stream;
    stream  << "UPDATE Rating "
            << "JOIN ("
            << "  SELECT RatingId, "
            << "  IF(SUM(Value)<"
            <<    threshold << ","
            <<    Rating::MinimumRating << "," << Rating::MaximumRating
            << "  +SUM(Value)) AS NewDerived, "
            << "  DerivedRating, "
            << "  MatchedRating "
            << "  FROM ("
            << "    (SELECT "
            << "      r.Id AS RatingId, "
            << "      SUM(t.Value) AS Value, "
            << "      r.DerivedRating, "
            << "      IF(r.DerivedRating=r.Rating,1,0) AS MatchedRating "
            << "      FROM Rating r "
            << "      JOIN Image i ON i.Id=r.ImageId "
            << "      JOIN Code c ON c.ImageId=r.ImageId "
            << "      JOIN CodeType t ON t.Id=c.CodeTypeId "
            << "      JOIN ("
            << "        SELECT DISTINCT r.Id "
            << "        FROM Rating r "
            << "        JOIN Code c ON c.ImageId=r.ImageId "
            << "        JOIN CodeType t ON t.Id=c.CodeTypeId "
            << "        WHERE t.CodeGroupId=" << Id
            << "     ) s ON s.Id=r.Id "
            << "      WHERE t.CodeGroupId IS NULL "
            << "      GROUP BY r.Id "
            << "   ) "
            << "    UNION ALL "
            << "    (SELECT RatingId, "
            << "      SUM(valueG) AS Value, "
            << "      DerivedRating, "
            << "      MatchedRating "
            << "      FROM ("
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
            << "        JOIN ("
            << "          SELECT DISTINCT r.Id "
            << "          FROM Rating r "
            << "          JOIN Code c ON c.ImageId=r.ImageId "
            << "          JOIN CodeType t ON t.Id=c.CodeTypeId "
            << "          WHERE t.CodeGroupId=" << Id
            << "       ) s ON s.Id=r.Id "
            << "        GROUP BY r.Id, g.Id "
            << "     ) AS x1 GROUP BY ratingId "
            << "   ) ORDER BY RatingId "
            << " ) AS x2 GROUP BY RatingId "
            << ") AS x3 ON x3.RatingId=Rating.Id "
            << "SET Rating.DerivedRating=x3.Newderived, "
            << "Rating.Rating=IF(x3.MatchedRating,x3.NewDerived,Rating.Rating) "
            << "WHERE x3.Newderived!=Rating.DerivedRating";

    app->Log("Querying Database: " + stream.str());
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();
    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }
  }
}  // namespace Alder
