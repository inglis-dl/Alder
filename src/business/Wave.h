/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Wave.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Wave
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Wave table
 */

#ifndef __Wave_h
#define __Wave_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Wave : public ActiveRecord
  {
  public:
    static Wave *New();
    vtkTypeMacro( Wave, ActiveRecord );
    std::string GetName() const { return "Wave"; }

  protected:
    Wave() {}
    ~Wave() {}

  private:
    Wave( const Wave& ); // Not implemented
    void operator=( const Wave& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
