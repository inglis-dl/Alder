/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ModelObject.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class ModelObject
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Base class for all model classes
 */

#ifndef __ModelObject_h
#define __ModelObject_h

#include <vtkObject.h>
#include <vtkSetGet.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class ModelObject : public vtkObject
  {
  public:
    vtkTypeMacro( ModelObject, vtkObject );

  protected:
    ModelObject();
    ~ModelObject() {}

  private:
    ModelObject( const ModelObject& ); // Not implemented
    void operator=( const ModelObject& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
