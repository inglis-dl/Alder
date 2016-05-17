/*=========================================================================

  Module:    vtkAnimationPlayer.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http: *www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/

/**
 * @class vtkAnimationPlayer

 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Abstract superclass for an animation player.
 *
 * This class was adapted from Paraview's vtkAnimationPlayer to work
 * with VTK's vtkAnimationCue and vtkAnimationScene since Paraview has
 * its own implementation of these two classes.
 */

#ifndef __vtkAnimationPlayer_h
#define __vtkAnimationPlayer_h

// VTK includes
#include <vtkObject.h>
#include <vtkWeakPointer.h>

class vtkAnimationScene;

class vtkAnimationPlayer : public vtkObject
{
  public:
    vtkTypeMacro(vtkAnimationPlayer, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    /**
     * Set the animation scene that is to be played by this player.
     * Note that the animation scene is not reference counted to avoid loops.
     */
    virtual void SetAnimationScene(vtkAnimationScene* scene);
    vtkAnimationScene* GetAnimationScene();

    /**
     * Start playing the animation.
     * Fires StartEvent when play begins and EndEvent when play stops.
     */
    void Play();

    /**
     * Stop playing the animation.
     */
    void Stop();

    /**
     * Returns if the animation is currently playing.
     */
    int IsInPlay() { return this->InPlay? 1 : 0; }
    vtkGetMacro(InPlay, bool);

    /**
     * Set to true to play the animation in a loop.
     */
    vtkSetMacro(Loop, bool);
    vtkGetMacro(Loop, bool);

    /**
     * Take the animation scene to next frame.
     */
    void GoToNext();

    /**
     * Take animation scene to previous frame.
     */
    void GoToPrevious();

    /**
     * Take animation scene to first frame.
     */
    void GoToFirst();

    /**
     * Take animation scene to last frame.
     */
    void GoToLast();

  protected:
    vtkAnimationPlayer();
    ~vtkAnimationPlayer();

    virtual void StartLoop(
      const double& starttime, const double& endtime,
      double* playbackWindow) = 0;
    virtual void EndLoop() = 0;

    /**
     * Return the next time given the current time.
     */
    virtual double GetNextTime(const double& currentime) = 0;

    virtual double GoToNextTime(
      const double& start, const double& end, const double& currenttime) = 0;
    virtual double GoToPreviousTime(
      const double& start, const double& end, const double& currenttime) = 0;

  private:
    vtkAnimationPlayer(const vtkAnimationPlayer&);  /** Not implemented */
    void operator=(const vtkAnimationPlayer&);  /** Not implemented */

    vtkWeakPointer<vtkAnimationScene> AnimationScene;
    bool InPlay;
    bool StopPlay;
    bool Loop;
    double CurrentTime;
};

#endif
