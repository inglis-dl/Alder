/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ParticipantData.cxx
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <ParticipantData.h>

// Alder includes
#include <Common.h>
#include <Exam.h>
#include <Image.h>
#include <Interview.h>
#include <QueryModifier.h>
#include <Wave.h>

// VTK includes
#include <vtkObjectFactory.h>

// C++ includes
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(ParticipantData);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ParticipantData::ParticipantData()
  {
    this->ActiveInterview = NULL;
    this->ActiveImage = NULL;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ParticipantData::~ParticipantData()
  {
    this->SetActiveImage(NULL);
    this->SetActiveInterview(NULL);
    this->Clear();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::Clear()
  {
    this->examImageMap.clear();
    this->childImageMap.clear();
    this->interviewExamMap.clear();
    this->waveInterviewMap.clear();
    this->rankWaveMap.clear();
    this->uid = "";
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::SetActiveInterview(Interview* interview)
  {
    if (interview != this->ActiveInterview)
    {
      if (interview && this->ActiveInterview)
      {
        if (*interview == *this->ActiveInterview)
          return;
      }

      std::string lastId;
      std::string similarId;
      if (this->ActiveImage)
      {
        lastId = this->ActiveImage->Get("Id").ToString();
      }

      if (interview)
        this->LoadData(interview->Get("UId").ToString());
      else
        this->Clear();

      if (this->ActiveInterview)
      {
        this->ActiveInterview->UnRegister(this);
      }

      this->ActiveInterview = interview;

      if (this->ActiveInterview)
      {
        this->ActiveInterview->Register(this);
      }

      this->Modified();
      this->InvokeEvent(Common::InterviewChangedEvent);

      if (this->ActiveInterview && !lastId.empty())
      {
        similarId = this->ActiveInterview->GetSimilarImageId(lastId);
      }
      if (!similarId.empty())
      {
        vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();
        image->Load("Id", similarId);
        this->SetActiveImage(image);
      }
      else
      {
        this->SetActiveImage(NULL);
      }
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::SetActiveImage(Image* image)
  {
    if (image && this->ActiveImage)
    {
      if (*image == *this->ActiveImage)
        return;
    }
    vtkSetObjectBodyMacro(ActiveImage, Image, image);
    this->InvokeEvent(Common::ImageChangedEvent);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ParticipantData::LoadInterview(const int& id)
  {
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    bool loaded = interview->Load("Id", id);
    if (loaded)
    {
      std::string _uid = interview->Get("UId").ToString();
      if (_uid != this->uid)
        loaded = this->LoadData(_uid);
      else
        this->ReloadData();

      if (loaded)
        this->SetActiveInterview(interview);
    }

    return loaded;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ParticipantData::LoadData(const std::string& _uid)
  {
    if (_uid.empty())
      return false;

    if (_uid == this->uid)
      return true;

    this->Clear();

    this->uid = _uid;

    this->BuildData();
    this->InvokeEvent(Common::DataChangedEvent);
    return true;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::ReloadData()
  {
    std::string last = this->uid;
    this->Clear();
    this->uid = last;
    this->BuildData();
    this->InvokeEvent(Common::DataChangedEvent);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::BuildData()
  {
    vtkVariant vuid = vtkVariant(this->uid.c_str());

    vtkSmartPointer<QueryModifier> modifier =
      vtkSmartPointer<QueryModifier>::New();

    std::vector<vtkSmartPointer<Wave>> wlist;
    Wave::GetAll(&wlist);
    for (auto w = wlist.begin(); w != wlist.end(); ++w)
    {
      vtkSmartPointer<Wave> wave = *w;
      vtkVariant v = wave->Get("Id");
      if (!v.IsValid())
        continue;
      this->rankWaveMap[v.ToInt()] = wave;

      // get the interviews
      std::vector<vtkSmartPointer<Interview>> ilist;
      modifier->Reset();
      modifier->Where("UId", "=", vuid);
      modifier->Where("WaveId", "=", v);

      Interview::GetAll(&ilist, modifier);
      this->waveInterviewMap[*w] = ilist;

      for (auto i = ilist.begin(); i != ilist.end(); ++i)
      {
        vtkSmartPointer<Interview> interview = *i;

        // get the exams
        std::vector<vtkSmartPointer<Exam>> elist;
        modifier->Reset();
        modifier->Order("Side");
        interview->GetList(&elist, modifier);
        this->interviewExamMap[*i] = elist;

        for (auto e = elist.begin(); e != elist.end(); ++e)
        {
          vtkSmartPointer<Exam> exam = *e;
          // get the parent images
          std::vector<vtkSmartPointer<Image>> plist;
          modifier->Reset();
          modifier->Where("ParentImageId", "=", vtkVariant(), false);
          exam->GetList(&plist, modifier);
          this->examImageMap[*e] = plist;

          for (auto  p = plist.begin(); p != plist.end(); ++p)
          {
            // get the child images
            vtkSmartPointer<Image> image = *p;
            std::vector<vtkSmartPointer<Image>> clist;
            image->GetList(&clist, "ParentImageId");
            this->childImageMap[*p] = clist;
          }
        }
      }
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetInterviewList(Wave* wave,
    std::vector<vtkSmartPointer<Interview>>* container)
  {
    container->clear();
    if (this->uid.empty()) return;
    auto it = this->waveInterviewMap.begin();
    while (it != this->waveInterviewMap.end())
    {
      if (*wave == *(it->first))
      {
        *container = it->second;
        break;
      }
      it++;
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetWaveList(
    std::vector<vtkSmartPointer<Wave>>* container)
  {
    container->clear();
    if (this->uid.empty()) return;
    for (auto it = rankWaveMap.begin(); it != rankWaveMap.end(); ++it)
    {
      vtkSmartPointer<Wave> record = it->second;
      container->push_back(record);
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetExamList(Interview* interview,
    std::vector<vtkSmartPointer<Exam>>* container)
  {
    container->clear();
    if (this->uid.empty()) return;
    auto it = this->interviewExamMap.begin();
    while (it != this->interviewExamMap.end())
    {
      if (*interview == *(it->first))
      {
        *container = it->second;
        break;
      }
      it++;
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetParentImageList(Exam* exam,
    std::vector<vtkSmartPointer<Image>>* container)
  {
    container->clear();
    if (this->uid.empty()) return;
    auto it = this->examImageMap.begin();
    while (it != this->examImageMap.end())
    {
      if (*exam == *(it->first))
      {
        *container = it->second;
        break;
      }
      it++;
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetChildImageList(Image* image,
    std::vector<vtkSmartPointer<Image>>* container)
  {
    container->clear();
    if (this->uid.empty()) return;
    auto it = this->childImageMap.begin();
    while (it != this->childImageMap.end())
    {
      if (*image == *(it->first))
      {
        *container = it->second;
        break;
      }
      it++;
    }
  }
}  // namespace Alder
