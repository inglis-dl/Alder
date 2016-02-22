/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ProgressProxy.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <ProgressProxy.h>

// Alder includes
#include <OpalService.h>

// VTK includes
#include <vtkCommand.h>

namespace Alder
{

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ProgressProxy::UpdateProgress( const double& percent )
  {
    if( !this->busyProgress )
    {
      this->progressData.second = percent;
      this->application->InvokeEvent( vtkCommand::ProgressEvent, static_cast<void *>( &this->progressData ) );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ProgressProxy::ConfigureProgress()
  {
    OpalService::SetCurlProgress( this->curlProgress );
    this->configurationData = std::pair<int, bool>( this->progressType, this->busyProgress );
    this->application->InvokeEvent( vtkCommand::ConfigureEvent, static_cast<void *>( &this->configurationData ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ProgressProxy::StartProgress()
  {
    this->progressData =
      std::pair<int, double>( this->progressType, 0.0 );
    this->application->InvokeEvent( vtkCommand::StartEvent, static_cast<void *>( &this->progressType ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ProgressProxy::EndProgress()
  {
    this->application->InvokeEvent( vtkCommand::EndEvent, static_cast<void *>( &this->progressType ) );
    if( this->curlProgress )
      OpalService::SetCurlProgress( false );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ProgressProxy::~ProgressProxy()
  {
    if( this->application->GetAbortFlag() )
      this->application->SetAbortFlag(false);
    this->application = NULL;
  }
}
