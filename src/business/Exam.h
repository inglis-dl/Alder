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

    enum SideStatus
    {
      Pending = 0,
      Fixed,
      Changeable,
      Swappable
    };

    /**
     * Returns the exam's code (used to determine the exam's path in the image data directory)
     */
    virtual std::string GetCode();

    /**
     * Returns whether this Exam has image data that can be downloaded.
     * Use the Exam Downloaded column to determine if the image has been downloaded.
     */
    bool HasImageData();

    /**
     * Updates all image data associated with the exam from Opal
     * @param aIdentifier The UID of a participant (Opal identifier)
     * @parem aSource     The Opal project name containing image data
     */
    void UpdateImageData( const std::string &aIdentifier="", const std::string &aSource="");

    /**
     * Returns whether a user has rated all images associated with the exam.
     * If the exam has no images this method returns true.
     * @param user a User object to check against
     */
    bool IsRatedBy( Alder::User* user );

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
    std::map< int, std::string > GetCodeTypeData();

    /**
     * Get the enum status of the Side modification capability of this exam.
     * Exam::Pending    - exam side status is unkown if the record is invalid or
     *                    the images for the exam have not been downloaded
     * Exam::Fixed      - the Side column of the exam record is "none"
     * Exam::Changeable - the exam has one Side, either left or right with
                          one image acquisition
     * Exam::Swappable  - the exam has two image acquisitions, a left and a right
     * @return Exam::SideStatus the side status of the exam
     */
    Exam::SideStatus GetSideStatus();

  protected:
    Exam() {}
    ~Exam() {}

    /**
     * Fixes laterality and anonymization issues with dicom images.
     */
    void CleanImages();

  private:
    Exam( const Exam& ); // Not implemented
    void operator=( const Exam& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
