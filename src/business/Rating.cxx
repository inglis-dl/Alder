/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Rating.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Rating.h>

// Alder includes
#include <Code.h>
#include <CodeType.h>
#include <CodeGroup.h>
#include <User.h>

// VTK includes
#include <vtkObjectFactory.h>

// C++ includes
#include <map>
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(Rating);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Rating::UpdateDerivedRating(const bool& derived)
  {
    this->AssertPrimaryId();

    vtkVariant userId = this->Get("UserId");
    vtkVariant imageId = this->Get("ImageId");
    if (userId.IsValid() && imageId.IsValid());
    {
      int derivedRating = Rating::MaximumRating;
      // loop over all selected codes

      // get the ungrouped codes
      std::vector<vtkSmartPointer<Code>> codeList;
      vtkSmartPointer<QueryModifier> modifier =
        vtkSmartPointer<QueryModifier>::New();
      modifier->Join("CodeType", "CodeType.Id", "Code.CodeTypeId");
      modifier->Where("UserId", "=", userId.ToInt());
      modifier->Where("ImageId", "=", imageId.ToInt());
      modifier->Where("CodeType.CodeGroupId", "=", vtkVariant());

      Code::GetAll(&codeList, modifier);
      for (auto it = codeList.begin(); it != codeList.end(); ++it)
      {
        vtkSmartPointer<CodeType> type;
        (*it)->GetRecord(type);
        derivedRating += type->Get("Value").ToInt();
      }

      codeList.clear();

      // get the grouped codes
      modifier->Reset();
      modifier->Join("CodeType", "CodeType.Id", "Code.CodeTypeId");
      modifier->Where("UserId", "=", userId.ToInt());
      modifier->Where("ImageId", "=", imageId.ToInt());
      modifier->Where("CodeType.CodeGroupId", "!=", vtkVariant());
      modifier->Group("CodeType.CodeGroupId");

      Code::GetAll(&codeList, modifier);
      for (auto it = codeList.begin(); it != codeList.end(); ++it)
      {
        vtkSmartPointer<CodeType> type;
        (*it)->GetRecord(type);
        vtkSmartPointer<CodeGroup> group;
        if (type->GetRecord(group))
        {
          derivedRating += group->Get("Value").ToInt();
        }
      }

      if (Rating::MinimumRating> derivedRating)
        derivedRating = Rating::MinimumRating;
      if (Rating::MaximumRating < derivedRating)
        derivedRating = Rating::MaximumRating;

      this->Set("DerivedRating", derivedRating);
      if (derived)
        this->Set("Rating", derivedRating);
      this->Save();
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Rating::UpdateRatings()
  {
    Application* app = Application::GetInstance();
    std::stringstream stream;
    int threshold = Rating::MinimumRating - Rating::MaximumRating;
    stream  << "UPDATE Rating "
            << "JOIN ("
            << "  SELECT RatingId, "
            << "  IF(SUM(Value)<"
            <<    threshold << ","
            <<    Rating::MinimumRating << ","
            <<    Rating::MaximumRating
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

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<std::string, int> Rating::GetNumberOfRatings(User* user)
  {
    if (!user)
      throw std::runtime_error("Tried to get rating counts for null user");

    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT Modality.Name, COUNT(*) AS Count "
           << "FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "JOIN ScanType ON ScanType.Id=Exam.ScanTypeId "
           << "JOIN Modality ON Modality.Id=ScanType.ModalityId "
           << "WHERE Rating.UserId = " << user->Get("Id").ToString() << " "
           << "GROUP BY Modality.Name";
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

    std::map<std::string, int> ratings;
    while (query->NextRow())
    {
      std::string modalityName = query->DataValue(0).ToString();
      ratings[modalityName] = query->DataValue(1).ToInt();
    }
    return ratings;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::map<std::string, std::string>> Rating::GetRatingReportData(
    User* user,  const std::string& modality)
  {
    if (!user)
      throw std::runtime_error("Tried to get rating data for null user");

    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT "
           << "User.Name AS RATER, "
           << "IFNULL(Rating.Rating,'NA') AS RATING, "
           << "IFNULL(Rating.DerivedRating,'NA') AS DERIVED, "
           << "IFNULL(x.Code,'NONE') AS CODE, "
           << "IFNULL(x.CodeID,'NONE') AS CODEID, "
           << "ScanType.Type AS TYPE, "
           << "Exam.Side AS SIDE, "
           << "Interview.UId AS UID, "
           << "Interview.VisitDate AS VISITDATE, "
           << "Site.Name AS SITE, "
           << "Exam.Interviewer AS INTERVIEWER, "
           << "Rating.CreateTimestamp AS RATING_CREATE_DATETIME, "
           << "Rating.UpdateTimestamp AS RATING_UPDATE_DATETIME "
           << "FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "JOIN ScanType ON ScanType.Id=Exam.ScanTypeId "
           << "JOIN Modality ON Modality.Id=ScanType.ModalityId "
           << "JOIN Interview ON Interview.Id=Exam.InterviewId "
           << "JOIN Site ON Site.Id=Interview.SiteId "
           << "JOIN User ON User.Id=Rating.UserId "
           << "LEFT JOIN ("
           << "  SELECT "
           << "  GROUP_CONCAT(DISTINCT Code ORDER BY Code SEPARATOR ',') AS Code, "
           << "  GROUP_CONCAT(DISTINCT CodeType.Id ORDER BY CodeType.Id SEPARATOR ',') AS CodeID, "
           << "  ImageId "
           << "  FROM Code "
           << "  JOIN CodeType ON CodeType.Id=Code.CodeTypeId "
           << "  JOIN Image ON Image.Id=Code.ImageId "
           << "  JOIN User ON User.Id=Code.UserId "
           << "  WHERE Code.UserId=" << user->Get("Id").ToString() << " "
           << "  GROUP BY Image.Id "
           << " ) x ON x.ImageId=Rating.ImageId "
           << "WHERE User.Id=" << user->Get("Id").ToString() << " ";

    if (!modality.empty())
    {
      stream << "AND Modality.Name='" << modality << "' ";
    }
    stream << "ORDER BY SITE, VISITDATE, UID";

    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();
    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    std::vector<std::map<std::string, std::string>> ratings;
    std::map<int, std::string> queryMap = {
      {0, "RATER"},
      {1, "RATING"},
      {2, "DERIVED"},
      {3, "CODE"},
      {4, "CODEID"},
      {5, "TYPE"},
      {6, "SIDE"},
      {7, "UID"},
      {8, "VISITDATE"},
      {9, "SITE"},
      {10, "INTERVIEWER"},
      {11, "RATING_CREATE_DATETIME"},
      {12, "RATING_UPDATE_DATETIME"}
    };

    while (query->NextRow())
    {
      std::map<std::string, std::string> tmp;
      for (auto it = queryMap.cbegin(); it != queryMap.cend(); ++it)
      {
        tmp[it->second] = query->DataValue(it->first).ToString();
      }
      ratings.push_back(tmp);
    }

    return ratings;
  }
}  // namespace Alder
