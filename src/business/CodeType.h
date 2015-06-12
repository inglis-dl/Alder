/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeType.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class CodeType
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the CodeType table
 */

#ifndef __CodeType_h
#define __CodeType_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class CodeType : public ActiveRecord
  {
  public:
    static CodeType *New();
    vtkTypeMacro( CodeType, ActiveRecord );
    std::string GetName() const { return "CodeType"; }

  protected:
    CodeType() {}
    ~CodeType() {}

  private:
    CodeType( const CodeType& ); // Not implemented
    void operator=( const CodeType& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif