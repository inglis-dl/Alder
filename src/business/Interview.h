/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Interview.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Interview
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Interview table
 */

#ifndef __Interview_h
#define __Interview_h

#include <ActiveRecord.h>
#include <ProgressProxy.h>
#include <Image.h>
#include <Wave.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Interview : public ActiveRecord
  {
  public:
    static Interview *New();
    vtkTypeMacro( Interview, ActiveRecord );
    std::string GetName() const { return "Interview"; }

    /**
     * Updates the Interview table with all existing interviews in Opal
     * by Wave Id and whether to restrict the update all Interviews, or those
     *
     * @param waveList std::pair vector of Wave Id, full update flag
     */
    static void UpdateInterviewData( const std::vector< std::pair< int, bool > > &waveList );

    /**
     * Returns whether this interview's exam data has been downloaded.
     */
    bool HasExamData();

    /**
     * Returns whether this interview's image data has been downloaded.
     * @param modifier QueryModifier to constrain to user allowed modalities
     */
    bool HasImageData( QueryModifier *modifier = NULL );

    /**
     * Updates and/or creates all Exam data associated with the Interview from Opal.
     * Note: Exam data must be downloaded before Image data.
     * @param aWave         Wave that this interview belongs to
     * @param aSource       Opal data source containing an Exam view
     */
    void UpdateExamData(
     Alder::Wave *aWave = NULL, const std::string &aSource = "" );

    /**
     * Updates all exam and image data associated with the interview from Opal.
     * Note: exam data must be downloaded before image data.
     */
    void UpdateImageData();

    /**
     * Loads exam and image data associated with interviews from Opal
     * from a list of identifiers (UId wave rank pairs).
     * Caveats:
     * 1) the administrator must update the interview data in the database first
     *    to ensure the requested interviews can be retrieved
     * 2) only the images the user is permitted to review are downloaded
     * @param list  vector of interview UId wave rank pairs
     * @return      the number of Interviews loaded
     */
    static int LoadFromList( const std::vector< std::pair < std::string, std::string > > &list );

    /**
     * Given an image Id, find an image in this interview having the same
     * characteristics and return its Id.
     * @param imageId the Id of an image record
     * @return        the Id of a similar image or empty string on fail
     */
    std::string GetSimilarImageId( const std::string &imageId );

    //@{
    /**
     * Methods to get the neighbouring interview in UId/VisitDate order.
     * @param forward the direction of search
     * @param loaded  whether the requested neighbour has to have images already downloaded
     * @param unRated whether the requested neighbour has to have unrated images only
     * @return        the neighbouring interview in UId/VisidDate order
     */
    vtkSmartPointer<Interview> GetNeighbour( const bool &forward, const bool &loaded, const bool &unRated );
    vtkSmartPointer<Interview> GetNext( const bool &loaded, const bool &unRated )
    { return this->GetNeighbour( true, loaded, unRated ); }
    vtkSmartPointer<Interview> GetNextLoaded( const bool &unRated )
    { return this->GetNeighbour( true, true, unRated ); }
    vtkSmartPointer<Interview> GetNextUnLoaded( const bool &unRated )
    { return this->GetNeighbour( true, false, unRated ); }
    vtkSmartPointer<Interview> GetPrevious( const bool &loaded, const bool &unRated )
    { return this->GetNeighbour( false, loaded, unRated ); }
    vtkSmartPointer<Interview> GetPreviousLoaded( const bool &unRated )
    { return this->GetNeighbour( false, true, unRated ); }
    vtkSmartPointer<Interview> GetPreviousUnLoaded( const bool &unRated )
    { return this->GetNeighbour( false, false, unRated ); }
    //@}

    /**
     * Convenience method to determine how many images this interview has.
     * @return number of images this interview has
     */
    int GetImageCount();

    /**
     * Returns whether a user has rated all images associated with the interview.
     * If the interview has no images this method returns true.
     * @param user a User object
     * @return     whether all images in this interview are rated by the User
     */
    bool IsRatedBy( User* user );

  protected:
    Interview() {}
    ~Interview() {}

    /**
     * Returns a vector of all UId/VisitDate pairs ordered by UId then VisitDate.
     * @return vector of interview UId, VisitDate data
     */
    // static std::vector<std::pair<std::string, std::string>> GetUIdVisitDateList();

  private:
    Interview( const Interview& ); // Not implemented
    void operator=( const Interview& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
