/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Image
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Image table
 */

#ifndef __Image_h
#define __Image_h

// Alder includes
#include <ActiveRecord.h>

// C++ includes
#include <string>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class Image : public ActiveRecord
  {
    public:
      static Image *New();
      vtkTypeMacro(Image, ActiveRecord);
      std::string GetName() const { return "Image"; }

      /**
       * Override parent class method.
       */
      void Remove();

      /**
       * Returns the image's code (used to determine the image's path
       * in the image data directory).
       * @return the InterviewId/ExamId/ImageId code
       * @throw  runtime_error
       */
      virtual std::string GetCode();

      /**
       * Get the full path to where the image associated with this record belongs.
       * @return the path to this image (excludes file name)
       * @throw  runtime_error
       */
      std::string GetFilePath();

      /**
       * Get the file name that this record represents (including path).
       * NOTE: this method depends on the file already existing: if it doesn't already
       * exist, an exception is thrown.
       * @return the full path and file name of this image
       * @throw  runtime_error
       */
      std::string GetFileName();

      /**
       * Create the path to the file name that this record represents
       *(including path and provided suffix) and returns the result
       * (with full path, file name and suffix).
       * NOTE: this method does not depend on the file already existing,
       * it simply uses the image's path and the provided extension
       * to create an empty file.
       * @param suffix a file extension
       * @return       the full path and file name of the image
       */
      std::string CreateFile(std::string const &suffix);

      /**
       * Once the file is written to the disk this method validates it.
       * It will unzip gzipped files and if the file is empty or unreadable
       * it will delete the file.
       * @return whether the file exists on disk
       */
      bool ValidateFile();

      /**
       * Set the dimensionality from dicom tag data.
       */
      void SetDimensionalityFromDICOM();

      /**
       * Get whether a particular user has rated this image.
       * @param user the User object to check against
       * @return     whether the user has rated this image
       * @throw      runtime_error
       */
      bool IsRatedBy(User* user);

      /**
       * Is this a dicom image? Determined via the modality
       * record associated with this image's parent exam.
       * @return whether this is a dicom image
       */
      bool IsDICOM();

      /**
       * Convenience method to get a DICOM tag value by its description string.
       * Accepted tag names are:
       *   AcquisitionDateTime         - 0x0008, 0x002a
       *   SeriesNumber                - 0x0020, 0x0011
       *   PatientName                 - 0x0010, 0x0010
       *   Laterality                  - 0x0020, 0x0060
       *   Manufacturer                - 0x0008, 0x0070
       *   PhotometricInterpretation   - 0x0028, 0x0004
       *   RecommendedDisplayFrameRate - 0x0008, 0x2144
       *   CineRate                    - 0x0018, 0x0040
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @param tagName the name of the dicom tag
       * @return        the value of the dicom tag
       * @throw         runtime_error
       */
      std::string GetDICOMTag(std::string const &tagName);

      /**
       * Get the acquisition date time.  Works only for dicom images.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return the value of the dicom AcquistionDateTime tag
       */
      std::string GetDICOMAcquisitionDateTime();

      /**
       * Get the number of rows, columns and frames of a dicom image.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return vector of dimensions in x, y, z: x, y only if 2D
       * @throw  runtime_error
       */
      std::vector< int > GetDICOMDimensions();

      /**
       * Set the side of the image's parent exam from its dicom tag
       * (laterality) if available.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return whether the exam side was changed
       */
      bool SwapExamSideFromDICOM();

      /**
       * Swap the side of the images' parent exam with the parent
       * exam's sibling.
       * @return whether the exam side was changed
       */
      bool SwapExamSide();

      /**
       * Anonymize a dicom image by clearing the PatientName tag.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return whether the file was anonymized
       * @throw  runtime_error
       */
      bool AnonymizeDICOM();

      /**
       * Erase the rectangular Patient Name box in Hologic DEXA images.
       * This method calls AnonymizeDICOM() after cleaning the actual
       * image.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return success or fail status of the clean
       * @throw  runtime_error
       */
      bool CleanHologicDICOM();

      /**
       * Convert photometric interpretation from YBR_FULL_422 to RGB.
       * No checking is done to ensure this is a dicom image: use
       * IsDICOM() method to check first.
       * @return success or fail status of the conversion
       * @throw  runtime_error
       */
      bool YBRToRGB();

      //@{
      /**
       * Returns the neighbouring interview in UId/VisitDate order.
       * A rating must be provided since an image may have more than one rating.
       * @param rating  the rating of the atlas image that matches this one
       * @param forward the order ASC (forward=true) or DESC to search by Interview.UId
       * @param id      the id of the root image that this atlas is for
       * @return        a neigbouring atlas image
       * @throw         runtime_error
       */
      vtkSmartPointer<Image> GetNeighbourAtlasImage(const int &rating,
        const bool &forward, const int &id = 0);
      vtkSmartPointer<Image> GetNextAtlasImage(const int &rating,
        const int &id = 0)
      { return this->GetNeighbourAtlasImage(rating, true, id); }
      vtkSmartPointer<Image> GetPreviousAtlasImage(int const &rating,
        const int &id = 0)
      { return this->GetNeighbourAtlasImage(rating, false, id); }
      //@}

      /**
       * Returns an atlas image matching the current image type.
       * @param rating the rating of the atlas image that matches this one
       * @return       an atlas image matching this image
       * @throw        runtime_error
       */
      vtkSmartPointer< Image > GetAtlasImage(const int &rating);

    protected:
      Image() {}
      ~Image() {}

      bool SwapExamSideTo(const std::string &side);

    private:
      Image(const Image&);  // Not implemented
      void operator=(const Image&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
