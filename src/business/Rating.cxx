/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Rating.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Rating.h>

#include <Code.h>
#include <CodeType.h>
#include <CodeGroup.h>
#include <Utilities.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( Rating );

  const int Rating::MaximumRating = 5;
  const int Rating::MinimumRating = 1;

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Rating::UpdateDerivedRating( bool setRatingFromDerived )
  {
    this->AssertPrimaryId();

    vtkVariant userId = this->Get( "UserId" );
    vtkVariant imageId = this->Get( "ImageId" );
    if( userId.IsValid() && imageId.IsValid() );
    {
      int derivedRating = Rating::MaximumRating;
      //loop over all selected codes

      // get the ungrouped codes
      std::vector< vtkSmartPointer< Alder::Code > > codeList;
      vtkSmartPointer< Alder::QueryModifier > modifier = vtkSmartPointer< Alder::QueryModifier >::New();
      modifier->Where( "UserId", "=" , userId.ToInt() );
      modifier->Where( "ImageId", "=" , imageId.ToInt() );
      modifier->Join( "CodeType", "CodeType.Id", "Code.CodeTypeId" );
      modifier->Where( "CodeType.CodeGroupId", "=", vtkVariant() );

      Alder::Code::GetAll( &codeList, modifier );
      for( auto it = codeList.begin(); it != codeList.end(); ++it )
      {
        vtkSmartPointer< Alder::CodeType > type;
        (*it)->GetRecord( type );
        derivedRating += type->Get("Value").ToInt();
      }

      codeList.clear();

      // get the grouped codes
      modifier->Reset();
      modifier->Where( "UserId", "=" , userId.ToInt() );
      modifier->Where( "ImageId", "=" , imageId.ToInt() );
      modifier->Join( "CodeType", "CodeType.Id", "Code.CodeTypeId" );
      modifier->Where( "CodeType.CodeGroupId", "!=", vtkVariant() );
      modifier->Group( "CodeType.CodeGroupId" );

      Alder::Code::GetAll( &codeList, modifier );
      for( auto it = codeList.begin(); it != codeList.end(); ++it )
      {
        vtkSmartPointer< Alder::CodeType > type;
        (*it)->GetRecord( type );
        vtkSmartPointer< Alder::CodeGroup > group;
        if( type->GetRecord( group ) )
        {
          derivedRating += group->Get("Value").ToInt();
        }
      }

      if( Rating::MinimumRating > derivedRating ) derivedRating = Rating::MinimumRating;
      if( Rating::MaximumRating < derivedRating ) derivedRating = Rating::MaximumRating;

      this->Set( "DerivedRating", derivedRating );
      if( setRatingFromDerived )
        this->Set( "Rating", derivedRating );
      this->Save();
    }
  }
}
