/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Rating.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Rating
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Rating table
 */

#ifndef __Rating_h
#define __Rating_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Rating : public ActiveRecord
  {
  public:
    static Rating *New();
    vtkTypeMacro( Rating, ActiveRecord );
    std::string GetName() const { return "Rating"; }

    const static int MaximumRating;
    const static int MinimumRating;

    /**
     * Compute and update the DerivedRating from any Codes that
     * are common to the user and image associated with this rating.
     * The Rating can be set from the computed DerivedRating.
     * bool setRatingFromDerived
     */
    void UpdateDerivedRating( bool setRatingFromDerived = false );

  protected:
    Rating() {}
    ~Rating() {}

  private:
    Rating( const Rating& ); // Not implemented
    void operator=( const Rating& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
