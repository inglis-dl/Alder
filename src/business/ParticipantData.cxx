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

namespace Alder
{
  vtkStandardNewMacro( ParticipantData );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ParticipantData::ParticipantData()
  {
    this->ActiveInterview = NULL;
    this->ActiveImage = NULL;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ParticipantData::~ParticipantData()
  {
    this->SetActiveImage( NULL );
    this->SetActiveInterview( NULL );
    this->Clear();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::Clear()
  {
    this->examImageMap.clear();
    this->childImageMap.clear();
    this->interviewExamMap.clear();
    this->waveInterviewMap.clear();
    this->rankWaveMap.clear();
    this->uid = "";
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::SetActiveInterview( Interview *interview )
  {
    if( interview != this->ActiveInterview )
    {
      if( interview && this->ActiveInterview )
      {
        if( interview->Get("Id") == this->ActiveInterview->Get("Id") )
          return;
      }

      std::string lastId;
      std::string similarId;
      if( this->ActiveImage )
      {
        lastId = this->ActiveImage->Get( "Id" ).ToString();
      }

      if( interview )
        this->LoadData( interview->Get("UId").ToString() );
      else
        this->Clear();

      if( this->ActiveInterview )
      {
        this->ActiveInterview->UnRegister( this );
      }

      this->ActiveInterview = interview;

      if( this->ActiveInterview )
      {
        this->ActiveInterview->Register( this );
      }

      this->Modified();
      this->InvokeEvent( Common::InterviewChangedEvent );

      if( this->ActiveInterview && !lastId.empty() )
      {
        similarId = this->ActiveInterview->GetSimilarImageId( lastId );
      }
      if( !similarId.empty() )
      {
        vtkSmartPointer<Image> image = vtkSmartPointer<Image>::New();
        image->Load( "Id", similarId );
        this->SetActiveImage( image );
      }
      else
      {
        this->SetActiveImage( NULL );
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::SetActiveImage( Image* image )
  {
    if( image && this->ActiveImage )
    {
      if( image->Get("Id") == this->ActiveImage->Get("Id") )
        return;
    }
    vtkSetObjectBodyMacro( ActiveImage, Image, image);
    this->InvokeEvent( Common::ImageChangedEvent );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ParticipantData::LoadInterview( const int& id )
  {
    vtkSmartPointer<Interview> interview = vtkSmartPointer<Interview>::New();
    bool loaded = interview->Load( "Id", id );
    if( loaded )
    {
      std::string _uid = interview->Get("UId").ToString();
      if( _uid != this->uid )
        loaded = this->LoadData( _uid );

      if( loaded )
        this->SetActiveInterview( interview );
    }

    return loaded;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ParticipantData::LoadData( const std::string& _uid )
  {
    if( _uid.empty() )
      return false;

    if( this->uid == _uid )
      return true;

    this->Clear();
    vtkSmartPointer< Alder::QueryModifier > modifier =
      vtkSmartPointer< Alder::QueryModifier >::New();

    std::vector<vtkSmartPointer<Wave>> wlist;
    Alder::Wave::GetAll( &wlist );
    for( auto w = wlist.begin(); w != wlist.end(); ++w )
    {
      vtkSmartPointer<Wave> wave = *w;
      vtkVariant v = wave->Get( "Id" );
      if( !v.IsValid() )
        continue;
      this->rankWaveMap[ v.ToInt() ] = wave;

      // get the interviews
      std::vector<vtkSmartPointer<Interview>> ilist;
      modifier->Reset();
      modifier->Where( "UId", "=", vtkVariant(_uid.c_str()) );
      modifier->Where( "WaveId", "=", v );

      Interview::GetAll( &ilist, modifier );
      this->waveInterviewMap[ *w ] = ilist;

      for( auto i = ilist.begin(); i != ilist.end(); ++i )
      {
        vtkSmartPointer<Interview> interview = *i;

        // get the exams
        std::vector<vtkSmartPointer<Exam>> elist;
        modifier->Reset();
        modifier->Order( "Side" );
        interview->GetList( &elist, modifier );
        this->interviewExamMap[ *i ] = elist;

        for( auto e = elist.begin(); e != elist.end(); ++e )
        {
          vtkSmartPointer<Exam> exam = *e;
          // get the parent images
          std::vector<vtkSmartPointer<Image>> plist;
          modifier->Reset();
          modifier->Where( "ParentImageId", "=", vtkVariant(), false );
          exam->GetList( &plist, modifier );
          this->examImageMap[ *e ] = plist;

          for( auto  p = plist.begin(); p != plist.end(); ++p )
          {
            // get the child images
            vtkSmartPointer<Image> image = *p;
            std::vector<vtkSmartPointer<Image>> clist;
            image->GetList( &clist, "ParentImageId" );
            this->childImageMap[ *p ] = clist;
          }
        }
      }
    }
    this->uid = _uid;
    this->InvokeEvent( Common::DataChangedEvent );
    return true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetInterviewList( Wave* wave,
    std::vector<vtkSmartPointer<Interview>>* container )
  {
    container->clear();
    if( this->uid.empty() ) return;
    std::map< Wave*, std::vector<vtkSmartPointer<Interview>> >::iterator i
      = this->waveInterviewMap.find( wave );
    if( this->waveInterviewMap.end() != i )
      *container = i->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetWaveList( std::vector<vtkSmartPointer<Wave>>* container )
  {
    container->clear();
    if( this->uid.empty() ) return;
    for( auto i = rankWaveMap.begin(); i != rankWaveMap.end(); ++i )
    {
      vtkSmartPointer<Wave> record = i->second;
      container->push_back( record );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetExamList( Interview* interview,
    std::vector<vtkSmartPointer<Exam>>* container )
  {
    container->clear();
    if( this->uid.empty() ) return;
    std::map< Interview*, std::vector<vtkSmartPointer<Exam>> >::iterator i
      = this->interviewExamMap.find( interview );
    if( this->interviewExamMap.end() != i )
      *container = i->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ParticipantData::GetParentImageList( Exam* exam,
    std::vector<vtkSmartPointer<Image>>* container )
  {
    container->clear();
    if( this->uid.empty() ) return;
    std::map< Exam*, std::vector<vtkSmartPointer<Image>> >::iterator i
      = this->examImageMap.find( exam );
    if( this->examImageMap.end() != i )
      *container = i->second;
  }

  void ParticipantData::GetChildImageList( Image* image,
    std::vector<vtkSmartPointer<Image>>* container )
  {
    container->clear();
    if( this->uid.empty() ) return;
    std::map< Image* const, std::vector<vtkSmartPointer<Image>> >::iterator i
      = this->childImageMap.find( image );
    if( this->childImageMap.end() != i )
      *container = i->second;
  }
}
