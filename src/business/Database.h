/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Database.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Database
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Class for interacting with the database
 *
 * This class provides methods to interact with the database.  It includes
 * metadata such as information about every column in every table.  A single
 * instance of this class is created and managed by the Application singleton
 * and it is primarily used by active records.
 */

#ifndef __Database_h
#define __Database_h

#include <ModelObject.h>

#include <vtkAlderMySQLQuery.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

#include <iostream>
#include <map>
#include <vector>

class vtkAlderMySQLDatabase;

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User;
  class Database : public ModelObject
  {
  public:
    static Database *New();
    vtkTypeMacro( Database, ModelObject );

    /**
     * Connects to a database given connection parameters
     * @param name string
     * @param user string
     * @param pass string
     * @param host string
     * @param port int
     */
    bool Connect(
      const std::string &name,
      const std::string &user,
      const std::string &pass,
      const std::string &host,
      const int &port );

    /**
     * Returns a vtkAlderMySQLQuery object for performing queries.
     * This method should only be used by Model objects.
     */
    vtkSmartPointer<vtkAlderMySQLQuery> GetQuery() const;

    /**
     * Returns a list of column names for a given table
     * @param table the name of a table to retrieve column names
     * @throws      runtime_error
     */
    std::vector<std::string> GetColumnNames( const std::string &table ) const;

    /**
     * Returns whether a table exists in the database.
     * @param table the name of a table
     */
    bool TableExists( const std::string &table ) const;

    /**
     * Returns whether a table column exists.
     * @param table  the name of a table
     * @param column the name of a column within the table
     */
    bool ColumnExists( const std::string &table, const std::string &column ) const;

    /**
     * Returns the default value for a table's column.
     * @param table  the name of a table
     * @param column the name of a column within the table
     * @return       the default value of the column as a vtkVariant
     * @throws       runtime_error
     */
    vtkVariant GetColumnDefault( const std::string &table, const std::string &column ) const;

    /**
     * Returns whether a table's column value may be null.
     * @param table  the name of a table
     * @param column the name of a column within the table
     * @throws       runtime_error
     */
    bool IsColumnNullable( const std::string &table, const std::string &column ) const;

    /**
     * Returns whether a table's column is a foreign key.
     * NOTE: it is not possible to get this information from the information schema. Instead,
     * this method uses the convention that all foreign keys end in "Id".
     * @param table  the name of a table
     * @param column the name of a column within the table
     * @throws       runtime_error
     */
    bool IsColumnForeignKey( const std::string &table, const std::string &column ) const;

  protected:
    Database();
    ~Database() {}

    /**
     * An internal method which is called once to read all table metadata from the
     * information_schema database.
     */
    void ReadInformationSchema();

    vtkSmartPointer<vtkAlderMySQLDatabase> MySQLDatabase;
    std::map< std::string,std::map< std::string,std::map< std::string, vtkVariant > > > Columns;

  private:
    Database( const Database& ); // Not implemented
    void operator=( const Database& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
