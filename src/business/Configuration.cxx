/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Configuration.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Configuration.h>

// VTK includes
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkXMLConfigurationFileReader.h>

// C++ includes
#include <string>

namespace Alder
{
  vtkStandardNewMacro(Configuration);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Configuration::Configuration()
  {
    this->Reader = vtkSmartPointer<vtkXMLConfigurationFileReader>::New();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Configuration::Read(const std::string fileName)
  {
    this->Reader->SetFileName(fileName);
    try
    {
      this->Reader->Update();
      this->Settings = this->Reader->GetSettings();
    }
    catch(std::exception &e)
    {
      return false;
    }

    return true;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Configuration::GetValue(const std::string category,
    const std::string key) const
  {
    auto categoryIt = this->Settings.find(category);

    return categoryIt != this->Settings.cend() &&
           categoryIt->second.find(key) != categoryIt->second.cend() ?
           categoryIt->second.at(key) : "";
  }
}  // namespace Alder
