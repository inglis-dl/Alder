/*=========================================================================

  Module:    vtkCustomInteractorStyleImage.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkCustomInteractorStyleImage
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief  Interactive manipulation of the camera specialized for images.
 *
 * @see vtkInteractorStyleImage
 */

#ifndef __vtkCustomInteractorStyleImage_h
#define __vtkCustomInteractorStyleImage_h

// VTK includes
#include <vtkInteractorStyleImage.h>

class vtkCustomInteractorStyleImage : public vtkInteractorStyleImage
{
  public:
    static vtkCustomInteractorStyleImage* New();
    vtkTypeMacro(vtkCustomInteractorStyleImage, vtkInteractorStyleImage);
    void PrintSelf(ostream& os, vtkIndent indent);

    /**
     * Method overridden from parent class to fix a bug with passing
     * window level events.
     */
    virtual void WindowLevel();

    virtual void OnChar();

  protected:
    vtkCustomInteractorStyleImage() {}
    ~vtkCustomInteractorStyleImage() {}

  private:
    vtkCustomInteractorStyleImage(const vtkCustomInteractorStyleImage&);  /** Not implemented. */
    void operator=(const vtkCustomInteractorStyleImage&);  /** Not implemented. */
};

#endif
