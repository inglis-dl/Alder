/*=========================================================================

  Module:    vtkImageDataWriter.h
  Language:  C++

  Copyright (c) Dean Inglis, Patrick Emond
  All rights reserved.
  See Copyright.txt for details.

=========================================================================*/
//
// .NAME vtkImageDataWriter - writes any type of VTK image file
//
// .SECTION Description
// In order to simplify the process of writing vtkImageData to disk this
// class wraps all image writer classes that extend vtkImageWriter, such
// as vtkJPEGWriter, vtkPNGWriter vtkMetaImageWriter, etc.  The type of
// writer used is determined by file extension.  This class also supports
// VTK's XML image format using vtkXMLImageDataWriter which it identifies
// by the extension .vti .
//
// The is no API yet for setting compression or other writer parameters.
// It is up to the user to know whether the file they are writing is 2D
// or 3D and if the requested file type is appropriate.

#ifndef __vtkImageDataWriter_h
#define __vtkImageDataWriter_h

// VTK includes
#include <vtkObject.h>

// C++ includes
#include <string>

class vtkAlgorithm;
class vtkImageData;

class vtkImageDataWriter : public vtkObject
{
  public:
    static vtkImageDataWriter* New();
    vtkTypeMacro(vtkImageDataWriter, vtkObject);
    void PrintSelf(ostream& os, vtkIndent indent);

    // Description:
    // Set/get the file name to be wrtitten to by the reader.
    virtual void SetFileName(const char* name);
    std::string GetFileName() { return this->FileName; }

    // Description:
    // Set the input to the writer.
    void SetInputData(vtkImageData *data);

    // Description:
    // Get the reader so we can query what type it was among other things.
    vtkGetObjectMacro(Writer, vtkAlgorithm);

    // Description:
    //  Write data to disk.
    void Write();

    // Description:
    // Before trying to do anything with the named file, check if it is in fact
    // writable by this object.
    static bool IsValidFileName(const char* name);

    // Description:
    // When writing, see if the scalar range will allow fitting within a
    // smaller number
    // of bytes.  Default is Off.
    vtkSetMacro(AutoDownCast, int);
    vtkGetMacro(AutoDownCast, int);
    vtkBooleanMacro(AutoDownCast, int);

  protected:
    // Constructor and destructor
    vtkImageDataWriter();
    ~vtkImageDataWriter();

    // Description:
    // Sets the the reader (keeping a reference to it and deleting the any
    // old one).
    virtual void SetWriter(vtkAlgorithm* writer);

    std::string FileName;
    vtkAlgorithm* Writer;
    vtkImageData* ImageData;
    int AutoDownCast;

  private:
    vtkImageDataWriter(const vtkImageDataWriter&);  // Not implemented.
    void operator=(const vtkImageDataWriter&);  // Not implemented.
};

#endif
