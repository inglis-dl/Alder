/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Common.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Common
 *
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Common class includes typedefs, macros, global events, etc.
 *
 */
#ifndef __Common_h
#define __Common_h

#include <vtkCommand.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Common
  {
  public:

    enum ImageStatus
    {
      None = 0,
      Pending,
      Complete
    };

    enum CustomEvents
    {
      ActiveUserEvent = vtkCommand::UserEvent + 100,
      ActiveInterviewEvent,
      ActiveInterviewUpdateImageDataEvent,
      ActiveImageEvent,
      ActiveAtlasImageEvent,
      SliceChangedEvent,
      OrientationChangedEvent
    };

  protected:
    Common() {}
    ~Common() {}
  };
}

#endif // __Common_h
