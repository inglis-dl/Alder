/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeGroup.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class CodeGroup
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the CodeGroup table
 */

#ifndef __CodeGroup_h
#define __CodeGroup_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class CodeGroup : public ActiveRecord
  {
  public:
    static CodeGroup *New();
    vtkTypeMacro( CodeGroup, ActiveRecord );
    std::string GetName() const { return "CodeGroup"; }

  protected:
    CodeGroup() {}
    ~CodeGroup() {}

  private:
    CodeGroup( const CodeGroup& ); // Not implemented
    void operator=( const CodeGroup& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
