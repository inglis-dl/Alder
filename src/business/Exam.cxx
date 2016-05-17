/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Exam.h>

// Alder includes
#include <Application.h>
#include <CodeType.h>
#include <Image.h>
#include <Interview.h>
#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// C++ includes
#include <map>
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(Exam);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetCode()
  {
    vtkSmartPointer<Interview> interview;
    if (!this->GetRecord(interview))
      throw std::runtime_error("Exam has no parent interview");

    std::stringstream stream;
    stream << interview->Get("Id").ToString()
           << "/"
           << this->Get("Id").ToString();
    return stream.str();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::HasImageData()
  {
    std::string dateStr;
    std::string stageStr;
    std::string emptyDate = "0000-00-00 00:00:00";
    std::string validStage = "Completed";
    vtkVariant date = this->Get("DatetimeAcquired");
    if (date.IsValid())
      dateStr = date.ToString();
    if (dateStr.empty() ||  emptyDate == dateStr)
      return false;
    vtkVariant stage = this->Get("Stage");
    if (stage.IsValid())
      stageStr = stage.ToString();
    if (stageStr.empty() || validStage != stageStr)
      return false;
    return true;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetScanType()
  {
    vtkSmartPointer<ScanType> scanType;
    if (!this->GetRecord(scanType))
      throw std::runtime_error("Exam has no parent scantype");

    return scanType->Get("Type").ToString();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetModalityName()
  {
    vtkSmartPointer<ScanType> scanType;
    if (!this->GetRecord(scanType))
      throw std::runtime_error("Exam has no parent scantype");

    vtkSmartPointer<Modality> modality;
    if (!scanType->GetRecord(modality))
      throw std::runtime_error("Exam has parent scantype with no modality");

    return modality->Get("Name").ToString();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::UpdateImageData(const std::string& aIdentifier,
    const std::string& aSource)
  {
    vtkSmartPointer<Interview> interview;
    std::string identifier = aIdentifier;
    if (identifier.empty())
    {
      if (!this->GetRecord(interview))
        throw std::runtime_error("Exam has no parent interview");

      identifier = interview->Get("UId").ToString();
    }

    std::string source = aSource;
    if (source.empty())
    {
      vtkSmartPointer<Wave> wave;
      if (!aIdentifier.empty())
      {
        if (!this->GetRecord(interview))
          throw std::runtime_error("Exam has no parent interview");
      }
      interview->GetRecord(wave);
      source = wave->Get("ImageDataSource").ToString();
    }

    // get the ScanType parameters
    vtkSmartPointer<ScanType> scanType;
    if (!this->GetRecord(scanType))
      throw std::runtime_error("Exam has no parent scantype");

    std::string type = scanType->Get("Type").ToString();
    int sideCount    = scanType->Get("SideCount").ToInt();
    int acqCount     = scanType->Get("AcquisitionCount").ToInt();
    int childCount   = scanType->Get("ChildCount").ToInt();
    std::string acqNameFormat =
      scanType->Get("AcquisitionNameFormat").ToString();

    // get the position index within the Opal sequence of images
    int sideIndex = -1;
    if (1 < sideCount)
      sideIndex = this->Get("SideIndex").ToInt();

    std::string suffix = scanType->Get("FileSuffix").ToString();
    std::string examId = this->Get("Id").ToString();
    bool dicom = this->IsDICOM();

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

    // loop over the expected number of Acquisitions
    int acqGlobal = 1;
    std::string parentId;
    int downloaded = 1;
    for (int acq = 1; acq <= acqCount; ++acq)
    {
      vtkNew<Image> image;
      std::map<std::string, std::string> loader;
      loader["ExamId"] = examId;
      loader["Acquisition"] = vtkVariant(acqGlobal++).ToString();
      if (!image->Load(loader))
      {
        image->Set(loader);
        image->Save();
        if (!image->Load(loader))
        {
          if (!sustain) opal->SustainConnectionOff();
          std::string err = "ERROR: failed loadng image, exam Id ";
          err += examId;
          app->Log(err);
          break;
        }

        std::string opalVar;
        if (1 < acqCount || std::string::npos != acqNameFormat.find("%d"))
        {
          char buffer[256];
          snprintf(buffer, sizeof(buffer), acqNameFormat.c_str(), acq);
          opalVar = buffer;
        }
        else
          opalVar = acqNameFormat;

        std::string fileName = image->CreateFile(suffix);
        opal->SaveFile(fileName, source, type, identifier, opalVar, sideIndex);
      }

      if (image->ValidateFile())
      {
        if (0 < childCount)
          parentId = image->Get("Id").ToString();
        if (dicom)
          image->SetDimensionalityFromDICOM();
      }
      else
      {
        try
        {
          image->Remove();
        }
        catch (std::runtime_error& e)
        {
          app->Log(e.what());
        }
        downloaded = 0;
        break;
      }
    }

    // loop over the expected number of child images
    std::string childId;
    if (downloaded && 0 < childCount && !parentId.empty())
    {
      std::string childNameFormat = scanType->Get("ChildNameFormat").ToString();
      for (int acq = 1; acq <= childCount; ++acq)
      {
        vtkNew<Image> image;
        std::map<std::string, std::string> loader;
        loader["ExamId"] = examId;
        loader["Acquisition"] = vtkVariant(acqGlobal++).ToString();
        loader["ParentImageId"] = parentId;
        if (!image->Load(loader))
        {
          image->Set(loader);
          image->Save();
          if (!image->Load(loader))
          {
            if (!sustain) opal->SustainConnectionOff();
            std::string err = "ERROR: failed loadng child image, exam Id ";
            err += examId;
            app->Log(err);
            break;
          }

          std::string opalVar;
          if (1 < childCount || std::string::npos != childNameFormat.find("%d"))
          {
            char buffer[256];
            snprintf(buffer, sizeof(buffer), childNameFormat.c_str(), acq);
            opalVar = buffer;
          }
          else
            opalVar = childNameFormat;

          std::string fileName = image->CreateFile(suffix);
          opal->SaveFile(
            fileName, source, type, identifier, opalVar, sideIndex);
        }

        if (image->ValidateFile())
        {
          childId = image->Get("Id").ToString();
          if (dicom)
            image->SetDimensionalityFromDICOM();
        }
        else
        {
          try
          {
            image->Remove();
          }
          catch (std::runtime_error& e)
          {
            app->Log(e.what());
          }
          downloaded = 0;
          break;
        }
      }
    }

    this->Set("Downloaded", vtkVariant(downloaded).ToString());
    this->Save();

    if (!sustain) opal->SustainConnectionOff();

    // parent an only child based on DatetimeAcquired
    if (downloaded && 1 == childCount && 1 < acqCount
        && !childId.empty() && dicom)
    {
      std::vector<vtkSmartPointer<Image>> vecImage;
      this->GetList(&vecImage);

      // map AcquisitionDateTime from dicom file headers to Image Ids
      std::map<std::string, std::string> mapTime;
      for (auto it = vecImage.cbegin(); it != vecImage.cend(); ++it)
      {
        Image* image = it->GetPointer();
        mapTime[image->Get("Id").ToString()] =
          image->GetDICOMTag("AcquisitionDateTime");
      }

      if (mapTime.find(childId) == mapTime.end()) return;

      std::string acqDateTime = mapTime[childId];
      parentId.clear();
      for (auto it = mapTime.cbegin(); it != mapTime.cend(); ++it)
      {
        if (it->first == childId) continue;

        if (it->second == acqDateTime)
        {
          parentId = it->first;
          break;
        }
      }
      if (!parentId.empty())
      {
        vtkNew<Image> image;
        image->Load("Id", childId);
        image->Set("ParentImageId", parentId);
        image->Save(true);
      }
    }

    if (downloaded && dicom)
    {
      this->CleanImages();
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::CleanImages()
  {
    if (!this->IsDICOM()) return;

    bool isHologic = "Dexa" == this->GetModalityName();
    std::vector<vtkSmartPointer<Image>> vecImage;
    this->GetList(&vecImage);

    for (auto it = vecImage.begin(); it != vecImage.end(); ++it)
    {
      Image* image = *it;
      if (isHologic)
      {
        image->CleanHologicDICOM();
      }
      image->AnonymizeDICOM();
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsDICOM()
  {
    vtkSmartPointer<ScanType> scanType;
    if (this->GetRecord(scanType))
    {
      std::string suffix = scanType->Get("FileSuffix").ToString();
      return std::string::npos != suffix.find(".dcm");
    }
    return false;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsRatedBy(User* user)
  {
    if (!user)
      throw std::runtime_error("Tried to get rating for null user");

    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM Rating "
           << "JOIN Image ON Image.Id=Rating.ImageId "
           << "JOIN User ON User.Id=Rating.UserId "
           << "WHERE Image.ExamId="
           << this->Get("Id").ToString() << " "
           << "AND User.Id="
           << user->Get("Id").ToString();

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
    query->NextRow();
    return 0 <query->DataValue(0).ToInt();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<int, std::string> Exam::GetCodeTypeData()
  {
    std::map<int, std::string> data;
    vtkSmartPointer<ScanType> scanType;
    if (!this->GetRecord(scanType))
      throw std::runtime_error("Exam missing parent ScanType");

    std::vector<vtkSmartPointer<CodeType>> vecCodeType;
    scanType->GetList(&vecCodeType);
    for (auto it = vecCodeType.cbegin(); it != vecCodeType.cend(); ++it)
    {
      vtkVariant code = (*it)->Get("Code");
      vtkVariant id = (*it)->Get("Id");
      if (code.IsValid() && id.IsValid())
        data[id.ToInt()] = code.ToString();
    }
    return data;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Exam::SideStatus Exam::GetSideStatus()
  {
    Exam::SideStatus status = Exam::SideStatus::Pending;
    vtkSmartPointer<ScanType> type;
    if (this->GetRecord(type))
    {
      int downloaded = this->Get("Downloaded").ToInt();
      if (0 == downloaded)
        status = Exam::SideStatus::Pending;
      else
      {
        std::string side = this->Get("Side").ToString();
        if ("none" == side)
        {
          status = Exam::SideStatus::Fixed;
        }
        else
        {
          int sideCount = type->Get("SideCount").ToInt();
          if (1 == sideCount)
            status = Exam::SideStatus::Changeable;
          else if (2 == sideCount)
            status = Exam::SideStatus::Swappable;
        }
      }
    }

    return status;
  }
}  // namespace Alder
