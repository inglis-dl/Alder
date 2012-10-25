/*=========================================================================

  Program:  Alder (CLSA Ultrasound Image Viewer)
  Module:   Study.h
  Language: C++

  Author: Patrick Emond <emondpd@mcmaster.ca>
  Author: Dean Inglis <inglisd@mcmaster.ca>

=========================================================================*/

#ifndef __Study_h
#define __Study_h

#include "ActiveRecord.h"

#include <iostream>
#include <vector>

namespace Alder
{
//  class Session;
  class Study : public ActiveRecord
  {
  public:
    static Study *New();
    vtkTypeMacro( Study, ActiveRecord );
    static std::vector< std::string > GetIdentifierList();

  protected:
    Study() {}
    ~Study() {}

    std::string GetName() { return "Study"; }

  private:
    Study( const Study& ); // Not implemented
    void operator=( const Study& ); // Not implemented
  };
}

#endif