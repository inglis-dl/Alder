/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkFrameAnimationPlayer.cxx
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
#include <vtkFrameAnimationPlayer.h>

// Alder includes
#include <vtkAnimationScene.h>

// VTK includes
#include <vtkMath.h>
#include <vtkObjectFactory.h>
#include <vtkWeakPointer.h>

vtkStandardNewMacro(vtkFrameAnimationPlayer);

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkFrameAnimationPlayer::vtkFrameAnimationPlayer()
{
  this->NumberOfFrames = 0;
  this->FrameNo = 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkFrameAnimationPlayer::~vtkFrameAnimationPlayer()
{
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkFrameAnimationPlayer::StartLoop(
  const double &starttime, const double &endtime,
  double* vtkNotUsed(playbackWindow))
{
  // the frame index is inited to 0 ONLY when an animation is not resumed from
  // an intermediate frame
  this->FrameNo = 0;

  this->StartTime = starttime;
  this->EndTime = endtime;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double vtkFrameAnimationPlayer::GetNextTime(const double &curtime)
{
  this->FrameNo = static_cast<int>((curtime - this->StartTime) /
      (this->EndTime-this->StartTime) * this->NumberOfFrames + 0.5);

  if (this->StartTime >= this->EndTime)
  {
    return vtkMath::Nan();
  }

  double time = 1./ this->GetAnimationScene()->GetFrameRate() + curtime;
  return time;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double vtkFrameAnimationPlayer::GoToNextTime(
  const double &start, const double &end, const double &curtime)
{
  double delta = static_cast<double>(end-start)/(this->NumberOfFrames-1);
  return (curtime + delta);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double vtkFrameAnimationPlayer::GoToPreviousTime(
  const double &start, const double &end, const double &curtime)
{
  double delta = static_cast<double>(end-start)/(this->NumberOfFrames-1);
  return (curtime - delta);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkFrameAnimationPlayer::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
