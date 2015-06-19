/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Exam
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Exam table
 */

#ifndef __Exam_h
#define __Exam_h

#include <ActiveRecord.h>

#include <iostream>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Exam : public ActiveRecord
  {
  public:
    static Exam *New();
    vtkTypeMacro( Exam, ActiveRecord );
    std::string GetName() const { return "Exam"; }

    /**
     * Returns the exam's code (used to determine the exam's path in the image data directory)
     */
    virtual std::string GetCode();

    /**
     * Returns whether this exam's image data has been downloaded
     */
    bool HasImageData();

    /**
     * Updates all image data associated with the exam from Opal
     */
    void UpdateImageData();

    /**
     * Returns whether a user has rated all images associated with the exam.
     * If the exam has no images this method returns true.
     * @param user a User object to check against
     */
    bool IsRatedBy( User* user );

    /**
     * Is this a dicom image?
     */
    bool IsDICOM();

    /**
     * Convenience method to get the Exam type from the ScanType table.
     * @return the exam's scan type
     */
    std::string GetScanType();

    /**
     * Convenience method to get the Exam modality from the ScanType table.
     * @return the exam's modality
     */
    std::string GetModalityName();

    /**
     * Convenience method to get the CodeType Id's and Codes.
     * @return map of Codes with their table Ids
     */
    std::map<int, std::string> GetCodeTypeData();

  protected:
    Exam() {}
    ~Exam() {}

    /**
     * Retrieves an image from Opal.
     * @param type          the type of exam
     * @param variable      the Opal name of the image data variable
     * @param UId           the participant UId associated with the Interview
     * @param settings      map of settings
     * @param suffix        the file suffix to attach to the requested binary data
     * @param repeatable    whether the image is a repeatable Opal data entity
     * @param sideVariable  the Opal name of the side variable
     * @return              success or fail to retrieve an image
     * @throws              exception
     */
    bool RetrieveImage(
      const std::string type,
      const std::string variable,
      const std::string UId,
      const std::map<std::string, vtkVariant> settings,
      const std::string suffix,
      const bool repeatable = false,
      const std::string sideVariable = "" );

    /**
     * Fixes laterality and anonymization issues with dicom images.
     * @param type the type of exam
     */
    void CleanImages( std::string const &type );

  private:
    Exam( const Exam& ); // Not implemented
    void operator=( const Exam& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
