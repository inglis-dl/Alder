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

// Alder includes
#include <ActiveRecord.h>

// C++ includes
#include <string>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class CodeGroup : public ActiveRecord
  {
    public:
      static CodeGroup* New();
      vtkTypeMacro(CodeGroup, ActiveRecord);
      std::string GetName() const { return "CodeGroup"; }

      /**
       * Get the number of times a CodeGroup has been used in the CodeType table.
       * @return number of times used
       */
      int GetUsage();


      /**
       * Check if a CodeGroup with the code and value exists.
       */
      static bool IsUnique(const std::string& code, const int& value);

      /**
       * Update all Rating and DerivedRating values in the Rating table.
       * This method must be called whenever the Value of this record changes.
       */
      void UpdateRatings();

    protected:
      CodeGroup() {}
      ~CodeGroup() {}

    private:
      CodeGroup(const CodeGroup&);  // Not implemented
      void operator=(const CodeGroup&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
