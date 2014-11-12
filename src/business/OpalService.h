/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   OpalService.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class OpalService
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Class for interacting with Opal
 *
 * This class provides a programming interface to Opal's RESTful interface by using the
 * curl library.  A description of Opal can be found
 * <a href="http://www.obiba.org/?q=node/63">here</a>.
 */

#ifndef __OpalService_h
#define __OpalService_h

#include <ModelObject.h>

#include <vtkSmartPointer.h>
#include <vtkAlderMySQLQuery.h>

#include <iostream>
#include <json/reader.h>
#include <map>
#include <vector>

class vtkAlderMySQLOpalService;

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class OpalService : public ModelObject
  {
  public:
    static OpalService *New();
    vtkTypeMacro( OpalService, ModelObject );

    /**
     * Defines connection parameters to use when communicating with the Opal server
     */
    void Setup(
      const std::string& username,
      const std::string& password,
      const std::string& host,
      const int& port = 8843,
      const int& timeout = 10,
      const int& verbose = 0 );

    vtkGetMacro( Port, int );
    vtkSetMacro( Port, int );

    vtkGetMacro( Timeout, int );
    vtkSetMacro( Timeout, int );

    /**
     * Call before invoking the application StartEvent for progress monitoring.
     * If check is true, then the first curl progress callback will set
     * whether the fine level (local) progress should be a regular progress
     * meter or a busy meter, based on whether the expected size of the data
     * to be downloaded is non-zero.  For file type data, this should be called
     * with false, since we expect (image) files to have significant size.
     */
    static void SetCurlProgressChecking( const bool check = true )
    {
      OpalService::configureEventSent = false;
      OpalService::curlProgressChecking = check;
    };

    /**
     * Returns a list of all identifiers in a particular data source and table
     * @param dataSource string
     * @param table string
     */
    std::vector< std::string > GetIdentifiers( const std::string dataSource, const std::string table ) const;

    /**
     * Returns all variables for all identifiers limited by the offset and limit parameters
     * @param dataSource string
     * @param table string
     * @param offset int The offset to begin the list at.
     * @param limit int The limit of how many key/value pairs to return
     */
    std::map< std::string, std::map< std::string, std::string > > GetRows(
      const std::string dataSource, const std::string table,
      const int offset = 0, const int limit = 100 ) const;

    /**
     * Returns all variables for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     */
    std::map< std::string, std::string > GetRow(
      const std::string dataSource, const std::string table, const std::string identifier ) const;

    /**
     * Returns all values for a particular variable limited by the offset and limit parameters
     * @param dataSource string
     * @param table string
     * @param variable string
     * @param offset int The offset to begin the list at.
     * @param limit int The limit of how many key/value pairs to return
     */
    std::map< std::string, std::string > GetColumn(
      const std::string dataSource, const std::string table, const std::string variable,
      const int offset = 0, const int limit = 100 );

    /**
     * Returns a particular variable for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     * @param variable string
     */
    std::string GetValue(
      const std::string dataSource, const std::string table,
      const std::string identifier, const std::string variable ) const;

    /**
     * Returns an array variable for a given identifier
     * @param dataSource string
     * @param table string
     * @param identifier string
     * @param variable string
     */
    std::vector< std::string > GetValues(
      const std::string dataSource, const std::string table,
      const std::string identifier, const std::string variable ) const;

    /**
     * Returns a particular variable for a given identifier
     * @param dataSource string
     * @param fileName string
     * @param table string
     * @param identifier string
     * @param variable string
     * @param integer position
     */
    void SaveFile(
      const std::string fileName,
      const std::string dataSource,
      const std::string table,
      const std::string identifier,
      const std::string variable,
      const int position = -1 ) const;

  protected:
    OpalService();
    ~OpalService();

    /**
     * Returns the response provided by Opal for a given service path, or if fileName is not
     * empty then writes the response to the given filename (returning an empty json value).
     * @param servicePath string
     * @param fileName string
     * @param doProgress bool
     * @throws runtime_error
     */
    virtual Json::Value Read(
      const std::string servicePath, const std::string fileName = "", const bool progress = true ) const;

    std::map< std::string, std::map< std::string, std::map< std::string, std::string > > > Columns;
    std::string Username;
    std::string Password;
    std::string Host;
    int Port;
    int Timeout;
    int Verbose;

  private:
    OpalService( const OpalService& ); /** Not implemented. */
    void operator=( const OpalService& ); /** Not implemented. */

    static int curlProgressCallback( const void* const , const double, const double, const double, const double );
    static bool configureEventSent;
    static bool curlProgressChecking;
  };
}

/** @} end of doxygen group */

#endif
