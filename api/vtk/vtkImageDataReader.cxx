/*=========================================================================

  Module:    vtkImageDataReader.cxx
  Program:   Alder (CLSA Medical Image Quality Assessment Tool)
  Language:  C++
  Author:    Patrick Emond <emondpd@mcmaster.ca>
  Author:    Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/
#include "vtkImageDataReader.h"

#include "Utilities.h"

#include "vtkBMPReader.h"
#include "vtkGDCMImageReader.h"
#include "vtkGESignaReader.h"
#include "vtkImageData.h"
#include "vtkJPEGReader.h"
#include "vtkMetaImageReader.h"
#include "vtkMINCImageReader.h"
#include "vtkObjectFactory.h"
#include "vtkPNGReader.h"
#include "vtkPNMReader.h"
#include "vtkSLCReader.h"
#include "vtkStringArray.h"
#include "vtkTIFFReader.h"
#include "vtkXMLImageDataReader.h"

#include <sstream>
#include <stdexcept>

vtkStandardNewMacro( vtkImageDataReader );

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataReader::vtkImageDataReader()
{
  this->Reader = NULL;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageDataReader::~vtkImageDataReader()
{
  if( this->Reader )
  {
    this->Reader->Delete();
    this->Reader = NULL;
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataReader::SetReader( vtkAlgorithm* reader )
{
  if( this->Reader != reader )
  {
    this->Reader = reader;
    this->Modified();
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataReader::SetFileName( const char* fileName )
{
  bool unknownFileType = false;
  std::string fileExtension, filePath, fileNameOnly, 
    fileNameStr( fileName );

  vtkBMPReader*          BMPReader;
  vtkGDCMImageReader*    GdcmReader;
  vtkGESignaReader*      GESignaReader;
  vtkJPEGReader*         JPEGReader;
  vtkMetaImageReader*    MetaImageReader;
  vtkMINCImageReader*    MINCImageReader;
  vtkPNGReader*          PNGReader;
  vtkPNMReader*          PNMReader;
  vtkSLCReader*          SLCReader;
  vtkTIFFReader*         TIFFReader;
  vtkXMLImageDataReader* XMLImageDataReader;

  if( this->FileName.empty() && fileName == NULL )
  {
    return;
  }

  if( !this->FileName.empty() && 
      !fileNameStr.empty() && 
      ( this->FileName == fileNameStr ) ) 
  {
    return;
  }

  // delete and set the file name to empty
  this->FileName.clear();

  if( !fileNameStr.empty() )
  {
    this->FileName = fileNameStr;
  }

  // mark the object as modified
  this->Modified();

  // don't do anything else if the new file name is null
  if( this->FileName.empty() )
  {
    return;
  }

  // make sure FileName exists, throw an exception if it doesn't
  if( Alder::Utilities::fileExists( this->FileName ) )
  {
    std::stringstream error;
    error << "File '" << this->FileName << "' not found.";
    throw std::runtime_error( error.str() );
  }

  // ok, we have a valid file name, get some details

  fileExtension = Alder::Utilities::getFileExtension( this->FileName );
  fileNameOnly = Alder::Utilities::getFilenameName( this->FileName );
  
  // we need an instance of all readers so we can scan extensions
  BMPReader          = vtkBMPReader::New();
  GdcmReader         = vtkGDCMImageReader::New();
  GESignaReader      = vtkGESignaReader::New();
  JPEGReader         = vtkJPEGReader::New();
  MetaImageReader    = vtkMetaImageReader::New();
  MINCImageReader    = vtkMINCImageReader::New();
  PNGReader          = vtkPNGReader::New();
  PNMReader          = vtkPNMReader::New();
  SLCReader          = vtkSLCReader::New();
  TIFFReader         = vtkTIFFReader::New();
  XMLImageDataReader = vtkXMLImageDataReader::New();

  // now search through each reader to see which 'likes' the file extension

  if( std::string::npos != Alder::Utilities::toLower(
        GdcmReader->GetFileExtensions() ).find( fileExtension ) )
  { // DICOM
    this->SetReader( GdcmReader );
  }
  else if( GdcmReader->CanReadFile( this->FileName.c_str() ) )
  {
    this->SetReader( GdcmReader );
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions() ).find( fileExtension ) )
  { // BMP
    if( BMPReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( BMPReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    GESignaReader->GetFileExtensions() ).find( fileExtension ) )
  { // GESigna file
    if( GESignaReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( GESignaReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions() ).find( fileExtension ) )
  { // JPEG file
    if( JPEGReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( JPEGReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions() ).find( fileExtension ) )
  { // MetaImage file
    if( MetaImageReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( MetaImageReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    MINCImageReader->GetFileExtensions() ).find( fileExtension ) )
  { // MINCImage file
    if( MINCImageReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( MINCImageReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions() ).find( fileExtension ) )
  { // PNG file
    if( PNGReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( PNGReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions() ).find( fileExtension ) )
  { // PNM file
    if( PNMReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( PNMReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    SLCReader->GetFileExtensions() ).find( fileExtension ) )
  { // SLC file
    if( SLCReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( SLCReader );
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions() ).find( fileExtension ) )
  { // TIFF file
    if( TIFFReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( TIFFReader );
    }
  }
  else if( fileExtension == ".vti" ) // no GetFileExtensions() method
  { // VTI file
    if( XMLImageDataReader->CanReadFile( this->FileName.c_str() ) )
    {
      this->SetReader( XMLImageDataReader );
    }
  }
  else // don't know how to handle this file, set the reader to NULL and
       // mark the file type as unknown
  {
    if( this->Reader ) 
    {
      this->Reader->Delete();
      this->Reader = NULL;
    }
    unknownFileType = true;
  }

  // delete all the readers (valid reader is now referenced in this->Reader
  BMPReader->Delete();
  GdcmReader->Delete();
  GESignaReader->Delete();
  JPEGReader->Delete();
  MetaImageReader->Delete();
  MINCImageReader->Delete();
  PNGReader->Delete();
  PNMReader->Delete();
  SLCReader->Delete();
  TIFFReader->Delete();
  XMLImageDataReader->Delete();

  if( unknownFileType )
  {
    std::stringstream error;
    error << "Unable to read '" << fileNameOnly << "', unknown file type.";
    throw std::runtime_error( error.str() );
  }
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
bool vtkImageDataReader::IsValidFileName( const char* fileName )
{
  if( fileName == NULL || !Alder::Utilities::fileExists( fileName ) )
  {
    return false;
  }

  bool knownFileType = false;
  std::string fileExtension;

  vtkBMPReader*          BMPReader;
  vtkGDCMImageReader*    GdcmReader;
  vtkGESignaReader*      GESignaReader;
  vtkJPEGReader*         JPEGReader;
  vtkMetaImageReader*    MetaImageReader;
  vtkMINCImageReader*    MINCImageReader;
  vtkPNGReader*          PNGReader;
  vtkPNMReader*          PNMReader;
  vtkSLCReader*          SLCReader;
  vtkTIFFReader*         TIFFReader;
  vtkXMLImageDataReader* XMLImageDataReader;

  // ok, we have a valid file or directory name, get some details
  fileExtension = Alder::Utilities::getFileExtension(
    Alder::Utilities::toLower( fileName ) );

  // we need an instance of all readers so we can scan extensions
  BMPReader          = vtkBMPReader::New();
  GdcmReader         = vtkGDCMImageReader::New();
  GESignaReader      = vtkGESignaReader::New();
  JPEGReader         = vtkJPEGReader::New();
  MetaImageReader    = vtkMetaImageReader::New();
  MINCImageReader    = vtkMINCImageReader::New();
  PNGReader          = vtkPNGReader::New();
  PNMReader          = vtkPNMReader::New();
  SLCReader          = vtkSLCReader::New();
  TIFFReader         = vtkTIFFReader::New();
  XMLImageDataReader = vtkXMLImageDataReader::New();

  // now search through each reader to see which 'likes' the file extension
  if( std::string::npos != Alder::Utilities::toLower(
        GdcmReader->GetFileExtensions() ).find( fileExtension ) )
  { // DICOM
    knownFileType = true;
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    BMPReader->GetFileExtensions() ).find( fileExtension ) )
  { // BMP
    if( BMPReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    GESignaReader->GetFileExtensions() ).find( fileExtension ) )
  { // GESigna file
    if( GESignaReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    JPEGReader->GetFileExtensions() ).find( fileExtension ) )
  { // JPEG file
    if( JPEGReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    MetaImageReader->GetFileExtensions() ).find( fileExtension ) )
  { // MetaImage file
    if( MetaImageReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    MINCImageReader->GetFileExtensions() ).find( fileExtension ) )
  { // MINCImage file
    if( MINCImageReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    PNGReader->GetFileExtensions() ).find( fileExtension ) )
  { // PNG file
    if( PNGReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    PNMReader->GetFileExtensions() ).find( fileExtension ) )
  { // PNM file
    if( PNMReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    SLCReader->GetFileExtensions() ).find( fileExtension ) )
  { // SLC file
    if( SLCReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( std::string::npos != Alder::Utilities::toLower(
    TIFFReader->GetFileExtensions() ).find( fileExtension ) )
  { // TIFF file
    if( TIFFReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else if( fileExtension == ".vti" ) // no GetFileExtensions() method
  { // VTI file
    if( XMLImageDataReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }
  else // last ditch effort to read a dicom
  {
    if( GdcmReader->CanReadFile( fileName ) )
    {
      knownFileType = true;
    }
  }

  // delete all the readers (valid reader is now referenced in this->Reader
  BMPReader->Delete();
  GdcmReader->Delete();
  GESignaReader->Delete();
  JPEGReader->Delete();
  MetaImageReader->Delete();
  MINCImageReader->Delete();
  PNGReader->Delete();
  PNMReader->Delete();
  SLCReader->Delete();
  TIFFReader->Delete();
  XMLImageDataReader->Delete();

  return knownFileType;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkImageDataReader::GetOutputAsNewInstance()
{
  vtkImageData* newImage = NULL;
  vtkImageData* image = this->GetOutput();

  if( image )
  {
    // create a copy of the image
    // this copy MUST be deleted by the caller of this method
    newImage = image->NewInstance();
    newImage->DeepCopy( image );
  }

  return newImage;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
vtkImageData* vtkImageDataReader::GetOutput()
{
  std::string fileNameOnly;
  vtkXMLImageDataReader* XMLReader;
  vtkGDCMImageReader* gdcmReader;
  vtkImageReader2* imageReader;
  vtkImageData* image = NULL;

  // if the file name or reader are null simply return null
  if( this->FileName.empty() || NULL == this->Reader )
  {
    return NULL;
  }

  // we might need this
  fileNameOnly = Alder::Utilities::getFilenameName( this->FileName );

  // Ok, we have a valid file and reader, process based on reader type
  if( this->Reader->IsA( "vtkXMLImageDataReader" ) )
  {
    // we know that Reader must be a vtkXMLImageDataReader object
    XMLReader = vtkXMLImageDataReader::SafeDownCast( this->Reader );

    // see if we have already read the data from the disk, and return it if we have
    if( this->ReadMTime >= this->GetMTime() )
    {
      image = XMLReader->GetOutput();
    }
    else // this reader is not up to date, re-read the file
    {
      // check that we can read the file
      if( !XMLReader->CanReadFile( this->FileName.c_str() ) )
      {
        std::stringstream error;
        error << "Unable to read '" << fileNameOnly << "' as a VTI file.";
        throw std::runtime_error( error.str() );
      }

      XMLReader->SetFileName( this->FileName.c_str() );

      // get a reference to the (updated) output image
      XMLReader->Update();
      image = XMLReader->GetOutput();
    }
  }
  else if( this->Reader->IsA( "vtkGDCMImageReader" ) )
  {
    gdcmReader = vtkGDCMImageReader::SafeDownCast( this->Reader );

    // see if we have already read the data from the disk, and return it if we have
    if( this->ReadMTime >= this->GetMTime() )
    {
      image = gdcmReader->GetOutput();
    }
    else
    {
      gdcmReader->SetFileName( this->FileName.c_str() );
      gdcmReader->Update();
      image = gdcmReader->GetOutput();

      if( !image )
      {
        std::stringstream error;
        error << "Failed to read dicom file '"
                 << this->FileName << "'.";
        throw std::runtime_error( error.str() );
      }
    }
  }
  else // if we get here then the reader is some form of vtkImageReader2
  {
    // we know that Reader must be a vtkImageReader2 object
    imageReader = vtkImageReader2::SafeDownCast( this->Reader );

    // see if we have already read the data from the disk, and return it if we have
    if( this->ReadMTime >= this->GetMTime() )
    {
      image = imageReader->GetOutput();
    }
    else // this reader is not up to date, re-read the file
    {
      // the GDCM reader has not implemented CanReadFile, so skip that check
      if( !imageReader->IsA( "vtkGDCMImageReader" ) )
      {
        // check that we can read the file
        if( !imageReader->CanReadFile( this->FileName.c_str() ) )
        {
          std::stringstream error;
          error << "Unable to read '" << fileNameOnly << "' as a ";
          error << imageReader->GetDescriptiveName() << " file.";
          throw std::runtime_error( error.str() );
        }
      }

      imageReader->SetFileName( this->FileName.c_str() );

      // get a reference to the (updated) output image
      imageReader->Update();
      image = imageReader->GetOutput();
    }
  }

  this->ReadMTime.Modified();
  return image;
}

//-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void vtkImageDataReader::PrintSelf( ostream& os, vtkIndent indent )
{
  this->Superclass::PrintSelf( os, indent );

  os << indent << "FileName: " << this->FileName << "\n";
}
