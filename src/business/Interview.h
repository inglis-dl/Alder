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
     * Updates the Interview table with all existing interviews in Opal.
     * @param proxy a progress proxy object
     */
    static void UpdateInterviewData( ProgressProxy& proxy );

    /**
     * Returns whether this interview's exam data has been downloaded.
     */
    bool HasExamData();

    /**
     * Returns whether this interview's image data has been downloaded.
     */
    bool HasImageData();

    /**
     * Updates all exam data associated with the interview from Opal.
     * Note: exam data must be downloaded before image data.
     * @param updateMetaData whether to update exam data
     */
    void UpdateExamData( const bool& updateMetaData = false );

    /**
     * Updates all exam and image data associated with the interview from Opal.
     * Note: exam data must be downloaded before image data.
     * @param proxy a progress proxy object
     */
    void UpdateImageData( ProgressProxy& proxy );

    /**
     * Loads exam and image data associated with interviews from Opal
     * from a list of participant identifiers (UIDs).
     * Caveats:
     * 1) the administrator must update the interview data in the database first
     *    to ensure the requested UIDs can be retrieved
     * 2) only the images the user is permitted to review are downloaded
     * @param uidList a list of interview UIds
     * @param proxy   a progress proxy object
     * @return        the number of UIDs loaded
     */
    static int LoadFromUIDList( std::vector< std::string > const &uidList, ProgressProxy& proxy );

    /**
     * Given an image Id, find an image in this interview having the same
     * characteristics and return its Id.
     * @param imageId the Id of an image record
     * @return        the Id of a similar image or empty string on fail
     */
    std::string GetSimilarImage( std::string const &imageId );

    //@{
    /**
     * Methods to get the neighbouring interview in UId/VisitDate order.
     * @param forward the direction of search
     * @param loaded  whether the requested neighbour has to have images already downloaded
     * @param unRated whether the requested neighbour has to have unrated images only
     * @return        the neighbouring interview in UId/VisidDate order
     */
    vtkSmartPointer<Interview> GetNeighbour( const bool forward, const bool loaded, const bool unRated );
    vtkSmartPointer<Interview> GetNext( const bool loaded, const bool unRated )
    { return this->GetNeighbour( true, loaded, unRated ); }
    vtkSmartPointer<Interview> GetNextLoaded( const bool unRated )
    { return this->GetNeighbour( true, true, unRated ); }
    vtkSmartPointer<Interview> GetNextUnLoaded( const bool unRated )
    { return this->GetNeighbour( true, false, unRated ); }
    vtkSmartPointer<Interview> GetPrevious( const bool loaded, const bool unRated )
    { return this->GetNeighbour( false, loaded, unRated ); }
    vtkSmartPointer<Interview> GetPreviousLoaded( const bool unRated )
    { return this->GetNeighbour( false, true, unRated ); }
    vtkSmartPointer<Interview> GetPreviousUnLoaded( const bool unRated )
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
    static std::vector<std::pair<std::string, std::string>> GetUIdVisitDateList();

  private:
    Interview( const Interview& ); // Not implemented
    void operator=( const Interview& ); // Not implemented
  };

  class BaseInterviewProgressFunc {
    public:
      virtual void progressFunc() = 0;
      virtual ~BaseInterviewProgressFunc() = 0;
  };

  inline BaseInterviewProgressFunc::~BaseInterviewProgressFunc(){}

  class SingleInterviewProgressFunc : public BaseInterviewProgressFunc {
    public:
      SingleInterviewProgressFunc( Interview* v, const bool p = true ) : interview(v), curlProgress(p) {}

      virtual void progressFunc()
      {
        if( NULL == this->interview ) return;
        ProgressProxy proxy;
        proxy.SetCurlProgress( this->curlProgress );
        if( !this->curlProgress )
        {
          proxy.SetProgressTypeLocal();
        }
        proxy.ConfigureProgress();
        this->interview->UpdateImageData( proxy );
      }

      ~SingleInterviewProgressFunc() { this->interview = NULL; };

    private:
      Interview* interview;
      bool curlProgress;
  };

  class MultiInterviewProgressFunc : public BaseInterviewProgressFunc {
    public:
      virtual void progressFunc()
      {
        ProgressProxy proxy;
        proxy.SetCurlProgressOn();
        proxy.ConfigureProgress();
        Interview::UpdateInterviewData( proxy );
      }
  };

  class ListInterviewProgressFunc : public BaseInterviewProgressFunc {
    public:
      ListInterviewProgressFunc( const std::vector<std::string>& v ) : uidVector(v), numLoaded(0) {}
      virtual void progressFunc()
      {
        ProgressProxy proxy;
        proxy.SetCurlProgressOff();
        proxy.ConfigureProgress();
        this->numLoaded = Interview::LoadFromUIDList( this->uidVector, proxy );
      }

      int GetNumLoaded() const { return this->numLoaded; }

    private:
      std::vector<std::string> uidVector;
      int numLoaded;
  };
}

/** @} end of doxygen group */

#endif
