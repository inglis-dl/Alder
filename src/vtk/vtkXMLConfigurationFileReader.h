/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   vtkXMLConfigurationFileReader.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
//
// .NAME vtkXMLConfigurationFileReader - Reads XML configuration files.
//
// .SECTION Description
// This is a source object that reads configuration files in XML format.
// There is no output, instead use the GetX() methods to get the read information.
//
// .SECTION See Also
// Database vtkXMLFileReader
// 

#ifndef __vtkXMLConfigurationFileReader_h
#define __vtkXMLConfigurationFileReader_h

// Alder includes
#include <vtkXMLFileReader.h>
#include <Utilities.h>

class vtkXMLConfigurationFileReader : public vtkXMLFileReader
{
public:
  static vtkXMLConfigurationFileReader *New();
  vtkTypeMacro( vtkXMLConfigurationFileReader, vtkXMLFileReader );
  
  std::map< std::string, std::map< std::string, std::string > > GetSettings() { return this->Settings; }

protected:
  vtkXMLConfigurationFileReader()
  {
    this->SetNumberOfOutputPorts( 0 );
  }
  ~vtkXMLConfigurationFileReader() {}

  virtual int ProcessRequest( vtkInformation *, vtkInformationVector **, vtkInformationVector * );
  virtual int FillOutputPortInformation( int, vtkInformation* ) { return 1; }
  virtual int RequestDataObject( vtkInformation *, vtkInformationVector **, vtkInformationVector * )
  { return 1; }

  std::map< std::string, std::map< std::string, std::string > > Settings;

private:
  vtkXMLConfigurationFileReader( const vtkXMLConfigurationFileReader& );  // Not implemented.
  void operator=( const vtkXMLConfigurationFileReader& );  // Not implemented.
};

#endif
