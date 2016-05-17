/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Wave.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Wave.h>

// Alder includes
#include <CodeType.h>
#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>
#include <Utilities.h>

// VTK includes
#include <vtkObjectFactory.h>

// C++ includes
#include <map>
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(Wave);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Wave::UpdateScanTypeData()
  {
    Application* app = Application::GetInstance();
    OpalService* opal = app->GetOpal();

    // the list of ScanTypes by Name
    std::string source = this->Get("MetaDataSource").ToString();
    std::vector<std::string> vecOpal = opal->GetIdentifiers(source, "ScanType");
    if (vecOpal.empty())
    {
      std::string err = "Opal data source ";
      err += source;
      err += " is missing ScanType identifiers";
      app->Log(err);
      return;
    }

    std::vector<std::string> validate =
      app->GetDB()->GetColumnNames("ScanType");

    std::map<std::string, std::map<std::string, std::string>> mapOpal =
      opal->GetRows(source, "ScanType", 0, vecOpal.size());

    std::string waveId = this->Get("Id").ToString();
    vtkSmartPointer<QueryModifier> modifier =
      vtkSmartPointer<QueryModifier>::New();

    for (auto it = mapOpal.cbegin(); it != mapOpal.cend(); ++it)
    {
      std::string type = it->first;
      if (type.empty())
         continue;

      std::map<std::string, std::string> loader;
      loader["WaveId"] = waveId;
      loader["Type"] = type;
      bool create = true;
      vtkNew<ScanType> scanType;
      if (scanType->Load(loader))
      {
        create = false;
        loader.clear();
      }

      std::map<std::string, std::string> mapVar = it->second;
      for (auto mit = mapVar.cbegin(); mit != mapVar.cend(); ++mit)
      {
        std::string var = mit->first;
        std::string val = mit->second;
        if (std::find(validate.begin(), validate.end(), var) == validate.end())
        {
          // is the variable the modality name?
          std::vector<std::string> tmp = Utilities::explode(var, ".");
          if (2 == tmp.size() && "Modality" == tmp.front())
          {
            vtkNew<Modality> modality;
            if (!modality->Load("Name", val))
            {
              modality->Set("Name", val);
              modality->Save();
              modality->Load("Name", val);
            }
            loader["ModalityId"] = modality->Get("Id").ToString();
          }
        }
        else
        {
          if (create || (val != scanType->Get(var).ToString()))
            loader[var] = val;
        }
      }

      if (!loader.empty())
      {
        scanType->Set(loader);
        scanType->Save();
      }
      loader["WaveId"] = waveId;
      loader["Type"] = type;
      if (scanType->Load(loader))
      {
        // update the ScanTypeHasCodeType information
        // get the vector of CodeTypes by ScanType.Type
        modifier->Reset();
        modifier->Join(
          "ScanTypeHasCodeType", "CodeType.Id",
          "ScanTypeHasCodeType.CodeTypeId");
        modifier->Join(
          "ScanType", "ScanType.Id",
          "ScanTypeHasCodeType.ScanTypeId");
        modifier->Where("ScanType.Type", "=", scanType->Get("Type"));

        std::vector<vtkSmartPointer<CodeType>> codeTypeList;
        CodeType::GetAll(&codeTypeList, modifier);
        for (auto cit = codeTypeList.begin(); cit != codeTypeList.end(); ++cit)
        {
          scanType->AddRecord((*cit));
        }
      }
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Wave::UpdateWaveData(const std::string& source)
  {
    // get the waves from Opal
    // create or update as required
    if (source.empty()) return;

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

    // the list of Waves by Name
    std::vector<std::string> vecOpal = opal->GetIdentifiers(source, "Wave");
    if (vecOpal.empty())
    {
      std::string err = "Opal data source ";
      err += source;
      err += " is missing Wave identifiers";
      app->Log(err);
      if (!sustain)
        opal->SustainConnectionOff();
      return;
    }

    std::vector<std::string> validate = app->GetDB()->GetColumnNames("Wave");

    std::map<std::string, std::map<std::string, std::string>> mapOpal =
      opal->GetRows(source, "Wave", 0, vecOpal.size());

    for (auto it = mapOpal.cbegin(); it != mapOpal.cend(); ++it)
    {
      std::string name = it->first;
      if (name.empty())
         continue;

      std::map<std::string, std::string> mapVar = it->second;
      std::map<std::string, std::string> loader;
      loader["Name"] = name;
      vtkNew<Wave> wave;
      bool create = true;
      if (wave->Load("Name", name))
      {
        create = false;
        loader.clear();
      }

      for (auto mit = mapVar.cbegin(); mit != mapVar.cend(); ++mit)
      {
        std::string var = mit->first;
        std::string val = mit->second;
        if (std::find(validate.begin(), validate.end(), var) == validate.end())
        {
          std::string warning = "Invalid wave column '";
          warning += var;
          warning += "' in Opal source ";
          warning += source;
          app->Log(warning);
          continue;
        }
        else
        {
          if (create || (val != wave->Get(var).ToString()))
            loader[var] = val;
        }
      }

      if (!loader.empty())
      {
        wave->Set(loader);
        wave->Save();
        if (create)
          wave->Load(loader);
      }

      wave->UpdateScanTypeData();
    }

    if (!sustain)
      opal->SustainConnectionOff();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Wave::GetIdentifierCount()
  {
    std::string source = this->Get("MetaDataSource").ToString();
    if (source.empty())
      throw std::runtime_error(
        "MetaDataSource missing for Wave identifier count");

    Application* app = Application::GetInstance();
    OpalService* opal = app->GetOpal();
    std::vector<std::string> vecOpal =
      opal->GetIdentifiers(source, "Interview");
    return vecOpal.size();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Wave::GetMaximumInterviewUpdateTimestamp()
  {
    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT MAX(UpdateTimestamp) "
           << "FROM Interview "
           << "WHERE WaveId=" << this->Get("Id").ToString();
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

    // only one row
    query->NextRow();
    return query->DataValue(0).ToString();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int Wave::GetMaximumExamCount()
  {
    Application* app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT SUM(IF(SideCount=0,1,SideCount)) "
           << "FROM ScanType "
           << "WHERE WaveId="
           << this->Get("Id").ToString();
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

    // only one row
    query->NextRow();
    return query->DataValue(0).ToInt();
  }
}  // namespace Alder
