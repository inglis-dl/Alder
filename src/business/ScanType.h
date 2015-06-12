/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ScanType.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class ScanType
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the ScanType table
 */

#ifndef __ScanType_h
#define __ScanType_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class ScanType : public ActiveRecord
  {
  public:
    static ScanType *New();
    vtkTypeMacro( ScanType, ActiveRecord );
    std::string GetName() const { return "ScanType"; }

  protected:
    ScanType() {}
    ~ScanType() {}

  private:
    ScanType( const ScanType& ); // Not implemented
    void operator=( const ScanType& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
