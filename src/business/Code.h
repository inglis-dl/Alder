/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Code.h
  Language: C++

  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Code
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Code table
 */

#ifndef __Code_h
#define __Code_h

#include <ActiveRecord.h>

// C++ includes
#include <string>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Code : public ActiveRecord
  {
    public:
      static Code *New();
      vtkTypeMacro(Code, ActiveRecord);
      std::string GetName() const { return "Code"; }

    protected:
      Code() {}
      ~Code() {}

    private:
      Code(const Code&);  // Not implemented
      void operator=(const Code&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
