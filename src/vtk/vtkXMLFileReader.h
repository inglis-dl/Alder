/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   vtkXMLFileReader.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class vtkXMLFileReader
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Generic XML file reader.
 *
 * This abstract class must be extended by any class which reads XML files.
 * The parent of this class, vtkAlgorithm, provides many methods for
 * controlling the reading of the data file, see vtkAlgorithm for more
 * information.
 *
 * @see vtkAlgorithm
 */

#ifndef __vtkXMLFileReader_h
#define __vtkXMLFileReader_h

// VTK includes
#include <vtkAlgorithm.h>
#include <vtkVariant.h>

// libxml includes
// NOTE: to avoid name mangling in vtk 6, build vtk with system libxml
#include <libxml/xmlreader.h>

// C++ includes
#include <stdexcept>
#include <sstream>
#include <string>

class vtkXMLFileReader : public vtkAlgorithm
{
  public:
    vtkTypeMacro(vtkXMLFileReader, vtkAlgorithm);
    void PrintSelf(ostream &os, vtkIndent indent);

    /**
     * Set/Get the file name
     */
    virtual std::string GetFileName() { return this->FileName; }
    virtual void SetFileName(const std::string &fileName);

  protected:
    /**
     * Internal struct for managing nodes
     */
    struct vtkXMLFileReaderNode
    {
      vtkXMLFileReaderNode() { this->Clear(); }
      void Clear()
      {
        this->Name = NULL;
        this->Depth = 0;
        this->AttributeCount = 0;
        this->NodeType = 0;
        this->IsEmptyElement = 0;
        this->HasContent = 0;
        this->Content = NULL;
      }
      const char* GetNodeTypeName()
      {
        if (XML_READER_TYPE_NONE == this->NodeType)
          return "None";
        else if (XML_READER_TYPE_ELEMENT == this->NodeType)
          return "Element";
        else if (XML_READER_TYPE_ATTRIBUTE == this->NodeType)
          return "Attribute";
        else if (XML_READER_TYPE_TEXT == this->NodeType)
          return "Text";
        else if (XML_READER_TYPE_CDATA == this->NodeType)
          return "CDATA";
        else if (XML_READER_TYPE_ENTITY_REFERENCE == this->NodeType)
          return "EntityReference";
        else if (XML_READER_TYPE_ENTITY == this->NodeType)
          return "Entity";
        else if (XML_READER_TYPE_PROCESSING_INSTRUCTION == this->NodeType)
          return "ProcessingInstruction";
        else if (XML_READER_TYPE_COMMENT == this->NodeType)
          return "Comment";
        else if (XML_READER_TYPE_DOCUMENT == this->NodeType)
          return "Document";
        else if (XML_READER_TYPE_DOCUMENT_TYPE == this->NodeType)
          return "DocumentType";
        else if (XML_READER_TYPE_DOCUMENT_FRAGMENT == this->NodeType)
          return "DocumentFragment";
        else if (XML_READER_TYPE_NOTATION == this->NodeType)
          return "Notation";
        else if (XML_READER_TYPE_WHITESPACE == this->NodeType)
          return "Whitespace";
        else if (XML_READER_TYPE_SIGNIFICANT_WHITESPACE == this->NodeType)
          return "SignificantWhitespace";
        else if (XML_READER_TYPE_END_ELEMENT == this->NodeType)
          return "EndElement";
        else if (XML_READER_TYPE_END_ENTITY == this->NodeType)
          return "EndEntity";
        else if (XML_READER_TYPE_XML_DECLARATION == this->NodeType)
          return "XmlDeclaration";
        return "Unknown";
      }
      void PrintSelf(ostream &os, vtkIndent indent)
      {
        os << indent << "Name: "
           << (NULL == this->Name ? "(null)" :
               reinterpret_cast<char*>(
               const_cast<unsigned char*>(this->Name))) << endl;
        os << indent << "NodeType: "
           << this->GetNodeTypeName() << " (" << this->NodeType << ")" << endl;
        os << indent << "Depth: "
           << this->Depth << endl;
        os << indent << "AttributeCount: "
           << this->AttributeCount << endl;
        os << indent << "IsEmptyElement: "
           << this->IsEmptyElement << endl;
        os << indent << "HasContent: "
           << this->HasContent << endl;
        os << indent << "Content: \""
           << (NULL == this->Content ? "(null)" :
               reinterpret_cast<char*>(
               const_cast<unsigned char*>(this->Content)))
           << "\"" << endl;
      }

      const xmlChar* Name;
      int Depth;
      int AttributeCount;
      int NodeType;
      int IsEmptyElement;
      int HasContent;
      const xmlChar* Content;
    };

    vtkXMLFileReader();
    ~vtkXMLFileReader();

    virtual int ProcessRequest(
      vtkInformation *requst,
      vtkInformationVector **inputVector,
      vtkInformationVector *outputVector);

    /**
     * Opens and parses the current XML file.
     * An exception is thrown if the file cannot be opened.
     */
    void CreateReader();

    /**
     * Closes the current file.
     */
    void FreeReader();

    /**
     * Parses the next node in the XML file.  Make sure to use Open() before
     * calling this method.  Returns 1 if a new node has been parsed or 0 if the
     * end of the file has been reached.
     * An exception is thrown if there is a parsing error.
     */
    int ParseNode();

    /**
     * Points the current node to the beginning of the file so that the first
     * node in the file will be parsed next time ParseNode() is called.
     */
    void RewindReader();

    void ReadValue(std::string &value);

    std::string FileName;
    xmlTextReader *Reader;
    vtkXMLFileReaderNode CurrentNode;

  private:
    vtkXMLFileReader(const vtkXMLFileReader&);  /** Not implemented. */
    void operator=(const vtkXMLFileReader&);  /** Not implemented. */
};

#endif
