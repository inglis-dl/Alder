/*=========================================================================

  Module:    vtkImageDataReader.h
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkImageDataReader
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Read an image.
 *
 * In order to simplify the process of opening ImageData from disk this
 * class wraps all image reader classes that extend vtkImageReader2, such
 * as vtkJPEGReader, vtkPNGReader vtkXMLImageDataReader, etc.  The type of
 * reader used is determined by file extension.  This class also supports
 * VTK's XML image format using vtkXMLImageDataReader which it identifies
 * by the extension .vti
 *
 * GDCM's reader is used instead of VTK's native DICOM reader.
 */
#ifndef __vtkImageDataReader_h
#define __vtkImageDataReader_h

// VTK includes
#include <vtkObject.h>
#include <vtkSmartPointer.h>
#include <vtkTimeStamp.h>

// C++ includes
#include <string>

class vtkAlgorithm;
class vtkImageData;
class vtkMedicalImageProperties;

class vtkImageDataReader : public vtkObject
{
  public:
    static vtkImageDataReader* New();
    vtkTypeMacro(vtkImageDataReader, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    //@{
    /**
     * Set/Get the file name to be opened by the reader.
     * If a directory is selected then the reader will attempt to open all
     * DICOM files in that directory.
     */
    virtual void SetFileName(const char* name);
    std::string GetFileName() { return this->FileName; }
    //@}

    /**
     * Returns a reference to a vtkImageData object created by opening the
     * current FileName.  If FileName is changed then the next time this
     * method is called the old reference will no longer be valid.
     */
    virtual vtkImageData* GetOutput();

    /**
     * Creates a new instance of a vtkImageData object created by opening
     * the current FileName and returns it.  A reference to this object is
     * not kept and it is up to the caller of this method to delete the
     * object.
     */
    vtkImageData* GetOutputAsNewInstance();

    /**
     * Get the reader so we can query what type it was among other things.
     */
    vtkGetObjectMacro(Reader, vtkAlgorithm);

    /**
     * Before trying to do anything with the named file, check if it is in fact
     * readable by this object.
     */
    static bool IsValidFileName(const char* name);

    /**
     * Get the medical image properties if the image, if there are any.
     * For DICOM images, we add additional user defined tags.
     */
    vtkMedicalImageProperties* GetMedicalImageProperties();

  protected:
    vtkImageDataReader();
    ~vtkImageDataReader();

    /**
     * Sets the the reader (keeping a reference to it and deleting the
     * old one).
     */
    virtual void SetReader(vtkAlgorithm* reader);

    std::string FileName;
    vtkAlgorithm* Reader;
    vtkTimeStamp ReadMTime;
    vtkSmartPointer<vtkMedicalImageProperties> MedicalImageProperties;

  private:
    vtkImageDataReader(const vtkImageDataReader&);  /** Not implemented. */
    void operator=(const vtkImageDataReader&);  /** Not implemented. */
};

#endif
