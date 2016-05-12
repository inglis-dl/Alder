/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkCustomInteractorStyleImage.cxx
  Language:  C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkCustomInteractorStyleImage.h>

// VTK includes
#include <vtkAbstractPropPicker.h>
#include <vtkAssemblyPath.h>
#include <vtkCommand.h>
#include <vtkImageProperty.h>
#include <vtkObjectFactory.h>
#include <vtkPropCollection.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>

vtkStandardNewMacro(vtkCustomInteractorStyleImage);

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkCustomInteractorStyleImage::WindowLevel()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  this->WindowLevelCurrentPosition[0] = rwi->GetEventPosition()[0];
  this->WindowLevelCurrentPosition[1] = rwi->GetEventPosition()[1];

  if (this->HandleObservers &&
      this->HasObserver(vtkCommand::WindowLevelEvent))
  {
    this->InvokeEvent(vtkCommand::WindowLevelEvent, this);
  }
  else
  {
    if (this->CurrentImageProperty)
    {
      int *size = this->CurrentRenderer->GetSize();

      double window = this->WindowLevelInitial[0];
      double level = this->WindowLevelInitial[1];

      // Compute normalized delta

      double dx = (this->WindowLevelCurrentPosition[0] -
                   this->WindowLevelStartPosition[0]) * 4.0 / size[0];
      double dy = (this->WindowLevelStartPosition[1] -
                   this->WindowLevelCurrentPosition[1]) * 4.0 / size[1];

      // Scale by current values

      if (fabs(window) > 0.01)
      {
        dx = dx * window;
      }
      else
      {
        dx = dx * (0.0 > window ? -0.01 : 0.01);
      }
      if (fabs(level) > 0.01)
      {
        dy = dy * level;
      }
      else
      {
        dy = dy * (0.0 > level ? -0.01 : 0.01);
      }

      // Abs so that direction does not flip

      if (0.0 > window)
        {
        dx = -1 * dx;
        }
      if (0.0 > level)
      {
        dy = -1 * dy;
      }

      // Compute new window level

      double newWindow = dx + window;
      double newLevel = level - dy;

      if (0.01 > newWindow)
      {
        newWindow = 0.01;
      }

      this->CurrentImageProperty->SetColorWindow(newWindow);
      this->CurrentImageProperty->SetColorLevel(newLevel);

      this->Interactor->Render();
    }
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkCustomInteractorStyleImage::OnChar()
{
  vtkRenderWindowInteractor *rwi = this->Interactor;

  // trap x y z char
  switch (rwi->GetKeyCode())
  {
    case 'x' :
    case 'X' :
    case 'y' :
    case 'Y' :
    case 'z' :
    case 'Z' :
      break;

    default:
      this->Superclass::OnChar();
      break;
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkCustomInteractorStyleImage::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
