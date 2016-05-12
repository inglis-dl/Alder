/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   vtkXMLConfigurationFileReader.cpp
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <vtkXMLConfigurationFileReader.h>

// VTK includes
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkObjectFactory.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkVariantArray.h>

// C++ includes
#include <algorithm>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

vtkStandardNewMacro(vtkXMLConfigurationFileReader);

// this undef is required on the hp. vtkMutexLock ends up including
// /usr/inclue/dce/cma_ux.h which has the gall to #define read as cma_read
#ifdef read
#undef read
#endif

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int vtkXMLConfigurationFileReader::ProcessRequest(
  vtkInformation *request,
  vtkInformationVector **inputVector,
  vtkInformationVector *outputVector)
{
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA_OBJECT()))
  {
    return this->RequestDataObject(request, inputVector, outputVector);
  }
  else if (request->Has(
           vtkStreamingDemandDrivenPipeline::REQUEST_UPDATE_EXTENT()))
  {
    return 1;
  }
  else if (request->Has(
           vtkDemandDrivenPipeline::REQUEST_INFORMATION()))
  {
    return 1;
  }
  if (request->Has(vtkDemandDrivenPipeline::REQUEST_DATA()))
  {
    vtkDebugMacro(<< "Reading configuration file ...");

    try
    {
      std::runtime_error e("Error reading configuration file.");
      this->CreateReader();

      // parse until we find the configuration element
      while (this->ParseNode())
      {
        if (XML_READER_TYPE_ELEMENT == this->CurrentNode.NodeType &&
            0 == xmlStrcasecmp(BAD_CAST "Configuration", this->CurrentNode.Name)) break;
      }

      // if we never found the configuration element then throw an exception
      if (XML_READER_TYPE_ELEMENT != this->CurrentNode.NodeType ||
          0 != xmlStrcasecmp(BAD_CAST "Configuration", this->CurrentNode.Name))
      {
        throw std::runtime_error(
          "File does not contain a configuration element.");
      }

      std::string str;
      int depth = this->CurrentNode.Depth;
      while (this->ParseNode())
      {
        // loop until we find the closing element at the same depth
        if (XML_READER_TYPE_END_ELEMENT == this->CurrentNode.NodeType &&
            depth == this->CurrentNode.Depth) break;

        if (XML_READER_TYPE_ELEMENT == this->CurrentNode.NodeType &&
            0 < xmlStrlen(this->CurrentNode.Name))
        { // new configuration category
          std::map<std::string, std::string> map;

          // now find all key-value pairs in this category
          while (this->ParseNode())
          {
            // loop until we find the closing element at the same depth
            if (XML_READER_TYPE_END_ELEMENT == this->CurrentNode.NodeType &&
                depth + 1 == this->CurrentNode.Depth) break;

            if (XML_READER_TYPE_ELEMENT == this->CurrentNode.NodeType)
            {
              this->ReadValue(str);
              map.insert(
                std::pair<std::string, std::string>(
                  reinterpret_cast<char*>(
                    const_cast<unsigned char*>(this->CurrentNode.Name)), str));
            }
          }

          this->Settings.insert(
            std::pair< std::string, std::map<std::string, std::string> >(
              reinterpret_cast<char*>(
                const_cast<unsigned char*>(this->CurrentNode.Name)), map));
        }
      }

      this->FreeReader();
    }
    catch(std::exception &e)
    {
      this->FreeReader();
      throw e;
    }
  }

  return this->Superclass::ProcessRequest(request, inputVector, outputVector);
}
