/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkFrameAnimationPlayer.h
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

/**
 * @class vtkFrameAnimationPlayer

 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Concrete implementation of an animation player.
 *
 * This class was adapted from Paraview's vtkSequenceAnimationPlayer to work
 * with VTK's vtkAnimationCue and vtkAnimationScene since Paraview has
 * its own implementation of these two classes. It has been further adapted
 * to play back images frame by frame.
 */

#ifndef __vtkFrameAnimationPlayer_h
#define __vtkFrameAnimationPlayer_h

// Alder includes
#include <vtkAnimationPlayer.h>

class vtkFrameAnimationPlayer : public vtkAnimationPlayer
{
  public:
    static vtkFrameAnimationPlayer* New();
    vtkTypeMacro(vtkFrameAnimationPlayer, vtkAnimationPlayer);
    void PrintSelf(ostream& os, vtkIndent indent);

    /**
     * Set/Get the number of frames. Default 0 and must be set.
     */
    vtkSetClampMacro(NumberOfFrames, int, 2, VTK_INT_MAX);
    vtkGetMacro(NumberOfFrames, int);

    /** Get the frame number currently in play */
    vtkGetMacro(FrameNo, int);

  protected:
    vtkFrameAnimationPlayer();
    ~vtkFrameAnimationPlayer();

    virtual void StartLoop(const double&, const double&, double*);
    virtual void EndLoop() {}

    /** Return the next time given the current time. */
    virtual double GetNextTime(const double& currentime);

    virtual double GoToNextTime(
      const double& start, const double& end, const double& currenttime);
    virtual double GoToPreviousTime(
      const double& start, const double& end, const double& currenttime);

    int NumberOfFrames;
    double StartTime;
    double EndTime;
    int FrameNo;

  private:
    vtkFrameAnimationPlayer(const vtkFrameAnimationPlayer&);  /** Not implemented */
    void operator=(const vtkFrameAnimationPlayer&);  /** Not implemented */
};

#endif
