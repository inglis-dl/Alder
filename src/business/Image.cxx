/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Image.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Image.h>

// Alder includes
#include <Configuration.h>
#include <Exam.h>
#include <Interview.h>
#include <Rating.h>
#include <User.h>
#include <Utilities.h>

// VTK includes
#include <vtkDirectory.h>
#include <vtkImageData.h>
#include <vtkImageDataReader.h>
#include <vtkImageCanvasSource2D.h>
#include <vtkImageFlip.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// GDCM includes
#include <gdcmAnonymizer.h>
#include <gdcmDirectoryHelper.h>
#include <gdcmImageReader.h>
#include <gdcmReader.h>
#include <gdcmTrace.h>
#include <gdcmWriter.h>

// vtk-dicom includes
#include <vtkDICOMCompiler.h>
#include <vtkDICOMMetaData.h>
#include <vtkDICOMParser.h>
#include <vtkDICOMReader.h>

#include <stdexcept>

namespace Alder
{
  vtkStandardNewMacro( Image );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetCode()
  {
    this->AssertPrimaryId();

    // get the exam's code and add append the image id
    vtkSmartPointer< Exam > exam;
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "Image has no parent exam!" );

    std::stringstream stream;
    stream << exam->GetCode() << "/" << this->Get( "Id" ).ToString();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFilePath()
  {
    this->AssertPrimaryId();

    vtkSmartPointer< Exam > exam;
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "Image has no parent exam!" );

    // the image file's directory is simply the image data path and the exam code
    std::stringstream stream;
    stream << Application::GetInstance()->GetConfig()->GetValue( "Path", "ImageData" )
           << "/" << exam->GetCode();

    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::CreateFile( std::string const &suffix )
  {
    // first get the path and create it if it doesn't exist
    std::string path = this->GetFilePath();
    if( !Utilities::fileExists( path ) )
      vtkDirectory::MakeDirectory( path.c_str() );

    return path + "/" + this->Get( "Id" ).ToString() + suffix;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Image::Remove()
  {
    // get the exam and set its downloaded status to false
    vtkSmartPointer< Alder::Exam > exam;
    if( this->GetRecord( exam ) )
    {
      // see if there are any child images and remove them
      vtkNew< Image > child;
      std::map< std::string, std::string > loader;
      loader[ "ExamId" ] = exam->Get( "Id" ).ToString();
      loader[ "ParentImageId" ] = this->Get( "Id" ).ToString();
      if( child->Load( loader ) )
      {
        try
        {
          child->Remove();
        }
        catch( std::runtime_error &e )
        {
          throw( e );
        }
      }

      if( 1 == exam->Get( "Downloaded" ).ToInt() )
      {
        exam->Set( "Downloaded", "0" );
        exam->Save();
      }
    }

    this->Superclass::Remove();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::ValidateFile()
  {
    Application *app = Application::GetInstance();
    bool valid = false;
    std::string fileName;
    try
    {
      fileName = this->GetFileName();
    }
    catch( std::runtime_error& e )
    {
      app->Log( e.what() );
      return valid;
    }

    // now check the file, if it is empty delete the image and the file
    if( 0 == Utilities::getFileLength( fileName ) )
    {
      valid = false;
    }
    else // file exists
    {
      // if the file has a .gz extension, unzip it
      if( ".gz" == fileName.substr( fileName.size() - 3, 3 ) )
      {
        std::string zipFileName = fileName;
        fileName = fileName.substr( 0, fileName.size() - 3 );

        std::string command = "gunzip ";
        command += zipFileName;

        // not a gz file, remove the .gz extension manually
        app->Log( std::string( "Unzipping file: " ) + fileName );
        if( "ERROR" == Utilities::exec( command ) )
          rename( zipFileName.c_str(), fileName.c_str() );
      }

      // now see if we can read the file
      valid = vtkImageDataReader::IsValidFileName( fileName.c_str() );
    }

    // if the file isn't valid, remove it from the disk
    if( !valid ) remove( fileName.c_str() );

    return valid;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetFileName()
  {
    // make sure the path exists
    std::string path = this->GetFilePath();

    // now look for image files in that directory
    vtkNew< vtkDirectory > directory;

    if( !directory->Open( path.c_str() ) )
    {
      std::stringstream error;
      error << "Tried to get image file but the path \"" << path << "\" does not exist.";
      throw std::runtime_error( error.str() );
    }

    // we don't know the file type yet, search for all files which have our Id
    std::string id = this->Get( "Id" ).ToString();
    for( vtkIdType index = 0; index < directory->GetNumberOfFiles(); index++ )
    {
      std::string fileName = directory->GetFile( index );
      if( fileName.substr( 0, id.size() ) == id )
      {
        std::stringstream name;
        name << path << "/" << fileName;
        return name.str();
      }
    }

    // if we get here then the file was not found
    std::stringstream error;
    error << "Tried to get image file in \"" << path << "\" but the file does not exist.";
    throw std::runtime_error( error.str() );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::IsRatedBy( User* user )
  {
    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    std::map< std::string, std::string > loader;
    loader[ "UserId" ] = user->Get( "Id" ).ToString();
    loader[ "ImageId" ] = this->Get( "Id" ).ToString();
    vtkNew< Alder::Rating > rating;
    if( !rating->Load( loader ) ) return false;

    // we have found a rating, make sure it is not null
    return rating->Get( "Rating" ).IsValid();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Image::GetDICOMTag( std::string const &tagName )
  {
    // get the name of the unzipped file
    std::string fileName = this->GetFileName();
    if( ".gz" == fileName.substr( fileName.size() - 3, 3 ) )
      fileName = fileName.substr( 0, fileName.size() - 3 );

    vtkNew<vtkDICOMParser> parser;
    parser->SetFileName( fileName.c_str() );
    parser->Update();

    vtkDICOMMetaData* meta = parser->GetMetaData();
    std::string value;
    if( "AcquisitionDateTime" == tagName )
      value = meta->GetAttributeValue( vtkDICOMTag( 0x0008, 0x002a ) ).AsString();
    else if( "SeriesNumber" == tagName )
      value = meta->GetAttributeValue( vtkDICOMTag( 0x0020, 0x0011 ) ).AsString();
    else if( "PatientName" == tagName )
      value = meta->GetAttributeValue( vtkDICOMTag( 0x0010, 0x0010 ) ).AsString();
    else if( "Laterality" == tagName )
      value = meta->GetAttributeValue( vtkDICOMTag( 0x0020, 0x0060 ) ).AsString();
    else if( "Manufacturer" == tagName )
      value = meta->GetAttributeValue( vtkDICOMTag( 0x0008, 0x0070 ) ).AsString();
    else
      throw std::runtime_error( "Unknown DICOM tag name." );

    return value;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector< int > Image::GetDICOMDimensions()
  {
    // get the name of the unzipped file
    std::string fileName = this->GetFileName();
    if( ".gz" == fileName.substr( fileName.size() - 3, 3 ) )
      fileName = fileName.substr( 0, fileName.size() - 3 );

    std::vector< int > dims;
    gdcm::ImageReader reader;
    reader.SetFileName( fileName.c_str() );
    if( reader.Read() )
    {
      gdcm::Image &image = reader.GetImage();
      for( int i = 0; i < 3; ++i )
        dims.push_back( image.GetDimension( i ) );
    }
    else
    {
      Application *app = Application::GetInstance();
      app->Log( "ERROR: failed read during get dicom dimensions" );
    }
    return dims;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Image::SetDimensionalityFromDICOM()
  {
    std::vector< int > dims = this->GetDICOMDimensions();
    int dimensionality = 0;
    for( auto it = dims.begin(); it != dims.end(); ++it )
    {
      if( *it > 1 ) dimensionality++;
    }
    if( 1 < dimensionality )
    {
      this->Set( "Dimensionality", dimensionality );
      this->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::AnonymizeDICOM()
  {
    if( !this->GetDICOMTag( "PatientName" ).empty() )
    {
      Application *app = Application::GetInstance();
      gdcm::Reader gdcmRead;
      std::string fileName = this->GetFileName();
      gdcmRead.SetFileName( fileName.c_str() );
      if( gdcmRead.Read() )
      {
        gdcm::Anonymizer gdcmAnon;
        gdcmAnon.SetFile( gdcmRead.GetFile() );
        gdcmAnon.Empty( gdcm::Tag( 0x10, 0x10 ) );

        gdcm::Writer gdcmWriter;
        gdcmWriter.SetFile( gdcmAnon.GetFile() );
        gdcmWriter.SetFileName( fileName.c_str() );
        if( gdcmWriter.Write() )
        {
          return true;
        }
        else
         app->Log("ERROR: failed write during anonymize dicom file");
      }
      else
      {
        app->Log("ERROR: failed read during anonymize dicom file");
      }
    }
    return false;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Image::SetExamSideFromDICOM()
  {
    vtkSmartPointer< Exam > exam;
    if( this->GetRecord( exam ) )
    {
      std::string latStr = exam->Get( "Side" ).ToString();
      if( "none" != latStr )
      {
        try{
          std::string tagStr = this->GetDICOMTag( "Laterality" );
          if( 0 < tagStr.size() )
          {
            tagStr = Utilities::toLower( tagStr );
            if( 0 != tagStr.compare(0, 1, latStr, 0, 1) )
            {
              latStr = 0 == tagStr.compare(0, 1, "l", 0, 1) ? "left" : "right";
              exam->Set( "Side", latStr );
              exam->Save();
            }
          }
        }
        catch(...)
        {
        }
      }
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::IsDICOM()
  {
    vtkSmartPointer< Exam > exam;
    return this->GetRecord( exam ) ? exam->IsDICOM() : false;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<Image> Image::GetNeighbourAtlasImage( int const &rating, bool const &forward )
  {
    this->AssertPrimaryId();

    Application *app = Application::GetInstance();
    Image *activeImage = app->GetActiveImage();
    bool hasParent = this->Get( "ParentImageId" ).IsValid();

    // get neighbouring image which matches this image's exam type and the given rating
    std::stringstream stream;
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN ScanType ON Exam.ScanTypeId = Exam.ScanTypeId "
           << "JOIN Interview ON Exam.InterviewId = Interview.Id "
           << "JOIN Rating ON Image.Id = Rating.ImageId "
           << "JOIN User ON Rating.UserId = User.Id "
           << "WHERE Exam.ScanTypeId = ( "
           <<   "SELECT Exam.ScanTypeId "
           <<   "FROM Exam "
           <<   "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
           <<   "JOIN Image ON Exam.Id = Image.ExamId "
           <<   "WHERE Image.Id = " << this->Get( "Id" ).ToString() << " "
           << ") "
           << "AND Image.ParentImageId IS " << ( hasParent ? "NOT" : "" ) << " NULL "
           << "AND Rating = " << rating << " "
           << "AND User.Expert = true ";

    // do not show the active image
    if( NULL != activeImage ) stream << "AND Image.Id != " << activeImage->Get( "Id" ).ToString() << " ";

    // order the query by UId (descending if not forward)
    stream << "ORDER BY Interview.UId ";
    if( !forward ) stream << "DESC ";

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    vtkVariant neighbourId;

    // store the first record in case we need to loop over
    if( query->NextRow() )
    {
      bool found = false;
      vtkVariant currentId = this->Get( "Id" );

      // if the current id is last in the following loop then we need the first id
      neighbourId = query->DataValue( 0 );

      do // keep looping until we find the current Id
      {
        vtkVariant id = query->DataValue( 0 );
        if( found )
        {
          neighbourId = id;
          break;
        }

        if( currentId == id ) found = true;
      }
      while( query->NextRow() );

      // we should always find the current image id
      if( !found ) throw std::runtime_error( "Cannot find current atlas image in database." );
    }

    vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New();
    if( neighbourId.IsValid() ) image->Load( "Id", neighbourId.ToString() );
    return image;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer< Image > Image::GetAtlasImage( int const &rating )
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer< vtkAlderMySQLQuery > query = app->GetDB()->GetQuery();

    vtkSmartPointer< Exam > exam;
    this->GetRecord( exam );
    bool hasParent = this->Get( "ParentImageId" ).IsValid();

    // get any image rated by an expert user having the given exam type and rating score
    std::stringstream stream;
    stream << "SELECT Image.Id "
           << "FROM Image "
           << "JOIN Exam ON Image.ExamId = Exam.Id "
           << "JOIN ScanType ON Exam.ScanTypeId = ScanType.Id "
           << "JOIN Rating ON Image.Id = Rating.ImageId "
           << "JOIN User ON Rating.UserId = User.Id "
           << "WHERE ScanType.Type = " << query->EscapeString( exam->GetScanType() ) << " "
           << "AND Image.ParentImageId IS " << ( hasParent ? "NOT" : "" ) << " NULL "
           << "AND Rating = " << rating << " "
           << "AND User.Expert = true "
           << "AND Image.Id != " << this->Get( "Id" ).ToString() << " "
           << "LIMIT 1";

    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    vtkSmartPointer< Image > image = vtkSmartPointer< Image >::New();
    if( query->NextRow() )
      image->Load( "Id", query->DataValue( 0 ).ToString() );
    return image;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Image::CleanHologicDICOM()
  {
    this->AssertPrimaryId();

    std::string manufacturer = this->GetDICOMTag( "Manufacturer" );
    if( "hologic" != Alder::Utilities::toLower( manufacturer ) )
      return false;

    vtkSmartPointer< Exam > exam = vtkSmartPointer<Exam>::New();
    if( !this->GetRecord( exam ) )
      throw std::runtime_error( "ERROR: no exam record for this image." );

    std::string latStr = exam->Get( "Side" ).ToString();
    std::string typeStr = exam->GetScanType();
    int examType = -1;

    if( "DualHipBoneDensity" == typeStr )
    {
      examType = "left" == latStr ? 0 : 1;
    }
    else if( "ForearmBoneDensity" == typeStr )
    {
      examType = 2;
    }
    else if( "WholeBodyBoneDensity" == typeStr )
    {
      // check if the image has a parent, if so, it is a body composition file
      examType = this->Get( "ParentImageId" ).IsValid() ? 4 : 3;
    }
    else if( "SpineBoneDensity" == typeStr )
    {
      examType = 5;
    }
    else if( "LateralBoneDensity" == typeStr )
    {
      // the lateral spine scans do not have a report to clean:
      // anoymize the PatientName dicom tag
      this->AnonymizeDICOM();
      return true;
    }

    if( -1 == examType ) return false;

    std::string fileName = this->GetFileName();
    vtkNew< vtkImageDataReader > reader;
    reader->SetFileName( fileName.c_str() );
    vtkImageData* image = reader->GetOutput();

    int extent[6];
    image->GetExtent( extent );
    int dims[3];
    image->GetDimensions(dims);

    // start in the middle of the left edge,
    // increment across until the color changes to 255,255,255

    // left edge coordinates for each DEXA exam type
    int x0[6] = { 168, 168, 168, 168, 193, 168 };
    // bottom edge coordinates
    int y0[6] = { 1622, 1622, 1111, 1377, 1434, 1401 };
    // top edge coordinates
    int y1[6] = { 1648, 1648, 1138, 1403, 1456, 1427 };

    bool found = false;
    // start search from the middle of the left edge
    int ix = x0[ examType ];
    int iy = y0[ examType ] + ( y1[ examType ] - y0[ examType ] )/2;
    do
    {
      int val = static_cast<int>( image->GetScalarComponentAsFloat( ix++, iy, 0, 0 ) );
      if( 255 == val )
      {
        found = true;
        ix--;
      }
    }while( !found && ix < extent[1] );

    if( !found )
    {
      return false;
    }

    int nComp = image->GetNumberOfScalarComponents();
    vtkNew< vtkImageCanvasSource2D > canvas;
    // copy the image onto the canvas
    canvas->SetNumberOfScalarComponents( nComp );
    canvas->SetScalarType( image->GetScalarType() );
    canvas->SetExtent( extent );
    canvas->DrawImage( 0, 0, image );
    // erase the name field with its gray background color
    canvas->SetDrawColor( 222, 222, 222 );
    canvas->FillBox( x0[ examType ] , ix, y0[ examType ], y1[ examType ] );
    canvas->Update();

    // flip the canvas vertically
    vtkNew< vtkImageFlip > flip;
    flip->SetInput( canvas->GetOutput() );
    flip->SetFilteredAxis( 1 );
    flip->Update();

    // byte size of the image
    unsigned long length = dims[0]*dims[1]*nComp;

    vtkNew<vtkDICOMReader> dcmReader;
    dcmReader->SetFileName( fileName.c_str() );
    dcmReader->UpdateInformation();

    vtkDICOMMetaData* meta = dcmReader->GetMetaData();
    meta->SetAttributeValue( vtkDICOMTag( 0x0010, 0x0010 ), std::string("") );

    vtkNew<vtkDICOMCompiler> dcmCompiler;
    dcmCompiler->SetFileName( fileName.c_str() );
    dcmCompiler->KeepOriginalPixelDataVROff();
    dcmCompiler->SetMetaData( meta );
    dcmCompiler->WriteHeader();
    dcmCompiler->WritePixelData(
      reinterpret_cast<unsigned char *>( flip->GetOutput()->GetScalarPointer() ), length );
    dcmCompiler->Close();

    return true;
  }
}
