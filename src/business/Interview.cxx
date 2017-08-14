/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Interview.h>

// Alder includes
#include <Application.h>
#include <Common.h>
#include <Exam.h>
#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>
#include <Site.h>
#include <User.h>
#include <Utilities.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// C++ includes
#include <algorithm>
#include <list>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(Interview);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Interview> Interview::GetNeighbour(
    const bool& forward, const bool& loaded, const bool& unRated)
  {
    Application* app = Application::GetInstance();
    vtkVariant currentId = this->Get("Id");
    std::string uId = this->Get("UId").ToString();

    Alder::User* user = app->GetActiveUser();
    std::string userId = user->Get("Id").ToString();
    std::stringstream stream;
    bool simple = !loaded && !unRated;

    // use a special query to quickly get the next interview
    if (simple)
    {
      stream << "SELECT Id FROM Interview ";
    }
    else
    {
      stream
        << "SELECT Interview.Id, UId "
        << "FROM User "
        << "CROSS JOIN Interview "
        << "JOIN Exam ON Interview.Id = Exam.InterviewId "
        << "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
        << "JOIN UserHasModality ON ScanType.ModalityId = UserHasModality.ModalityId "
        << "AND UserHasModality.UserId = User.Id "
        << "JOIN Image ON Image.ExamId = Exam.Id "
        << "WHERE User.Id = " << userId << " ";
      if (loaded)
      {
        stream
          << "AND Stage = 'Completed' AND Downloaded = true ";
      }
      stream
        << "GROUP BY Interview.Id "
        << "UNION SELECT " << currentId.ToString() << ", '" << uId << "' ";
    }

    // order the query by UId (descending if not forward)
    //
    stream << "ORDER BY UId ";
    if (!forward) stream << "DESC ";

    app->Log("Querying Database (Interview::GetNeighbour): " + stream.str());
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();

    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    std::list<int> idList;
    while (query->NextRow())
    {
      vtkVariant vId = query->DataValue(0);
      if (vId.IsValid())
      {
        idList.push_back(vId.ToInt());
      }
    }

    // iterator to the current interview Id in the list
    //
    std::list<int>::iterator currentIt = find(idList.begin(), idList.end(), currentId.ToInt());

    // append the list with all preceding Ids
    //
    if (currentIt != idList.begin())
    {
      std::list<int> append;
      std::copy(idList.begin(), currentIt, std::back_inserter(append));
      idList.insert(idList.end(), append.begin(), append.end());
    }

    // iterator from the current Id forward until a qualifying interview is found
    //
    bool found = false;
    while (++currentIt != idList.end() && !found)
    {
      interview->Load("Id", *currentIt);
      if (uId != interview->Get("UId").ToString())
      {
        if( unRated && interview->IsRatedBy(user))
        {
          continue;
        }
        found = true;
      }
    }
    if(!found) interview->Load("Id", currentId.ToString());
    return interview;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::GetImageCount()
  {
    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM Image "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "WHERE Exam.InterviewId=" << this->Get("Id").ToString();

    Application* app = Application::GetInstance();
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();

    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    // only one row
    //
    query->NextRow();
    return query->DataValue(0).ToInt();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::IsRatedBy(User* user)
  {
    if (!user)
      throw std::runtime_error("Tried to get rating for null user");

    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN User ON User.Id=Rating.UserId "
           << "JOIN Exam ON Exam.Id=Image.ExamId "
           << "WHERE Exam.InterviewId=" << this->Get("Id").ToString() << " "
           << "AND User.Id=" << user->Get("Id").ToString();

    Application* app = Application::GetInstance();
    vtkSmartPointer<vtkMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery(stream.str().c_str());
    query->Execute();

    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    // only one row
    //
    query->NextRow();
    return 0 < query->DataValue(0).ToInt();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Interview::HasExamData()
  {
    return 0 < this->GetCount("Exam");
  }

  /**
   * caller: accepted
   * source: interface/qt/model/QSelectInterviewDialog.cxx
   * caller: setActiveInterview
   * source: interface/qt/model/QAlderInterviewWidget.cxx
   *
   * ImageStatus are:
   * None (no image records)
   * Pending (exams with image records but no image files)
   * Complete (exams with image records with image files)
   */
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Interview::ImageStatus Interview::GetImageStatus(QueryModifier* modifier)
  {
    std::vector<vtkSmartPointer<Exam>> vecExam;
    this->GetList(&vecExam, modifier);
    Interview::ImageStatus status = Interview::ImageStatus::None;
    if (vecExam.empty())
      return status;

    int downloadCount = 0;
    int pendingCount = 0;
    int partialCount = 0;
    int missingCount = 0;
    int examCount = 0;
    for (auto it = vecExam.cbegin(); it != vecExam.cend(); ++it, ++examCount)
    {
      // the exam has valid DatetimeAcquired, stage Completed
      //
      if ((*it)->HasImageData())
      {
        int current = (*it)->GetCurrentImageCount();
        int expected = (*it)->GetExpectedImageCount();
        if (current == expected)
        {
          downloadCount++;
        }
        else
        {
          if(0 == current)
            pendingCount++;
          else
            partialCount++;
        }
      }
      else
        missingCount++;
    }

    // return status by precedence
    //
    if (missingCount == examCount) // none of the exams is valid
      status = Interview::ImageStatus::None;
    else if (0 < pendingCount)  // image records exist, no file downloaded
      status = Interview::ImageStatus::Pending;
    else if (0 < partialCount)  // image records exist, some files downloaded
      status = Interview::ImageStatus::Partial;
    else if (0 < downloadCount)  // image records exist, all files downloaded
      status = Interview::ImageStatus::Complete;

    return status;
  }

  /**
   * caller: selectionChanged
   * source: interface/qt/model/QSelectInterviewDialog.cxx
   * caller: setActiveInterview
   * source: interface/qt/model/QAlderInterviewWidget.cxx
   * caller: UpdateInterviewData, LoadFromList
   * srouce: business/Interview.cxx
   */
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateExamData(Wave* aWave,
    const std::string& aMetaSource)
  {
    Application* app = Application::GetInstance();
    OpalService* opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if (!sustain)
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch (std::runtime_error& e)
      {
        app->Log(e.what());
        return;
      }
    }

    // get the Wave associated with this Interview
    //
    vtkSmartPointer<Wave> wave;
    if (NULL == aWave)
    {
      if (!this->GetRecord(wave))
        throw std::runtime_error("Interview has no Wave record!");
    }
    else
      wave = aWave;

    std::string source = aMetaSource;
    if (source.empty())
      source = wave->Get("MetaDataSource").ToString();

    // get the Opal Exam table data for the given UId
    //
    std::string identifier = this->Get("UId").ToString();

    // Opal data are Exam table variable name key value pairs
    //
    std::map<std::string, std::string> opalData =
      opal->GetRow(source, "Exam", identifier);

    if (!sustain)
      opal->SustainConnectionOff();

    if (opalData.empty())
    {
      return;
    }

    std::string interviewId = this->Get("Id").ToString();

    // build a map of Alder Exam db table rows
    // and populate with values particular to the given UId
    //
    std::map<std::string, std::map<std::string, std::string>> examData;
    std::vector<vtkSmartPointer<ScanType>> scanTypeList;
    wave->GetList(&scanTypeList);
    for (auto it = scanTypeList.cbegin(); it != scanTypeList.cend(); ++it)
    {
      std::string typeStr = (*it)->Get("Type").ToString();
      std::string idStr   = (*it)->Get("Id").ToString();
      std::string sideStr = (*it)->Get("SideCount").ToString();
      examData[typeStr]["ScanTypeId"] = idStr;
      examData[typeStr]["SideCount"] = sideStr;
    }

    // parse the Opal data and populate the map by ScanType
    //
    for (auto it = opalData.cbegin(); it != opalData.cend(); ++it)
    {
      std::string opalVar = it->first;
      std::string opalVal = it->second;
      // The Opal Exam table contains variable names of the form
      // Type.VariableName (eg.,Type.Stage, Type.Interviewer etc.)
      // where Type is the value of Alder ScanType table column Type
      // and VariableName is the value of an Exam table column
      //
      std::vector<std::string> tmp = Utilities::explode(opalVar, ".");
      if (2 != tmp.size())
      {
        continue;
      }
      std::string type = tmp.front();
      std::string var  = tmp.back();

      if (examData.find(type) == examData.end())
      {
        continue;
      }
      examData[type][var] = opalVal;
    }

    std::string noneStr = "none";
    std::string rightStr = "right";
    std::string leftStr = "left";

    for (auto it = examData.cbegin(); it != examData.cend(); ++it)
    {
      std::string type = it->first;
      std::map<std::string, std::string> columns = it->second;
      std::map<std::string, std::string> mapSide;
      int sideCount = vtkVariant(columns["SideCount"]).ToInt();
      if (0 == sideCount)
      {
        mapSide[noneStr] = "0";
      }
      else
      {
        if (columns.find("Side") == columns.end() ||
            columns["Side"].empty())
          continue;

        if (1 == sideCount)
        {
          mapSide[columns["Side"]] = "0";
        }
        else if (2 == sideCount)
        {
          if (columns.find("SideIndex") == columns.end() ||
              columns["SideIndex"].empty())
            continue;

          std::vector<std::string> vecSide =
            Utilities::explode(columns["Side"], ",");

          std::vector<std::string> vecSideIndex =
            Utilities::explode(columns["SideIndex"], ",");

          if (vecSide.size() != vecSideIndex.size())
            continue;

          auto sit = vecSide.cbegin();
          auto iit = vecSideIndex.cbegin();

          for (; sit != vecSide.cend(), iit != vecSideIndex.cend();
                ++sit, ++iit)
          {
            mapSide[*sit] = *iit;
          }
        }
      }

      std::map<std::string, std::string> loader;
      loader["ScanTypeId"] = columns["ScanTypeId"];
      loader["InterviewId"] = interviewId;
      for (auto mit = mapSide.cbegin(); mit != mapSide.cend(); ++mit)
      {
        std::string sideStr = mit->first;
        std::string indexStr = mit->second;
        if (0 == sideCount)
        {
          if (noneStr != sideStr)
            continue;
        }
        else
        {
          if (leftStr != sideStr && rightStr != sideStr)
            continue;
        }

        loader["Side"] = sideStr;
        vtkNew<Exam> exam;
        if (exam->Load(loader))
        {
          // check and update data as required
          //
          bool save = false;
          std::string value = columns["Stage"];
          if (exam->Get("Stage").ToString() != value)
          {
            exam->Set("Stage", value);
            save = true;
          }

          value = columns["Interviewer"];
          if (exam->Get("Interviewer").ToString() != value)
          {
            exam->Set("Interviewer", value);
            save = true;
          }

          value = columns["DatetimeAcquired"];
          if (exam->Get("DatetimeAcquired").ToString() != value)
          {
            exam->Set("DatetimeAcquired", value);
            save = true;
          }

          if (1 < sideCount && exam->Get("SideIndex").ToString() != indexStr)
          {
            exam->Set("SideIndex", indexStr);
            save = true;
          }

          if (save)
          {
            exam->Save();
          }
        }
        else
        {
          exam->Set("InterviewId", interviewId);
          exam->Set("ScanTypeId", columns["ScanTypeId"]);
          if (0 < sideCount)
          {
            exam->Set("Side", sideStr);
            if (1 < sideCount)
              exam->Set("SideIndex", indexStr);
          }
          exam->Set("Stage", columns["Stage"]);
          exam->Set("Interviewer", columns["Interviewer"]);
          exam->Set("DatetimeAcquired", columns["DatetimeAcquired"]);
          exam->Save();
        }
      }
    }
  }

  /**
   * caller: accepted
   * source: interface/qt/model/QSelectInterviewDialog.cxx
   * caller: setActiveInterview
   * source: interface/qt/model/QAlderInterviewWidget.cxx
   */
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateImageData()
  {
    Application* app = Application::GetInstance();
    User* user = app->GetActiveUser();
    std::vector<vtkSmartPointer<Exam>> vecExam;
    if (!user) return;

    std::stringstream stream;
    stream << "SELECT Exam.* FROM Exam "
           << "JOIN ScanType ON Exam.ScanTypeId=ScanType.Id "
           << "JOIN ("
           << "SELECT Modality.Id FROM Modality "
           << "JOIN UserHasModality ON UserHasModality.ModalityId=Modality.Id "
           << "JOIN User ON User.Id=UserHasModality.UserId "
           << "WHERE User.Id=" << user->Get("Id").ToString() << " "
           << ") AS x ON x.Id=ScanType.ModalityId "
           << "WHERE DatetimeAcquired IS NOT NULL "
           << "AND Stage='Completed' "
           << "AND Downloaded=0 "
           << "AND InterviewId=" << this->Get("Id").ToString();

    Database* db = app->GetDB();
    vtkSmartPointer<vtkMySQLQuery> query = db->GetQuery();

    app->Log("Querying Database: " + stream.str());
    query->SetQuery(stream.str().c_str());
    query->Execute();

    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    while (query->NextRow())
    {
      vtkSmartPointer<Exam> exam = vtkSmartPointer<Exam>::New();
      exam->LoadFromQuery(query);
      vecExam.push_back(exam);
    }

    if (!vecExam.empty())
    {
      vtkSmartPointer<Wave> wave;
      this->GetRecord(wave);

      std::string source = wave->Get("ImageDataSource").ToString();
      std::string identifier = this->Get("UId").ToString();

      int index = 0;
      int lastProgress = 0;
      int progress = 0;
      double progressScale = 100.0/vecExam.size();
      double progressValue = 0.;
      std::string message = "Updating image data of ";
      message += std::to_string(static_cast<int>(vecExam.size())) + " exams";
      app->SetAbortFlag(0);
      app->InvokeEvent(
        vtkCommand::StartEvent,
        reinterpret_cast<void*>(const_cast<char*>(message.c_str())));

      // loop over the exams, updating images based on UID and the Opal table
      // that contains the images for the current wave
      //
      for (auto it = vecExam.cbegin(); it != vecExam.cend(); ++it, ++index)
      {
        if (app->GetAbortFlag())
          break;
        progress = static_cast<int>(index*progressScale);
        if (lastProgress != progress)
        {
          progressValue = 0.01*progress;
          app->InvokeEvent(
            vtkCommand::ProgressEvent, reinterpret_cast<void*>(&progressValue));
          lastProgress = progress;
        }
        try
        {
          if (!(*it)->DownloadComplete())
          {
            (*it)->UpdateImageData(identifier, source);
          }
        }
        catch (std::runtime_error& e)
        {
          app->Log(e.what());
        }
      }
      app->InvokeEvent(vtkCommand::EndEvent);
    }
  }

  /**
   * caller: adminUpdateInterviewDatabase()
   * source: /src/interface/qt/application/QMainAlderWindow.cxx
   */
  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Interview::UpdateInterviewData(
    const std::vector<std::pair< int, bool>>& waveList)
  {
    Application* app = Application::GetInstance();
    OpalService* opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if (!sustain)
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch (std::runtime_error& e)
      {
        app->Log(e.what());
        return;
      }
    }

    // loop over requested Waves and determine the total number of
    // participant identifiers to be updated
    //
    double size = 0.;
    std::map<std::string,        // key Wave.Id
      std::vector<std::string>>  // value vector of Interview.UId
        mapWave;
    for (auto it = waveList.cbegin(); it != waveList.cend(); ++it)
    {
      std::string waveId = vtkVariant(it->first).ToString();
      bool fullUpdate = it->second;
      vtkNew<Wave> wave;
      if (!wave->Load("Id", waveId)) continue;

      // get the Opal view that contains the Interview, Exam, and ScanType tables
      //
      std::string source = wave->Get("MetaDataSource").ToString();

      // get and sort the list of UId's available for the current Wave
      //
      std::vector<std::string> vecOpal =
        opal->GetIdentifiers(source, "Interview");

      if (vecOpal.empty()) continue;

      std::sort(vecOpal.begin(), vecOpal.end());
      std::vector<std::string> vecUId;

      // if we are doing a partial update,
      // skip any existing identifiers that already have Exam meta data
      //
      if (!fullUpdate)
      {
        std::stringstream stream;
        stream << "SELECT DISTINCT UId FROM Interview "
               << "JOIN Exam ON Exam.InterviewId=Interview.Id "
               << "WHERE WaveId=" << waveId << " "
               << "ORDER BY UId ";
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

        while (query->NextRow())
        {
          std::string uidStr  = query->DataValue(0).ToString();
          vecUId.push_back(uidStr);
        }
      }

      // there are no Exams records joined to the Interview table
      // use all the identifiers retrieved from Opal
      //
      if (vecUId.empty())
      {
        mapWave[waveId] = vecOpal;
      }
      else
      {
        // the maximum number of identifiers comes from Opal
        //
        std::vector<std::string> vecWave(vecOpal.size());

        // get the set difference such that only new Opal identifiers remain
        //
        std::vector<std::string>::iterator vit =
          std::set_difference(
            vecOpal.begin(), vecOpal.end(),
            vecUId.begin(), vecUId.end(), vecWave.begin());
        vecWave.resize(vit - vecWave.begin());

        mapWave[waveId] = vecWave;
      }

      size += mapWave[waveId].size();
    } // end loop on Waves

    if (0. == size)
    {
      if (!sustain)
        opal->SustainConnectionOff();
      return;
    }

    // get a map of site ids, names and aliases
    //
    std::vector<vtkSmartPointer<Site>> vecSite;
    Site::GetAll(&vecSite);
    std::map<std::string, std::string> mapSite;
    for (auto it = vecSite.cbegin(); it != vecSite.cend(); ++it)
    {
      Site* site = *it;
      std::string siteId = site->Get("Id").ToString();
      mapSite[site->Get("Name").ToString()] = siteId;
      std::string alias = site->Get("Alias").ToString();
      if (!alias.empty())
        mapSite[alias] = siteId;
    }

    // determine number of identifiers to pull per Opal curl call
    //
    int limit = static_cast<int>(0.1*size);
    limit = limit > 500 ? 500 : (limit < 10 ? 10 : limit);
    int index = 0;
    int progress = 0;
    int lastProgress = progress;
    double progressScale = 100.0/size;
    double progressValue = 0.;
    bool done = false;
    vtkSmartPointer<Wave> wave = vtkSmartPointer<Wave>::New();
    std::string message = "Updating metadata of ";
    message += std::to_string(static_cast<int>(size)) + " interviews";
    app->SetAbortFlag(0);
    app->InvokeEvent(
      vtkCommand::StartEvent,
      reinterpret_cast<void*>(const_cast<char*>(message.c_str())));

    // loop over Waves, updating records by participant identifier
    //
    for (auto it = mapWave.cbegin(); it != mapWave.cend() && !done; ++it)
    {
      if (app->GetAbortFlag())
      {
         break;
      }
      std::string waveId = it->first;
      wave->Load("Id", waveId);
      std::string source = wave->Get("MetaDataSource").ToString();

      std::vector<std::string> vecUId = it->second;
      std::vector<std::string>::iterator ibegin = vecUId.begin();
      std::vector<std::string>::iterator iend = vecUId.end();

      // storage structure of participant identifier key (UId) with value
      // map of Opal variable name key value pairs
      //
      std::map<std::string, std::map<std::string, std::string>> mapOpal;
      int localIndex = 0;
      do
      {
        progress = static_cast<int>(progressScale*index);
        if (lastProgress != progress)
        {
          progressValue = 0.01*progress;
          app->InvokeEvent(
            vtkCommand::ProgressEvent, reinterpret_cast<void*>(&progressValue));
          lastProgress = progress;
        }

        mapOpal = opal->GetRows(source, "Interview", localIndex, limit);
        for (auto mit = mapOpal.cbegin(); mit != mapOpal.cend(); ++mit)
        {
          if (app->GetAbortFlag())
          {
            done = true;
            break;
          }
          // skip identifiers that are not in the requested update list
          //
          std::string uidStr = mit->first;
          if (iend == std::find(ibegin, iend, uidStr))
            continue;

          // if found, try to load the interview
          //
          std::map<std::string, std::string> mapVar = mit->second;

          // create a unique value map to load the interview
          //
          std::map<std::string, std::string> loader;
          loader["WaveId"] = waveId;
          loader["UId"] = uidStr;

          vtkNew<Interview> interview;
          if (!interview->Load(loader))
          {
            // create the new interview
            //
            std::string siteStr = mapVar["Site"];
            std::string siteId;
            auto sit = mapSite.find(siteStr);
            if (sit == mapSite.end())
            {
              vtkNew<Site> site;
              if (site->Load("Name", siteStr))
                siteId = site->Get("Id").ToString();
              else if (site->Load("Alias", siteStr))
                siteId = site->Get("Id").ToString();
              else
              {
                site->Set("Name", siteStr);
                site->Save();
                siteId = site->Get("Id").ToString();
              }
              mapSite[siteStr] = siteId;
            }
            else
            {
              siteId = sit->second;
            }
            loader["SiteId"] = siteId;
            loader["Barcode"] = mapVar["Barcode"];
            loader["VisitDate"] = mapVar["VisitDate"];

            interview->Set(loader);
            interview->Save();
            interview->Load(loader);
          }
          else
          {
            bool modify = false;
            if(interview->Get("Barcode").ToString() != mapVar["Barcode"])
            {
              interview->Set("Barcode",mapVar["Barcode"]);
              modify = true;
            }
            if(interview->Get("VisitDate").ToString() != mapVar["VisitDate"])
            {
              interview->Set("VisitDate",mapVar["VisitDate"]);
              modify = true;
            }
            if(modify)
            {
              interview->Save();
            }
          }

          // update the Exam records linked to the interview
          //
          interview->UpdateExamData(wave, source);
          index++;
        }

        localIndex += mapOpal.size();
      } while (!mapOpal.empty() && !done);
    }
    app->InvokeEvent(vtkCommand::EndEvent);

    if (!sustain)
      opal->SustainConnectionOff();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Interview::LoadFromList(
    const std::vector<std::pair<std::string, std::string>>& list)
  {
    // load images for Interviews already pulled from Opal
    // via the administrator UI
    //
    Application* app = Application::GetInstance();
    std::map<int, vtkSmartPointer<Interview>> revised;
    std::map<std::string, vtkSmartPointer<Wave>> waveMap;
    vtkSmartPointer<Wave> wave = vtkSmartPointer<Wave>::New();

    // loop over UID, wave rank pairs
    //
    for (auto it = list.cbegin(); it != list.cend(); ++it)
    {
      std::string uidStr = it->first;
      std::string rankStr = it->second;
      // populate the rank/wave map as needed
      //
      if (waveMap.end() == waveMap.find(rankStr))
      {
        if (wave->Load("Rank", rankStr))
          waveMap[rankStr] = wave;
        else
        {
          std::string err = "Error: failed to load UID, wave rank pair: ";
          err += uidStr;
          err += ", ";
          err += rankStr;
          app->Log(err);
          continue;
        }
      }
      else
        wave = waveMap[rankStr];

      vtkSmartPointer<Interview> interview =
        vtkSmartPointer<Interview>::New();
      std::map<std::string, std::string> loader;
      loader["UId"] = uidStr;
      loader["WaveId"] = wave->Get("Id").ToString();
      //populate the interview list as needed
      //
      if (interview->Load(loader))
      {
        int id = interview->Get("Id").ToInt();
        if (revised.end() == revised.find(id))
          revised[id] = interview;
      }
    }

    // no interviews were found
    //
    if (revised.empty()) return 0;

    OpalService* opal = app->GetOpal();
    bool sustain = opal->GetSustainConnection();
    if (!sustain)
    {
      try
      {
        opal->SustainConnectionOn();
      }
      catch (std::runtime_error& e)
      {
        app->Log(e.what());
        return 0;
      }
    }

    // the active user can only load exam images they have permission for
    //
    User* user = app->GetActiveUser();

    vtkSmartPointer<QueryModifier> modifier =
      vtkSmartPointer<QueryModifier>::New();
    user->InitializeExamModifier(modifier);

    int index = 0;
    int lastProgress = 0;
    int progress = 0;
    double size = static_cast<double>(revised.size());
    double progressScale = 100.0/size;
    double progressValue = 0.;
    std::string message = "Downloading images of ";
    message += std::to_string(static_cast<int>(size)) + " interviews";
    app->SetAbortFlag(0);
    app->InvokeEvent(
      vtkCommand::StartEvent,
      reinterpret_cast<void*>(const_cast<char*>(message.c_str())));
    for (auto it = revised.cbegin(); it != revised.cend(); ++it, ++index)
    {
      if (app->GetAbortFlag())
      {
        break;
      }

      progress = static_cast<int>(progressScale*index);
      if (lastProgress != progress)
      {
        progressValue = 0.01*progress;
        app->InvokeEvent(
          vtkCommand::ProgressEvent, reinterpret_cast<void*>(&progressValue));
        lastProgress = progress;
      }

      Interview* interview = it->second;
      vtkSmartPointer<Wave> wave;
      interview->GetRecord(wave);
      // update the Exam db records attributed to this Interview
      //
      try
      {
        interview->UpdateExamData(wave);
      }
      catch (std::runtime_error& e)
      {
        std::string err =
          "There was an error while trying to update exam data (UId : ";
        err += interview->Get("UId").ToString();
        err += "). Error: ";
        err += e.what();
        app->Log(err);
        continue;
      }
      // get the list of exams this user can retrieve images for
      //
      std::vector<vtkSmartPointer<Exam>> vecExam;
      interview->GetList(&vecExam, modifier);
      std::string source = wave->Get("ImageDataSource").ToString();
      std::string identifier = interview->Get("UId").ToString();
      for (auto vit = vecExam.cbegin(); vit != vecExam.cend(); ++vit)
      {
        try
        {
          if (!(*vit)->DownloadComplete())
          {
            (*vit)->UpdateImageData(identifier, source);
          }
        }
        catch (std::runtime_error& e)
        {
          std::string err =
            "There was an error while trying to update image data (UId : ";
          err += interview->Get("UId").ToString();
          err += "). Error: ";
          err += e.what();
          app->Log(err);
          continue;
        }
      }
    }
    app->InvokeEvent(vtkCommand::EndEvent);
    if (!sustain)
      opal->SustainConnectionOff();

    return index;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Interview::GetSimilarImageId(const std::string& imageId)
  {
    this->AssertPrimaryId();
    std::string matchId;
    if (imageId.empty()) return matchId;

    vtkNew<Image> image;
    image->Load("Id", imageId);
    bool hasParent = image->Get("ParentImageId").IsValid();

    std::stringstream stream;

    // given an image Id, find an image in this record having the same
    // characteristics
    //
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN Exam AS simExam ON Exam.ScanTypeId = simExam.ScanTypeId "
           << "AND Exam.ScanTypeId = simExam.ScanTypeId "
           << "AND Exam.Side = simExam.Side "
           << "AND Exam.Stage = simExam.Stage "
           << "JOIN Image AS simImage ON simImage.ExamId = simExam.Id "
           << "WHERE Exam.InterviewId = "
           << this->Get("Id").ToString()
           << " "
           << "AND simImage.Id = "
           << imageId
           << " "
           << "AND simImage.ParentImageId IS "
           << (hasParent ? "NOT NULL " : "NULL ")
           << "AND Image.ParentImageId IS "
           << (hasParent ? "NOT NULL " : "NULL ")
           << "LIMIT 1";

    Application* app = Application::GetInstance();
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

    if (query->NextRow())
    {
      matchId = query->DataValue(0).ToString();
    }
    return matchId;
  }
}  // namespace Alder
