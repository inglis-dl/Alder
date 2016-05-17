/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkAnimationPlayer.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
#include <vtkAnimationPlayer.h>

// VTK includes
#include <vtkAnimationScene.h>
#include <vtkCommand.h>
#include <vtkMath.h>
#include <vtkObjectFactory.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkAnimationPlayer::vtkAnimationPlayer()
{
  this->AnimationScene = 0;
  this->InPlay = false;
  this->CurrentTime = 0;
  this->StopPlay = false;
  this->Loop = false;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkAnimationPlayer::~vtkAnimationPlayer()
{
  this->SetAnimationScene(0);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::SetAnimationScene(vtkAnimationScene* scene)
{
  if (this->AnimationScene != scene)
  {
    this->AnimationScene = scene;
    this->Modified();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkAnimationScene* vtkAnimationPlayer::GetAnimationScene()
{
  return this->AnimationScene;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::Play()
{
  if (!this->AnimationScene)
  {
    vtkErrorMacro("No animation scene to play.");
    return;
  }

  if (this->InPlay)
  {
    vtkErrorMacro("Cannot play while playing.");
    return;
  }

  this->InvokeEvent(vtkCommand::StartEvent);

  double starttime = this->AnimationScene->GetStartTime();
  double endtime = this->AnimationScene->GetEndTime();

  double playbackWindow[2];
  playbackWindow[0] = starttime;
  playbackWindow[1] = endtime;
  double period = endtime - starttime;
  this->CurrentTime = starttime;

  this->InPlay = true;
  this->StopPlay = false;

  do
  {
    this->StartLoop(starttime, endtime, playbackWindow);
    this->AnimationScene->Initialize();
    double deltatime = 0.0;
    while (!this->StopPlay && this->CurrentTime <= endtime)
    {
      this->AnimationScene->Tick(
        this->CurrentTime, deltatime, this->CurrentTime);
      double progress = (this->CurrentTime - starttime) / period;
      this->InvokeEvent(vtkCommand::ProgressEvent, &progress);
      double nexttime = this->GetNextTime(this->CurrentTime);
      if (vtkMath::IsNan(nexttime)) break;
      deltatime = nexttime - this->CurrentTime;
      this->CurrentTime = nexttime;
    }

    // Finalize will get called when Tick() is called with time>=endtime on the
    // cue. However, no harm in calling this method again since it has any
    // effect only the first time it gets called.
    this->AnimationScene->Finalize();

    this->CurrentTime = starttime;
    this->EndLoop();
    // loop when this->Loop is true.
  } while (this->Loop && !this->StopPlay);

  this->InPlay = false;
  this->StopPlay = false;

  this->InvokeEvent(vtkCommand::EndEvent);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::Stop()
{
  if (this->InPlay)
  {
    this->StopPlay = true;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::GoToFirst()
{
  this->Stop();
  if (this->AnimationScene)
  {
    this->AnimationScene->SetAnimationTime(
      this->AnimationScene->GetStartTime());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::GoToLast()
{
  this->Stop();
  if (this->AnimationScene)
  {
    this->AnimationScene->SetAnimationTime(
      this->AnimationScene->GetEndTime());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::GoToNext()
{
  this->Stop();
  double starttime = this->AnimationScene->GetStartTime();
  double endtime = this->AnimationScene->GetEndTime();
  double time = this->GoToNextTime(starttime, endtime,
                this->AnimationScene->GetAnimationTime());

  if (time >= starttime && time < endtime)
  {
    this->AnimationScene->SetAnimationTime(time);
  }
  else
  {
    this->AnimationScene->SetAnimationTime(endtime);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::GoToPrevious()
{
  this->Stop();
  double starttime = this->AnimationScene->GetStartTime();
  double endtime = this->AnimationScene->GetEndTime();
  double time = this->GoToPreviousTime(starttime, endtime,
    this->AnimationScene->GetAnimationTime());

  if (time >= starttime && time < endtime)
  {
    this->AnimationScene->SetAnimationTime(time);
  }
  else
  {
    this->AnimationScene->SetAnimationTime(starttime);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkAnimationPlayer::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
