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

#include <iostream>

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
     */
    static void UpdateInterviewData( ProgressProxy& proxy );

    /**
     * Returns whether this interview's exam and image data has been downloaded
     * @return bool
     */
    bool HasExamData();
    bool HasImageData();

    /**
     * Updates all exam and image data associated with the interview from Opal
     * Note: exam data must be downloaded before image data
     */
    void UpdateExamData( const bool& updateMetaData = false );
    void UpdateImageData( ProgressProxy& proxy );

    /**
     * Loads exam and image data associated with interviews from Opal
     * from a list of participant identifiers (UIDs).
     * Caveats:
     * 1) the administrator must update the interview data in the database first
     *    to ensure the requested UIDs can be retrieved
     * 2) only the images the user is permitted to review are downloaded
     * @return int The number of UIDs loaded
     */
    static int LoadFromUIDList( std::vector< std::string > const &uidList, ProgressProxy& proxy );

    /**
     * Given an image Id, find an image in this record having the same
     * characteristics and return its Id
     * @return std::string Id of a similar image or empty string on fail
     */
    std::string GetSimilarImage( std::string const &imageId );

    /**
     * Get the neighbouring interview in UId/VisitDate order
     * @return vtkSmartPointer<Alder::Interview> The neighbouring interview in UId/VisidDate order
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

    /**
     * Convenience method to determine how many images this interview has
     * @return int Number of images this interview has
     */
    int GetImageCount();

    /**
     * Returns whether a user has rated all images associated with the interview.
     * If the interview has no images this method returns true
     * @return bool Whether all images in this interview are rated by the User
     */
    bool IsRatedBy( User* user );

  protected:
    Interview() {}
    ~Interview() {}

    /**
     * Returns a vector of all UId/VisitDate pairs ordered by UId then VisitDate
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
