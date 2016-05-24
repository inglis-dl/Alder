/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ImageNote.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class ImageNote
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the ImageNote table
 */

#ifndef __ImageNote_h
#define __ImageNote_h

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
  class ImageNote : public ActiveRecord
  {
    public:
      static ImageNote* New();
      vtkTypeMacro(ImageNote, ActiveRecord);
      std::string GetName() const { return "ImageNote"; }

    protected:
      ImageNote() {}
      ~ImageNote() {}

    private:
      ImageNote(const ImageNote&);  // Not implemented
      void operator=(const ImageNote&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
