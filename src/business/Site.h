/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Site.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Site
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Site table
 */

#ifndef __Site_h
#define __Site_h

// VTK includes
#include <ActiveRecord.h>

// C++ includes
#include <string>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Site : public ActiveRecord
  {
    public:
      static Site* New();
      vtkTypeMacro(Site, ActiveRecord);
      std::string GetName() const { return "Site"; }

    protected:
      Site() {}
      ~Site() {}

    private:
      Site(const Site&);  // Not implemented
      void operator=(const Site&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
