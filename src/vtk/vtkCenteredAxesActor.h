/*=========================================================================

  Module:    vtkCenteredAxesActor.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkCenteredAxesActor
 *
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @see vtkAxesActor
 */

#ifndef __vtkCenteredAxesActor_h
#define __vtkCenteredAxesActor_h

// VTK includes
#include <vtkAxesActor.h>

class vtkCenteredAxesActor : public vtkAxesActor
{
  public:
    static vtkCenteredAxesActor *New();
    vtkTypeMacro(vtkCenteredAxesActor, vtkAxesActor);
    void PrintSelf(ostream &os, vtkIndent indent);

    double* GetBounds();

  protected:
    vtkCenteredAxesActor() {}
    ~vtkCenteredAxesActor() {}

  private:
    vtkCenteredAxesActor(const vtkCenteredAxesActor&);  /** Not implemented. */
    void operator=(const vtkCenteredAxesActor&);  /** Not implemented. */
};

#endif
