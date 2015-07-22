/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Exam.h>

#include <Application.h>
#include <CodeType.h>
#include <Image.h>
#include <Interview.h>
#include <Modality.h>
#include <OpalService.h>
#include <ScanType.h>
#include <Utilities.h>

#include <vtkNew.h>
#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( Exam );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetCode()
  {
    this->AssertPrimaryId();

    vtkSmartPointer< Interview > interview;
    if( !this->GetRecord( interview ) )
      throw std::runtime_error( "Exam has no parent interview!" );

    std::stringstream stream;
    stream << interview->Get( "Id" ).ToString() << "/" << this->Get( "Id" ).ToString();
    return stream.str();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::HasImageData()
  {
    // An exam has all images if it is marked as downloaded and has a Completed status.
    // Alder does not download images from incomplete or contra-indicated exams.
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
    bool downLoaded = this->Get( "Downloaded" ).ToInt() == 1;
    std::string stageStatus = this->Get( "Stage" ).ToString();
    return ( ( downLoaded && stageStatus == "Completed" ) ||
             ( !downLoaded && stageStatus == "NotApplicable" ) );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetScanType()
  {
    vtkSmartPointer< ScanType > scanType;
    std::string type;

    if( this->GetRecord( scanType ) )
      type = scanType->Get( "Type" ).ToString();

    return type;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Exam::GetModalityName()
  {
    std::string modalityName;
    vtkSmartPointer< ScanType > scanType;
    if( this->GetRecord( scanType ) )
    {
      vtkSmartPointer< Modality > modality;
      if( scanType->GetRecord( modality ) )
        modalityName = modality->Get( "Name" ).ToString();
    }

    return modalityName;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::UpdateImageData()
  {
    if( this->HasImageData() ) return;
    vtkSmartPointer< Interview > interview;

    // start by getting the UId
    this->GetRecord( interview );
    std::string UId = interview->Get( "UId" ).ToString();

    // status of download: all files of the exam are downloaded
    bool resultAll = true;
    bool resultAny = false;
    // whether to clean up dicom images
    bool clean = true;

    // determine which Opal table to fetch from based on exam modality
    std::string type = this->GetScanType();
    std::map< std::string, vtkVariant > settings;
    settings[ "ExamId" ] = this->Get( "Id" );
    settings[ "Acquisition" ] = 1;

    if( "CarotidIntima" == type )
    {
      // write cineloops 1, 2 and 3
      // for now, assume that the parent image id for the still image
      // associated with one of the 3 possible cineloops is the first valid one
      bool hasParent = false;
      std::string suffix = ".dcm.gz";
      std::string sideVariable = "Measure.SIDE";
      int acquisition = 0;
      bool repeatable = true;

      for( int i = 1; i <= 3; ++i )
      {
        std::string variable = "Measure.CINELOOP_";
        variable += vtkVariant( i ).ToString();
        settings[ "Acquisition" ] = i;

        bool success = this->RetrieveImage(
          type, variable, UId, settings, suffix, repeatable, sideVariable );
        resultAll &= success;

        if( success )
        {
          resultAny = success;
          hasParent = true;
          acquisition = i;
        }
      }

      if( hasParent )
      {
        //TODO: SR files still need to be downloaded and processed

        settings[ "Acquisition" ] = ++acquisition;
        std::string variable = "Measure.STILL_IMAGE";
        bool success = this->RetrieveImage(
          type, variable, UId, settings, suffix, repeatable, sideVariable );
        resultAll &= success;

        if( success )
        {
          // get the list of cIMT images in this exam

          std::vector< vtkSmartPointer< Alder::Image > > imageList;
          this->GetList( &imageList );
          if( imageList.empty() )
            throw std::runtime_error( "Failed list load during cIMT parenting" );

          // map the AcquisitionDateTimes from the dicom file headers to the images

          std::map< int, std::string > acqDateTimes;
          for( auto imageIt = imageList.cbegin(); imageIt != imageList.cend(); ++imageIt )
          {
            Alder::Image *image = imageIt->GetPointer();
            acqDateTimes[ image->Get( "Id" ).ToInt() ] = image->GetDICOMTag( "AcquisitionDateTime" );
          }

          // find which cineloop has a matching datetime to the still and set the still's
          // ParentImageId

          vtkNew< Alder::Image > still;
          int stillId = still->GetLastInsertId();
          std::string stillAcqDateTime = acqDateTimes[ stillId ];

          // in case of no matching datetime, associate the still with
          // the group of cineloops

          int parentId = -1;
          for( auto mapIt = acqDateTimes.cbegin(); mapIt != acqDateTimes.cend(); ++mapIt )
          {
            if( mapIt->first == stillId ) continue;

            if( mapIt->second == stillAcqDateTime )
            {
              parentId = mapIt->first;
              break;
            }
            else
            {
              // use the last inserted cineloop Id in case of no match
              parentId = mapIt->first > parentId ? mapIt->first : parentId;
            }
          }

          if( parentId == -1 )
            throw std::runtime_error( "Failed to parent cIMT still" );

          still->Load( "Id", vtkVariant( stillId ).ToString() );
          still->Set( "ParentImageId", parentId );
          still->Save();
        }
      }
    }
    else if( "DualHipBoneDensity" == type )
    {
      std::string variable = "Measure.RES_HIP_DICOM";
      std::string sideVariable = "Measure.OUTPUT_HIP_SIDE";
      std::string suffix = ".dcm";
      bool repeatable = true;
      resultAll &= this->RetrieveImage( type, variable, UId, settings, suffix, repeatable,
        sideVariable );
      resultAny = resultAll;
    }
    else if( "ForearmBoneDensity" == type )
    {
      std::string variable = "RES_FA_DICOM";
      std::string sideVariable = "OUTPUT_FA_SIDE";
      std::string suffix = ".dcm";
      bool repeatable = false;
      resultAll &= this->RetrieveImage( type, variable, UId, settings, suffix, repeatable,
        sideVariable );
      resultAny = resultAll;
    }
    else if( "LateralBoneDensity" == type )
    {
      std::string variable = "RES_SEL_DICOM_MEASURE";
      std::string suffix = ".dcm";
      resultAll &= this->RetrieveImage( type, variable, UId, settings, suffix );
      resultAny = resultAll;
    }
    else if( "WholeBodyBoneDensity" == type )
    {
      std::string variable = "RES_WB_DICOM_1";
      std::string suffix = ".dcm";
      bool success = this->RetrieveImage( type, variable, UId, settings, suffix );
      resultAll &= success;

      if( success )
      {
        resultAny = true;
        // store the previous image's id
        vtkNew< Alder::Image > image;
        int parentId = image->GetLastInsertId();

        variable = "RES_WB_DICOM_2";
        settings[ "Acquisition" ] = 2;
        success = this->RetrieveImage( type, variable, UId, settings, suffix );
        resultAll &= success;

        // re-parent this image to the first one
        int lastId = image->GetLastInsertId();
        if( success && parentId != lastId )
        {
          image->Load( "Id", vtkVariant( lastId ).ToString() );
          image->Set( "ParentImageId", parentId );
          image->Save();
        }
      }
    }
    else if( "RetinalScan" == type )
    {
      std::string variable = "Measure.EYE";
      std::string sideVariable = "Measure.SIDE";
      std::string suffix = ".jpg";
      bool repeatable = true;
      resultAll &= this->RetrieveImage( type, variable, UId, settings, suffix, repeatable,
        sideVariable );
      resultAny = resultAll;
      clean = false;
    }
    else
    {
      std::string errStr = "Cannot retrieve images for unknown exam type: " + type;
      throw std::runtime_error( errStr );
    }

    // now set that we have downloaded at least one of the images
    if( resultAny )
    {
      this->Set( "Downloaded", 1 );
      this->Save();
      if( clean )
        this->CleanImages( type );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::RetrieveImage(
    const std::string type,
    const std::string variable,
    const std::string UId,
    const std::map<std::string, vtkVariant> settings,
    const std::string suffix,
    const bool repeatable,
    const std::string sideVariable )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool result = true;
    int sideIndex = 0;
    std::stringstream log;
    std::string laterality = this->Get( "Laterality" ).ToString();

    if( "none" != laterality )
    {
      std::vector< std::string > sideList;
      if( repeatable )
      {
        sideList = opal->GetValues( "clsa-dcs-images", type, UId, sideVariable );
      }
      else
      {
        std::string side = opal->GetValue( "clsa-dcs-images", type, UId, sideVariable );
        if( !side.empty() ) sideList.push_back( side );
      }

      int numSides = sideList.empty() ? 0 : sideList.size();

      if( numSides > 1 )
      {
        // sort into right, left
        std::sort( sideList.begin(), sideList.end(), std::greater< std::string >());

        // enforce unique strings
        sideList.erase( std::unique( sideList.begin(), sideList.end() ), sideList.end() );

        // remove empty strings
        sideList.erase(
          std::remove_if( sideList.begin(), sideList.end(), mem_fun_ref(&std::string::empty) ),
          sideList.end() );

        // if reduced to one side only, add the opposing side
        if( sideList.size() == 1 )
        {
          if( "right" == Utilities::toLower( sideList[0] ) )
          {
            sideList.push_back( "left" );
          }
          else if( "left" == Utilities::toLower( sideList[0] ) )
          {
            sideList.insert( sideList.begin(), "right" );
          }
        }
      }

      bool found = false;

      for( auto sideListIt = sideList.cbegin(); sideListIt != sideList.cend(); ++sideListIt )
      {
        if( Utilities::toLower( *sideListIt ) == laterality )
        {
          found = true;
          break;
        }
        sideIndex++;
      }

      if( !found ) return false;
    }

    log << "Adding " << variable << " to database for UId \"" << UId << "\"";
    app->Log( log.str() );

    // add a new entry in the image table (or replace it)
    vtkNew< Alder::Image > image;
    for( auto it = settings.cbegin(); it != settings.cend(); ++it ) image->Set( it->first, it->second );

    try{
      image->Save( true );
    }
    catch( std::runtime_error& e )
    {
      app->Log( e.what() );
    }

    // now write the file and validate it
    std::string fileName = image->CreateFile( suffix );
    opal->SaveFile( fileName, "clsa-dcs-images", type, UId, variable, repeatable ? sideIndex : -1 );

    if( !image->ValidateFile() )
    {
      log.str( "" );
      log << "Removing " << variable << " from database (invalid)";
      app->Log( log.str() );
      image->Remove();
      result = false;
    }
    else
    {
      int dimensionality = 2;
      if( this->IsDICOM() )
      {
        // set the image record's Dimensionality column
        std::vector<int> dims = image->GetDICOMDimensions();
        // count the number of dimensions > 1
        dimensionality = 0;
        for( auto it = dims.begin(); it != dims.end(); ++it ) if( *it > 1 ) dimensionality++;
      }
      image->Set( "Dimensionality", dimensionality );
      try {
        image->Save();
      }
      catch( std::runtime_error& e )
      {
        app->Log( e.what() );
      }
    }

    return result;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::CleanImages( std::string const &type )
  {
    this->AssertPrimaryId();

    if( !this->IsDICOM() ) return;

    bool isHologic = "CarotidIntima" != type;
    std::vector< vtkSmartPointer< Image > > imageList;
    this->GetList( &imageList );

    for( auto it = imageList.begin(); it != imageList.end(); ++it )
    {
      Image* image = *it;
      if( isHologic )
      {
        // TODO: disable until a workaround can be found for unique index issue
        // image->SetLateralityFromDICOM();
        image->CleanHologicDICOM();
      }
      else image->AnonymizeDICOM();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsDICOM()
  {
    std::string modalityName = this->GetModalityName();
    return ( "Dexa" == modalityName || "Ultrasound" == modalityName );
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    // loop through all images
    std::vector< vtkSmartPointer< Image > > imageList;
    this->GetList( &imageList );
    for( auto imageIt = imageList.cbegin(); imageIt != imageList.cend(); ++imageIt )
    {
      Image *image = *(imageIt);
      if( !image->IsRatedBy( user ) ) return false;
    }

    // only return true if there was at least one image rated
    return 0 < imageList.size();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<int,std::string> Exam::GetCodeTypeData()
  {
    std::map<int,std::string> data;
    vtkSmartPointer< ScanType > scanType;
    this->GetRecord( scanType );
    std::vector< vtkSmartPointer< Alder::CodeType > > codeTypeList;
    scanType->GetList( &codeTypeList );
    for( auto it = codeTypeList.cbegin(); it != codeTypeList.cend(); ++it )
    {
      vtkVariant code = (*it)->Get("Code");
      vtkVariant id = (*it)->Get("Id");
      if( code.IsValid() && id.IsValid() )
        data[id.ToInt()] = code.ToString();
    }
    return data;
  }
}
