/*=========================================================================

  Program:   Teneos VTK extension library
  Module:    $RCSfile: vtkImageDataWriter.cxx,v $
  Language:  C++
  Date:      $Date: 2011-01-27 19:19:46 $
  Version:   $Revision: 1.14 $

  Copyright (c) Dean Inglis, Patrick Emond
  All rights reserved.
  See Copyright.txt for details.

=========================================================================*/
#include <vtkImageDataWriter.h>

// Alder includes
#include <Utilities.h>

// VTK includes
#include <vtkBMPReader.h>
#include <vtkBMPWriter.h>
#include <vtkGDCMImageReader.h>
#include <vtkGDCMImageWriter.h>
#include <vtkGESignaReader.h>
#include <vtkImageCast.h>
#include <vtkImageData.h>
#include <vtkImageShiftScale.h>
#include <vtkJPEGReader.h>
#include <vtkJPEGWriter.h>
#include <vtkMetaImageReader.h>
#include <vtkMetaImageWriter.h>
#include <vtkMINCImageReader.h>
#include <vtkMINCImageWriter.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkPNGReader.h>
#include <vtkPNGWriter.h>
#include <vtkPNMReader.h>
#include <vtkPNMWriter.h>
#include <vtkPostScriptWriter.h>
#include <vtkSmartPointer.h>
#include <vtkTIFFReader.h>
#include <vtkTIFFWriter.h>
#include <vtkXMLImageDataReader.h>
#include <vtkXMLImageDataWriter.h>

// C++ includes
#include <sstream>
#include <stdexcept>
#include <string>

vtkStandardNewMacro(vtkImageDataWriter);
vtkCxxSetObjectMacro(vtkImageDataWriter, Writer, vtkAlgorithm);

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataWriter::vtkImageDataWriter()
{
  this->Writer = NULL;
  this->ImageData = NULL;
  this->AutoDownCast = 0;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataWriter::~vtkImageDataWriter()
{
  if (this->ImageData)
  {
    this->ImageData->UnRegister(this);
    this->ImageData = NULL;
  }
  this->SetWriter(NULL);
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool vtkImageDataWriter::IsValidFileName(const char *fileName)
{
  if (NULL == fileName)
  {
    return false;
  }

  bool knownFileType = false;
  std::string fileExtension;

  // we need an instance of some of the readers so we can scan extensions
  vtkNew<vtkBMPReader> BMPReader;
  vtkNew<vtkGDCMImageReader> GDCMImageReader;
  vtkNew<vtkJPEGReader> JPEGReader;
  vtkNew<vtkMetaImageReader> MetaImageReader;
  vtkNew<vtkPNGReader> PNGReader;
  vtkNew<vtkPNMReader> PNMReader;
  vtkNew<vtkTIFFReader> TIFFReader;

  fileExtension = Alder::Utilities::getFileExtension(
    Alder::Utilities::toLower(fileName));

  // now search through each reader to see which 'likes' the file extension
  if (std::string::npos != Alder::Utilities::toLower(
        GDCMImageReader->GetFileExtensions()).find(fileExtension))
  { // DICOM
    knownFileType = true;
  }
  if (std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions()).find(fileExtension))
  { // BMP
    knownFileType = true;
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions()).find(fileExtension))
  { // JPEG file
    knownFileType = true;
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions()).find(fileExtension))
  { // MetaImage file
    knownFileType = true;
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions()).find(fileExtension))
  { // PNG file
    knownFileType = true;
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions()).find(fileExtension))
  { // PNM file
    knownFileType = true;
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions()).find(fileExtension))
  { // TIFF file
    knownFileType = true;
  }
  else if (".vti" == fileExtension)  // no GetFileExtensions() method
  { // VTI file
    knownFileType = true;
  }

  return knownFileType;
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataWriter::SetFileName(const char *fileName)
{
  std::string fileNameStr(fileName);

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

  bool unknownFileType = false;
  std::string fileExtension, fileNameOnly;

  // ok, we have a valid file name, get some details
  fileExtension = Alder::Utilities::getFileExtension(
    Alder::Utilities::toLower(this->FileName));
  fileNameOnly = Alder::Utilities::getFilenameName(this->FileName);

  // we need an instance of some of the readers so we can scan extensions
  vtkNew<vtkBMPReader> BMPReader;
  vtkNew<vtkGDCMImageReader> GDCMImageReader;
  vtkNew<vtkJPEGReader> JPEGReader;
  vtkNew<vtkMetaImageReader> MetaImageReader;
  vtkNew<vtkPNGReader> PNGReader;
  vtkNew<vtkPNMReader> PNMReader;
  vtkNew<vtkTIFFReader> TIFFReader;
  vtkNew<vtkXMLImageDataReader> XMLImageDataReader;

  vtkSmartPointer<vtkBMPWriter> BMPWriter =
    vtkSmartPointer<vtkBMPWriter>::New();
  vtkSmartPointer<vtkJPEGWriter> JPEGWriter =
    vtkSmartPointer<vtkJPEGWriter>::New();
  vtkSmartPointer<vtkGDCMImageWriter> GdcmWriter =
    vtkSmartPointer<vtkGDCMImageWriter>::New();
  vtkSmartPointer<vtkMetaImageWriter> MetaImageWriter =
    vtkSmartPointer<vtkMetaImageWriter>::New();
  vtkSmartPointer<vtkMINCImageWriter> MINCImageWriter =
    vtkSmartPointer<vtkMINCImageWriter>::New();
  vtkSmartPointer<vtkPNGWriter> PNGWriter =
    vtkSmartPointer<vtkPNGWriter>::New();
  vtkSmartPointer<vtkPNMWriter> PNMWriter =
    vtkSmartPointer<vtkPNMWriter>::New();
  vtkSmartPointer<vtkPostScriptWriter> PostScriptWriter =
    vtkSmartPointer<vtkPostScriptWriter>::New();
  vtkSmartPointer<vtkTIFFWriter> TIFFWriter =
    vtkSmartPointer<vtkTIFFWriter>::New();
  vtkSmartPointer<vtkXMLImageDataWriter> XMLImageDataWriter =
    vtkSmartPointer<vtkXMLImageDataWriter>::New();

  // now search through each reader to see which 'likes' the file extension
  if (std::string::npos != Alder::Utilities::toLower(
        GDCMImageReader->GetFileExtensions()).find(fileExtension))
  { // DICOM
    this->SetWriter(GdcmWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions()).find(fileExtension))
  { // BMP
    this->SetWriter(BMPWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions()).find(fileExtension))
  { // JPEG file
    this->SetWriter(JPEGWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions()).find(fileExtension))
  { // MetaImage file
    MetaImageWriter->SetCompression(false);
    this->SetWriter(MetaImageWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    MINCImageWriter->GetFileExtensions()).find(fileExtension))
  { // MINCImage file: the only writer that uses GetFileExtensions
    this->SetWriter(MINCImageWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions()).find(fileExtension))
  { // PNG file
    this->SetWriter(PNGWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions()).find(fileExtension))
  { // PNM file
    this->SetWriter(PNMWriter);
  }
  else if (".ps" == fileExtension)
  { // PS file
    this->SetWriter(PostScriptWriter);
  }
  else if (std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions()).find(fileExtension))
  { // TIFF file
    this->SetWriter(TIFFWriter);
  }
  else if (".vti" == fileExtension)  // no GetFileExtensions() method
  { // VTI file
    this->SetWriter(XMLImageDataWriter);
  }
  else  // don't know how to handle this file
  {
    unknownFileType = true;
  }

  if (unknownFileType)
  {
    std::stringstream stream;
    stream << __FILE__;
    stream << " ";
    stream << __LINE__;
    stream << " ";
    stream << "Unable to write '" << fileNameOnly << "', unknown file type.";
    throw std::runtime_error(stream.str());
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataWriter::SetInputData(vtkImageData *input)
{
  if (this->ImageData == input) return;

  if (this->ImageData)
  {
    this->ImageData->UnRegister(this);
  }
  this->ImageData = input;
  if (this->ImageData)
  {
    this->ImageData->Register(this);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataWriter::Write()
{
  // if the file name or writer are null simply return
  if (this->FileName.empty() ||
      NULL == this->Writer || NULL == this->ImageData)
  {
    return;
  }

  vtkImageData* image = this->ImageData;
  vtkNew<vtkImageCast > castFilter;

  if (this->AutoDownCast)
  {
    double range[2];
    this->ImageData->GetScalarRange(range);
    castFilter->SetInputData(this->ImageData);

    int type = this->ImageData->GetScalarType();
    castFilter->SetOutputScalarType(type);

    if (VTK_DOUBLE == type || VTK_FLOAT == type)
    {
      if (VTK_FLOAT_MIN <= range[0] &&
          VTK_FLOAT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToFloat();
      }
    }
    else
    {
      if (VTK_UNSIGNED_CHAR_MIN <= range[0] &&
          VTK_UNSIGNED_CHAR_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToUnsignedChar();
      }
      else if (VTK_CHAR_MIN <= range[0] &&
               VTK_CHAR_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToChar();
      }
      else if (VTK_UNSIGNED_SHORT_MIN <= range[0] &&
               VTK_UNSIGNED_SHORT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToUnsignedShort();
      }
      else if (VTK_SHORT_MIN <= range[0] &&
               VTK_SHORT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToShort();
      }
      else if (VTK_UNSIGNED_INT_MIN <= range[0] &&
               VTK_UNSIGNED_INT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToUnsignedInt();
      }
      else if (VTK_INT_MIN <= range[0] &&
               VTK_INT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToInt();
      }
      else if (VTK_UNSIGNED_INT_MIN <= range[0] &&
               VTK_UNSIGNED_INT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToUnsignedInt();
      }
      else if (VTK_INT_MIN <= range[0] &&
               VTK_INT_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToInt();
      }
      else if (VTK_UNSIGNED_LONG_MIN <= range[0] &&
               VTK_UNSIGNED_LONG_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToUnsignedLong();
      }
      else if (VTK_LONG_MIN <= range[0] &&
               VTK_LONG_MAX >= range[1])
      {
        castFilter->SetOutputScalarTypeToLong();
      }
      else
      {
        vtkWarningMacro(
          "Error: range cannot be casted down from "
          << range[0] <<", " << range[1]);
      }
    }

    castFilter->Update();
    image = castFilter->GetOutput();
  }

  // Ok, we have a valid file and writer, process based on writer type
  if (this->Writer->IsA("vtkXMLImageDataWriter"))
  {
    // we know that Writer must be a vtkXMLImageDataWriter object
    vtkXMLImageDataWriter* XMLWriter =
      vtkXMLImageDataWriter::SafeDownCast(this->Writer);
    XMLWriter->SetFileName(this->FileName.c_str());
    XMLWriter->SetInputData(image);
    XMLWriter->Write();
  }
  else  // if we get here then the reader is some form of vtkImageWriter
  {
    // we know that Writer must be a vtkImageWriter object
    vtkImageWriter* imageWriter = vtkImageWriter::SafeDownCast(this->Writer);
    imageWriter->SetFileName(this->FileName.c_str());

    double range[2];
    image->GetScalarRange(range);

    int scalarType = image->GetScalarType();

    // the following image writers only support the listed scalar types:
    // BMP, JPEG, PNM, PostScript => unsigned char
    // IMG => unsigned short
    // PNG => unsigned char, unsigned short
    // TIFF => unsigned char, unsigned short, float

    // The following writers only support unsigned char, force conversion
    if ((this->Writer->IsA("vtkBMPWriter") ||
         this->Writer->IsA("vtkJPEGWriter") ||
         this->Writer->IsA("vtkPNMWriter") ||
         this->Writer->IsA("vtkPostScriptWriter")) &&
         VTK_UNSIGNED_CHAR != scalarType)
    {
      if (VTK_UNSIGNED_CHAR_MIN <= range[0] &&
          VTK_UNSIGNED_CHAR_MAX >= range[1])
      {
        vtkNew<vtkImageCast > convertFilter;
        convertFilter->SetOutputScalarTypeToUnsignedChar();
        convertFilter->SetInputData(image);
        imageWriter->SetInputConnection(convertFilter->GetOutputPort());
      }
      else
      {
        vtkNew<vtkImageShiftScale > convertFilter;
        convertFilter->SetOutputScalarTypeToUnsignedChar();
        convertFilter->SetShift(-range[0]);
        if (VTK_UNSIGNED_CHAR_MAX > (range[1]-range[0]))
        {
          convertFilter->SetScale(1);
        }
        else
        {
          convertFilter->SetScale(VTK_UNSIGNED_CHAR_MAX/(range[1] - range[0]));
       }
        convertFilter->SetInputData(image);
        imageWriter->SetInputConnection(convertFilter->GetOutputPort());
      }
    }
    // TIFF supports float, so if we have doubles convert to float
    else if (this->Writer->IsA("vtkTIFFWriter") &&
      VTK_DOUBLE == scalarType)
    {
      vtkNew<vtkImageCast > convertFilter;
      convertFilter->SetOutputScalarTypeToFloat();
      convertFilter->SetInputData(image);
      imageWriter->SetInputConnection(convertFilter->GetOutputPort());
    }
    // IMG, PNG and TIFF support unsigned shorts, so convert to that but only
    // if the scalar type isn't supported
    else if ((this->Writer->IsA("vtkIMGWriter") &&
              VTK_UNSIGNED_SHORT != scalarType) ||
             (this->Writer->IsA("vtkPNGWriter") &&
              VTK_UNSIGNED_CHAR  != scalarType &&
              VTK_UNSIGNED_SHORT != scalarType) ||
             (this->Writer->IsA("vtkTIFFWriter") &&
              VTK_UNSIGNED_CHAR  != scalarType &&
              VTK_UNSIGNED_SHORT != scalarType &&
              VTK_FLOAT          != scalarType))
    {
      if (VTK_UNSIGNED_SHORT_MIN <= range[0] &&
          VTK_UNSIGNED_SHORT_MAX >= range[1])
      {
        vtkNew<vtkImageCast > convertFilter;
        convertFilter->SetOutputScalarTypeToUnsignedShort();
        convertFilter->SetInputData(image);
        imageWriter->SetInputConnection(convertFilter->GetOutputPort());
      }
      else
      {
        vtkNew<vtkImageShiftScale > convertFilter;
        convertFilter->SetOutputScalarTypeToUnsignedShort();
        convertFilter->SetShift(-range[0]);
        if ((range[1]-range[0]) < VTK_UNSIGNED_SHORT_MAX)
        {
          convertFilter->SetScale(1);
        }
        else
        {
          convertFilter->SetScale(VTK_UNSIGNED_SHORT_MAX/(range[1] - range[0]));
        }
        convertFilter->SetInputData(image);
        imageWriter->SetInputConnection(convertFilter->GetOutputPort());
      }
    }
    // otherwise leave the type alone
    else
    {
      imageWriter->SetInputData(image);
    }

    imageWriter->Write();
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataWriter::PrintSelf(ostream &os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "FileName: " << this->FileName << "\n";
}
