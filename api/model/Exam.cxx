/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Exam.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include "Exam.h"

#include "Application.h"
#include "Image.h"
#include "Interview.h"
#include "OpalService.h"
#include "Utilities.h"

#include "vtkNew.h"
#include "vtkObjectFactory.h"

namespace Alder
{
  vtkStandardNewMacro( Exam );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::HasImageData()
  {
    // An exam has all images if it is marked as downloaded or if the stage is not complete.
    // Alder does not download images from incomplete exams.
    // NOTE: it is possible that an exam with state "Ready" has valid data, but we are leaving
    // those exams out for now since we don't know for sure whether they are always valid
    return 0 != this->Get( "Downloaded" ).ToInt() ||
           "Completed" != this->Get( "Stage" ).ToString();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Exam::UpdateImageData()
  {
    vtkSmartPointer< Interview > interview;

    // start by getting the UId
    this->GetRecord( interview );
    std::string UId = interview->Get( "UId" ).ToString();
    bool result = false;

    if( !this->HasImageData() )
    {
      // determine which Opal table to fetch from based on exam modality
      std::string type = this->Get( "Type" ).ToString();
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

        for( int i = 1; i <= 3; ++i )
        {
          std::string variable = "Measure.CINELOOP_";
          variable += vtkVariant( i ).ToString();
          settings[ "Acquisition" ] = i;

          result = this->RetrieveImage( type, variable, UId, settings,
                                        suffix, sideVariable );
          if( result )
          {
            hasParent = true;
            acquisition = i;
          }
        }

        //TODO: SR files still need to be downloaded and processed

        acquisition++;
        settings[ "Acquisition" ] = acquisition;
        std::string variable = "Measure.STILL_IMAGE";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );

        if( hasParent && result )
        {
          // get the list of cIMT images in this exam

          std::vector< vtkSmartPointer< Alder::Image > > imageList;
          std::vector< vtkSmartPointer< Alder::Image > >::iterator imageIt;
          this->GetList( &imageList );
          if( imageList.empty() ) throw std::runtime_error( "Failed list load during cIMT parenting" );

          // map the AcquisitionDateTimes from the dicom file headers to the images

          std::map< int, std::string > acqDateTimes;
          for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
          {
            Alder::Image *image = imageIt->GetPointer();
            acqDateTimes[ image->Get( "Id" ).ToInt() ] = image->GetDICOMAcquisitionDateTime();
          }

          // find which cineloop has a matching datetime to the still and set the still's
          // ParentImageId

          vtkNew< Alder::Image > still;
          int stillId = still->GetLastInsertId();
          std::string stillAcqDateTime = acqDateTimes[ stillId ];

          // in case of no matching datetime associate the still with
          // the group of cineloops

          std::map< int, std::string >::iterator mapIt;
          int parentId = -1;
          for( mapIt = acqDateTimes.begin(); mapIt != acqDateTimes.end(); mapIt++ )
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
      else if( "DualHipBoneDensity" == type )
      {
        std::string variable = "Measure.RES_HIP_DICOM";
        std::string sideVariable = "Measure.OUTPUT_HIP_SIDE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( "ForearmBoneDensity" == type )
      {
        std::string variable = "RES_FA_DICOM";
        std::string sideVariable = "OUTPUT_FA_SIDE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( "LateralBoneDensity" == type )
      {
        std::string variable = "RES_SEL_DICOM_MEASURE";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings, suffix );
      }
      else if( "Plaque" == type )
      {
        std::string variable = "Measure.CINELOOP_1";
        std::string sideVariable = "Measure.SIDE";
        std::string suffix = ".dcm.gz";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( "RetinalScan" == type )
      {
        std::string variable = "Measure.EYE";
        std::string sideVariable = "Measure.SIDE";
        std::string suffix = ".jpg";
        result = this->RetrieveImage( type, variable, UId, settings,
                                      suffix, sideVariable );
      }
      else if( "WholeBodyBoneDensity" == type )
      {
        std::string variable = "RES_WB_DICOM_1";
        std::string suffix = ".dcm";
        result = this->RetrieveImage( type, variable, UId, settings, suffix );

        if( result )
        {
          variable = "RES_WB_DICOM_2";
          settings[ "Acquisition" ] = 2;
          result = this->RetrieveImage( type, variable, UId, settings, suffix );
        }
      }

      // now set that we have downloaded all the images
      this->Set( "Downloaded", 1 );
      this->Save();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::RetrieveImage( std::string type, std::string variable, std::string UId,
                            std::map<std::string, vtkVariant> settings,
                            std::string suffix, std::string sideVariable )
  {
    Application *app = Application::GetInstance();
    OpalService *opal = app->GetOpal();
    bool repeatable = sideVariable.length() > 0;
    bool result = true;
    int sideIndex = 0;

    if( repeatable )
    {
      std::vector< std::string > sideList;
      std::vector< std::string >::iterator sideListIt;
      sideList = opal->GetValues( "clsa-dcs-images", type, UId, sideVariable );
     
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
      std::string laterality = this->Get( "Laterality" ).ToString();

      for( sideListIt = sideList.begin(); sideListIt != sideList.end(); ++sideListIt )
      {
        if( laterality == Utilities::toLower( *sideListIt ) )
        {
          found = true;
          break;
        }
        sideIndex++;
      }

      if( !found ) return false;
    }

    std::stringstream log;
    log << "Adding " << variable << " to database for UId \"" << UId << "\"";
    Utilities::log( log.str() );

    // add a new entry in the image table (or replace it)
    vtkNew< Alder::Image > image;
    std::map< std::string, vtkVariant >::iterator it = settings.begin();
    for( it = settings.begin(); it != settings.end(); it++ ) image->Set( it->first, it->second );
    image->Save( true );

    // now write the file and validate it
    std::string fileName = image->CreateFile( suffix );
    opal->SaveFile( fileName, "clsa-dcs-images", type, UId, variable, repeatable ? sideIndex : -1 );

    if( !image->ValidateFile() )
    {
      log.str( "" );
      log << "Removing " << variable << " from database (invalid)";
      Utilities::log( log.str() );
      image->Remove();
      result = false;
    }
    return result;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Exam::IsRatedBy( User* user )
  {
    this->AssertPrimaryId();

    // make sure the user is not null
    if( !user ) throw std::runtime_error( "Tried to get rating for null user" );

    // loop through all images
    std::vector< vtkSmartPointer< Image > > imageList;
    std::vector< vtkSmartPointer< Image > >::iterator imageIt;
    this->GetList( &imageList );
    for( imageIt = imageList.begin(); imageIt != imageList.end(); ++imageIt )
    {
      Image *image = *(imageIt);
      if( !image->IsRatedBy( user ) ) return false;
    }

    // only return true if there was at least one image rated
    return 0 < imageList.size();
  }
}
