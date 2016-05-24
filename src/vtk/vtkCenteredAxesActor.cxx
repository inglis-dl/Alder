/*=========================================================================

  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Module:    vtkCenteredAxesActor.cxx
  Language:  C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkCenteredAxesActor.h>

// VTK includes
#include <vtkActor.h>
#include <vtkBoundingBox.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkCenteredAxesActor);

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
double* vtkCenteredAxesActor::GetBounds()
{
  vtkBoundingBox totalBoundingBox(this->XAxisShaft->GetBounds());
  totalBoundingBox.AddBounds(this->YAxisShaft->GetBounds());
  totalBoundingBox.AddBounds(this->ZAxisShaft->GetBounds());
  totalBoundingBox.AddBounds(this->XAxisTip->GetBounds());
  totalBoundingBox.AddBounds(this->YAxisTip->GetBounds());
  totalBoundingBox.AddBounds(this->ZAxisTip->GetBounds());
  totalBoundingBox.GetBounds(this->Bounds);
  return this->Bounds;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkCenteredAxesActor::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}
