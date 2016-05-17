/*=========================================================================

  Module:    vtkImageDataReader.cxx
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd AT mcmaster DOT ca>
  Author:    Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkImageDataReader.h>

// Alder inludes
#include <Utilities.h>

// VTK includes
#include <vtkBMPReader.h>
#include <vtkGDCMImageReader.h>
#include <vtkGESignaReader.h>
#include <vtkImageData.h>
#include <vtkJPEGReader.h>
#include <vtkMedicalImageProperties.h>
#include <vtkMetaImageReader.h>
#include <vtkMINCImageReader.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkPNMReader.h>
#include <vtkSLCReader.h>
#include <vtkStringArray.h>
#include <vtkTIFFReader.h>
#include <vtkXMLImageDataReader.h>

// GDCM includes
#include <gdcmDirectoryHelper.h>
#include <gdcmImageReader.h>

// C++ includes
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

vtkStandardNewMacro(vtkImageDataReader);
vtkCxxSetObjectMacro(vtkImageDataReader, Reader, vtkAlgorithm);

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataReader::vtkImageDataReader()
{
  this->Reader = NULL;
  this->MedicalImageProperties =
    vtkSmartPointer<vtkMedicalImageProperties>::New();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataReader::~vtkImageDataReader()
{
  this->SetReader(NULL);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkMedicalImageProperties* vtkImageDataReader::GetMedicalImageProperties()
{
  return this->MedicalImageProperties;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataReader::SetFileName(const char* fileName)
{
  std::string fileExtension, filePath, fileNameOnly,
    fileNameStr(fileName);

  if (this->FileName.empty() && NULL == fileName)
  {
    return;
  }

  if (!this->FileName.empty() &&
      !fileNameStr.empty() &&
      (this->FileName == fileNameStr))
  {
    return;
  }

  // delete and set the file name to empty
  this->FileName.clear();

  if (!fileNameStr.empty())
  {
    this->FileName = fileNameStr;
  }

  // mark the object as modified
  this->Modified();

  // don't do anything else if the new file name is null
  if (this->FileName.empty())
  {
    return;
  }

  // make sure FileName exists, throw an exception if it doesn't
  if (!Alder::Utilities::fileExists(this->FileName))
  {
    std::stringstream error;
    error << "File '" << this->FileName << "' not found.";
    throw std::runtime_error(error.str());
  }

  fileExtension = Alder::Utilities::getFileExtension(this->FileName);
  fileNameOnly = Alder::Utilities::getFilenameName(this->FileName);

  // need an instance of all readers to scan valid extensions
  vtkSmartPointer<vtkBMPReader> BMPReader =
    vtkSmartPointer<vtkBMPReader>::New();
  vtkSmartPointer<vtkGDCMImageReader> GDCMImageReader =
    vtkSmartPointer<vtkGDCMImageReader>::New();
  vtkSmartPointer<vtkGESignaReader> GESignaReader =
    vtkSmartPointer<vtkGESignaReader>::New();
  vtkSmartPointer<vtkJPEGReader> JPEGReader =
    vtkSmartPointer<vtkJPEGReader>::New();
  vtkSmartPointer<vtkMetaImageReader> MetaImageReader =
    vtkSmartPointer<vtkMetaImageReader>::New();
  vtkSmartPointer<vtkMINCImageReader> MINCImageReader =
    vtkSmartPointer<vtkMINCImageReader>::New();
  vtkSmartPointer<vtkPNGReader> PNGReader =
    vtkSmartPointer<vtkPNGReader>::New();
  vtkSmartPointer<vtkPNMReader> PNMReader =
    vtkSmartPointer<vtkPNMReader>::New();
  vtkSmartPointer<vtkSLCReader> SLCReader =
    vtkSmartPointer<vtkSLCReader>::New();
  vtkSmartPointer<vtkTIFFReader> TIFFReader =
    vtkSmartPointer<vtkTIFFReader>::New();
  vtkSmartPointer<vtkXMLImageDataReader> XMLImageDataReader =
    vtkSmartPointer<vtkXMLImageDataReader>::New();

  // search through each reader to see which 'likes' the file extension

  if (std::string::npos != Alder::Utilities::toLower(
        GDCMImageReader->GetFileExtensions()).find(fileExtension))
  { // DICOM
    this->SetReader(GDCMImageReader);
  }
  else if (GDCMImageReader->CanReadFile(this->FileName.c_str()))
  {
    this->SetReader(GDCMImageReader);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions()).find(fileExtension))
  { // BMP
    if (BMPReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(BMPReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    GESignaReader->GetFileExtensions()).find(fileExtension))
  { // GESigna file
    if (GESignaReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(GESignaReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions()).find(fileExtension))
  { // JPEG file
    if (JPEGReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(JPEGReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions()).find(fileExtension))
  { // MetaImage file
    if (MetaImageReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(MetaImageReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MINCImageReader->GetFileExtensions()).find(fileExtension))
  { // MINCImage file
    if (MINCImageReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(MINCImageReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions()).find(fileExtension))
  { // PNG file
    if (PNGReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(PNGReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions()).find(fileExtension))
  { // PNM file
    if (PNMReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(PNMReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    SLCReader->GetFileExtensions()).find(fileExtension))
  { // SLC file
    if (SLCReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(SLCReader);
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions()).find(fileExtension))
  { // TIFF file
    if (TIFFReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(TIFFReader);
    }
  }
  else if (".vti" == fileExtension)  // no GetFileExtensions() method
  { // VTI file
    if (XMLImageDataReader->CanReadFile(this->FileName.c_str()))
    {
      this->SetReader(XMLImageDataReader);
    }
  }
  else  // don't know how to handle this file, set the reader to NULL and
        // mark the file type as unknown
  {
    this->SetReader(NULL);
    std::stringstream error;
    error << "Unable to read '" << fileNameOnly << "', unknown file type.";
    throw std::runtime_error(error.str());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool vtkImageDataReader::IsValidFileName(const char* fileName)
{
  if (NULL == fileName || !Alder::Utilities::fileExists(fileName))
  {
    return false;
  }

  bool knownFileType = false;
  std::string fileExtension;

  // ok, we have a valid file or directory name, get some details
  fileExtension = Alder::Utilities::getFileExtension(
    Alder::Utilities::toLower(fileName));

  // we need an instance of all readers so we can scan extensions
  vtkNew<vtkBMPReader> BMPReader;
  vtkNew<vtkGDCMImageReader> GDCMImageReader;
  vtkNew<vtkGESignaReader> GESignaReader;
  vtkNew<vtkJPEGReader> JPEGReader;
  vtkNew<vtkMetaImageReader> MetaImageReader;
  vtkNew<vtkMINCImageReader> MINCImageReader;
  vtkNew<vtkPNGReader> PNGReader;
  vtkNew<vtkPNMReader> PNMReader;
  vtkNew<vtkSLCReader> SLCReader;
  vtkNew<vtkTIFFReader> TIFFReader;
  vtkNew<vtkXMLImageDataReader> XMLImageDataReader;

  // now search through each reader to see which 'likes' the file extension
  if (std::string::npos != Alder::Utilities::toLower(
        GDCMImageReader->GetFileExtensions()).find(fileExtension))
  { // DICOM
    if (GDCMImageReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions()).find(fileExtension))
  { // BMP
    if (BMPReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    GESignaReader->GetFileExtensions()).find(fileExtension))
  { // GESigna file
    if (GESignaReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions()).find(fileExtension))
  { // JPEG file
    if (JPEGReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions()).find(fileExtension))
  { // MetaImage file
    if (MetaImageReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MINCImageReader->GetFileExtensions()).find(fileExtension))
  { // MINCImage file
    if (MINCImageReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions()).find(fileExtension))
  { // PNG file
    if (PNGReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions()).find(fileExtension))
  { // PNM file
    if (PNMReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    SLCReader->GetFileExtensions()).find(fileExtension))
  { // SLC file
    if (SLCReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions()).find(fileExtension))
  { // TIFF file
    if (TIFFReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else if (".vti" == fileExtension)  // no GetFileExtensions() method
  { // VTI file
    if (XMLImageDataReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }
  else  // last ditch effort to read a dicom
  {
    if (GDCMImageReader->CanReadFile(fileName))
    {
      knownFileType = true;
    }
  }

  return knownFileType;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkImageDataReader::GetOutputAsNewInstance()
{
  vtkImageData* newImage = NULL;
  vtkImageData* image = this->GetOutput();

  if (image)
  {
    // create a copy of the image
    // this copy MUST be deleted by the caller of this method
    newImage = image->NewInstance();
    newImage->DeepCopy(image);
  }

  return newImage;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkImageDataReader::GetOutput()
{
  std::string fileNameOnly;
  vtkXMLImageDataReader* XMLReader = NULL;
  vtkGDCMImageReader* gdcmReader = NULL;
  vtkImageReader2* imageReader = NULL;
  vtkImageData* image = NULL;
  this->MedicalImageProperties->Clear();

  // if the file name or reader are null simply return null
  if (this->FileName.empty() || NULL == this->Reader)
  {
    return NULL;
  }

  // we might need this
  fileNameOnly = Alder::Utilities::getFilenameName(this->FileName);

  // Ok, we have a valid file and reader, process based on reader type
  if (this->Reader->IsA("vtkXMLImageDataReader"))
  {
    // we know that Reader must be a vtkXMLImageDataReader object
    XMLReader = vtkXMLImageDataReader::SafeDownCast(this->Reader);

    // see if we have already read the data from the disk
    // and return it if we have
    if (this->ReadMTime >= this->GetMTime())
    {
      image = XMLReader->GetOutput();
    }
    else  // this reader is not up to date, re-read the file
    {
      // check that we can read the file
      if (!XMLReader->CanReadFile(this->FileName.c_str()))
      {
        std::stringstream error;
        error << "Unable to read '" << fileNameOnly << "' as a VTI file.";
        throw std::runtime_error(error.str());
      }

      XMLReader->SetFileName(this->FileName.c_str());

      // get a reference to the (updated) output image
      XMLReader->Update();
      image = XMLReader->GetOutput();
    }
  }
  else if (this->Reader->IsA("vtkGDCMImageReader"))
  {
    gdcmReader = vtkGDCMImageReader::SafeDownCast(this->Reader);

    // see if we have already read the data from the disk
    // and return it if we have
    if (this->ReadMTime >= this->GetMTime())
    {
      image = gdcmReader->GetOutput();
    }
    else
    {
      gdcmReader->SetFileName(this->FileName.c_str());
      gdcmReader->Update();
      image = gdcmReader->GetOutput();

      if (!image)
      {
        std::stringstream error;
        error << "Failed to read dicom file '"
                 << this->FileName << "'.";
        throw std::runtime_error(error.str());
      }
    }
  }
  else  // if we get here then the reader is some form of vtkImageReader2
  {
    // we know that Reader must be a vtkImageReader2 object
    imageReader = vtkImageReader2::SafeDownCast(this->Reader);

    // if this reader is not up to date, re-read the file
    if (this->ReadMTime < this->GetMTime())
    {
      // the GDCM reader has not implemented CanReadFile, so skip that check
      if (!imageReader->IsA("vtkGDCMImageReader"))
      {
        // check that we can read the file
        if (!imageReader->CanReadFile(this->FileName.c_str()))
        {
          std::stringstream error;
          error << "Unable to read '" << fileNameOnly << "' as a ";
          error << imageReader->GetDescriptiveName() << " file.";
          throw std::runtime_error(error.str());
        }
      }

      imageReader->SetFileName(this->FileName.c_str());

      // get a reference to the (updated) output image
      imageReader->Update();
    }
    image = imageReader->GetOutput();
  }

  // if we did get an image, check that it has valid extents:
  // GetNumberOfCells is zero for invalid vtkImageData Extents
  if (image)
  {
    vtkIdType nCells = image->GetNumberOfCells();
    if (nCells == 0)
    {
      return NULL;
    }
  }

  // set up the medical image properties
  if (this->Reader->IsA("vtkGESignaReader"))
  {
    vtkGESignaReader* imageReader =
      vtkGESignaReader::SafeDownCast(this->Reader);
    this->MedicalImageProperties->DeepCopy(
      imageReader->GetMedicalImageProperties());
  }
  else if (this->Reader->IsA("vtkGDCMImageReader"))
  {
    vtkGDCMImageReader* imageReader =
      vtkGDCMImageReader::SafeDownCast(this->Reader);
    this->MedicalImageProperties->DeepCopy(
      imageReader->GetMedicalImageProperties());

    gdcm::ImageReader reader;
    reader.SetFileName(imageReader->GetFileName());
    reader.Read();
    const gdcm::File &file = reader.GetFile();
    const gdcm::DataSet &ds = file.GetDataSet();

    std::map< std::string, gdcm::Tag > dicomMap;
    dicomMap["AcquisitionDateTime"] = gdcm::Tag(0x0008, 0x002a);
    dicomMap["SeriesNumber"] = gdcm::Tag(0x0020, 0x0011);
    dicomMap["CineRate"] = gdcm::Tag(0x0018, 0x0040);
    dicomMap["RecommendedDisplayFrameRate"] = gdcm::Tag(0x0008, 0x2114);

    for (auto it = dicomMap.cbegin(); it != dicomMap.cend(); ++it)
    {
      if (ds.FindDataElement(it->second))
      {
        std::string name = it->first;
        std::string value =
          gdcm::DirectoryHelper::GetStringValueFromTag(it->second, ds);
        if (!value.empty())
          value = Alder::Utilities::trim(value);
        this->MedicalImageProperties->AddUserDefinedValue(
          name.c_str(), value.c_str());
      }
    }
  }

  this->ReadMTime.Modified();
  return image;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataReader::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
