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
     * @param derive set the rating from the derived rating
     */
    void UpdateDerivedRating( const bool derive = false );

    /**
     * Get the number of ratings by modality that the User has access to.
     * @param user a User object
     * @return     map of modality names to rating counts for the specified user
     * @throws     runtime_error
     */
    static std::map<std::string,int> GetNumberOfRatings( User* user );

    /**
     * Get rating data created by the User and other meta data to build a rating report.
     * @param user     a User object
     * @param modality optional modality restriction
     * @return         vector of maps containing key value pairs containing the data
     * @throws         runtime_error
     */
    static std::vector<std::map<std::string,std::string>> GetRatingReportData(
      User* user, const std::string modality = "" );

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
