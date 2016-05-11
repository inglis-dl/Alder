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

// Alder includes
#include <ActiveRecord.h>

// C++ includes
#include <map>
#include <string>

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
      vtkTypeMacro(CodeType, ActiveRecord);
      std::string GetName() const { return "CodeType"; }

      /**
       * Get the number of times a CodeType has been used in the Rating table.
       * @return number of times used
       */
      int GetUsage();

      /**
       * Get the number of times each CodeType has been used in the Rating table.
       */
      static void GetUsageById(std::map<int, int>&);

      /**
       * Check if a CodeType with the code, value and group Id exists.
       */
      static bool IsUnique(const std::string &code, const int &value,
        const int &groupId = -1);

      /**
       * Update all Rating and DerivedRating values in the Rating table.
       * This method must be called whenever the Value of this record changes.
       */
      void UpdateRatings();

    protected:
      CodeType() {}
      ~CodeType() {}

    private:
      CodeType(const CodeType&);  // Not implemented
      void operator=(const CodeType&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
