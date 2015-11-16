/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ActiveRecord.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class ActiveRecord
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Abstract base class for all active record classes
 *
 * ActiveRecord provides a programming interface to the database.  All classes
 * which correspond directly to a table (and named exactly the same way) must
 * extend this class.
 */

#ifndef __ActiveRecord_h
#define __ActiveRecord_h

#include <ModelObject.h>

#include <Application.h>
#include <Database.h>
#include <QueryModifier.h>

#include <vtkAlderMySQLQuery.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>

#include <map>
#include <stdexcept>
#include <typeinfo>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class ActiveRecord : public ModelObject
  {
  public:
    vtkTypeMacro( ActiveRecord, ModelObject );
    void PrintSelf( ostream& os, vtkIndent indent );

    /**
     * Returns whether this record has a particular column.
     * @param column the name of the column
     */
    bool ColumnNameExists( const std::string column );

    //@{
    /**
     * Loads a specific record from the database.  Input parameters must include the values
     * of a primary or unique key in the corresponding table.
     * @param key   a primary or unique key
     * @param value the value associated with the key
     * @throws      runtime_error
     */
    bool Load( const std::string key, const int value )
    {
      return this->Load( key, vtkVariant(value).ToString() );
    }
    bool Load( const std::string key, const std::string value )
    {
      return this->Load( std::pair< std::string, std::string >( key, value ) );
    }
    bool Load( std::pair< const std::string, const std::string > pair )
    {
      std::map< std::string, std::string > map;
      map.insert( pair );
      return this->Load( map );
    }
    virtual bool Load( const std::map< std::string, std::string > map );
    //@}

    /**
     * Saves the record's current values to the database.  If the record was not loaded
     * then a new record will be inserted into the database.
     * @param replace whether to replace an existing record
     */
    virtual void Save( const bool replace = false );

    /**
     * Removes the current record from the database.
     * @throws runtime_error
     */
    virtual void Remove();

    /**
     * Get the id of the last inserted record.
     * @return id of the last inserted record
     */
    int GetLastInsertId() const;

    /**
     * Provides a list of all records which exist in a table.
     * @param list     an existing vector to put all records into
     * @param modifier QueryModifier
     */
    template< class T > static void GetAll(
      std::vector< vtkSmartPointer< T > > *list, QueryModifier *modifier = NULL )
    { // we have to implement this here because of the template
      Application *app = Application::GetInstance();
      // get the class name of T, return error if not found
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      std::stringstream stream;
      stream << "SELECT * FROM " << type;
      if( NULL != modifier ) stream << " " << modifier->GetSql();
      vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();

      app->Log( "Querying Database: " + stream.str() );
      query->SetQuery( stream.str().c_str() );
      query->Execute();

      if( query->HasError() )
      {
        app->Log( query->GetLastErrorText() );
        throw std::runtime_error( "There was an error while trying to query the database." );
      }

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take( T::SafeDownCast( T::New() ) );
        record->LoadFromQuery( query );
        list->push_back( record );
      }
    }

    //@{
    /**
     * Provides a list of all records which are related to this record by foreign key or
     * has a joining N-to-N relationship with another table.
     * @param list     an existing vector to put all records into
     * @param modifier a QueryModifier to refine selection of the list elements
     * @throws         runtime_error
     */
    template< class T > void GetList(
      std::vector< vtkSmartPointer< T > > *list, QueryModifier *modifier = NULL )
    { this->GetList( list, modifier, "" ); }
    template< class T > void GetList(
      std::vector< vtkSmartPointer< T > > *list, const std::string override )
    { this->GetList( list, NULL, override ); }
    template< class T > void GetList(
      std::vector< vtkSmartPointer< T > > *list, QueryModifier *modifier, const std::string override )
    {
      Application *app = Application::GetInstance();
      Database *db = app->GetDB();
      std::stringstream stream;
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      vtkSmartPointer<vtkAlderMySQLQuery> query = db->GetQuery();

      vtkNew<QueryModifier> mod;
      if( NULL != modifier ) mod->Merge( modifier );

      // if no override is provided, figure out necessary table/column names
      std::string joiningTable = override.empty() ? this->GetName() + "Has" + type : override;
      std::string column = override.empty() ? this->GetName() + "Id" : override;

      int relationship = this->GetRelationship( type, override );
      if( ActiveRecord::ManyToMany == relationship )
      {
        stream << "SELECT " << type << ".* "
               << "FROM " << joiningTable << " "
               << "JOIN " << type << " ON " << joiningTable << "." << type << "Id = " << type << ".Id";
        mod->Where( this->GetName() + "Id", "=", this->Get( "Id" ).ToString() );
      }
      else if( ActiveRecord::OneToMany == relationship )
      {
        stream << "SELECT * FROM " << type;
        mod->Where( column, "=", this->Get( "Id" ).ToString() );
      }
      else // no relationship (we don't support one-to-one relationships)
      {
        std::stringstream stream;
        stream << "Cannot determine relationship between " << this->GetName() << " and " << type;
        throw std::runtime_error( stream.str() );
      }

      // execute the query, check for errors, put results in the list
      std::string sql = stream.str() + " " + mod->GetSql();
      app->Log( "Querying Database: " + sql );
      query->SetQuery( sql.c_str() );
      query->Execute();
      if( query->HasError() )
      {
        app->Log( query->GetLastErrorText() );
        throw std::runtime_error( "There was an error while trying to query the database." );
      }

      while( query->NextRow() )
      {
        // create a new instance of the child class
        vtkSmartPointer< T > record = vtkSmartPointer< T >::Take( T::SafeDownCast( app->Create( type ) ) );
        record->LoadFromQuery( query );
        list->push_back( record );
      }
    }
    //@}

    /**
     * Returns whether a record has a relationship with another record.  This can either be
     * due to a joining table (N-to-N relationship) or a foreign key column (1-to-N relationship)
     * @param record   the record to test for a relationship with
     * @param override an alternate joining table name
     * @throws         runtime_error
     */
    template <class T> bool Has( vtkSmartPointer< T > &record, const std::string override = "" )
    {
      Application *app = Application::GetInstance();
      Database *db = app->GetDB();
      std::stringstream sql;
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      vtkSmartPointer<vtkAlderMySQLQuery> query = db->GetQuery();

      // if no override is provided, figure out necessary table/column names
      std::string joiningTable = override.empty() ? this->GetName() + "Has" + type : override;
      std::string column = override.empty() ? this->GetName() + "Id" : override;

      int relationship = this->GetRelationship( type, override );
      if( ActiveRecord::ManyToMany == relationship )
      {
        sql << "SELECT COUNT(*) FROM " << joiningTable << " "
            << "WHERE " << this->GetName() << "Id = " << this->Get( "Id" ).ToString() << " "
            << "AND " << type << "Id = " << record->Get( "Id" ).ToString();
      }
      else if( ActiveRecord::OneToMany == relationship )
      {
        sql << "SELECT COUNT(*) FROM " << type << " "
            << "WHERE " << column << " = " << this->Get( "Id" ).ToString();
      }
      else // no relationship (we don't support one-to-one relationships)
      {
        std::stringstream stream;
        stream << "Cannot determine relationship between " << this->GetName() << " and " << type;
        throw std::runtime_error( stream.str() );
      }

      // execute the query, check for errors, put results in the list
      app->Log( "Querying Database: " + sql.str() );
      query->SetQuery( sql.str().c_str() );
      query->Execute();
      if( query->HasError() )
      {
        app->Log( query->GetLastErrorText() );
        throw std::runtime_error( "There was an error while trying to query the database." );
      }

      query->NextRow();
      return 0 < query->DataValue( 0 ).ToInt();
    }

    /**
     * This method should only be used by records which have a many to many relationship
     * with another record type.  It adds another record to this one.
     * @param record a related record to add to this one
     * @throws       runtime_error
     */
    template <class T> void AddRecord( vtkSmartPointer< T > &record )
    {
      Application *app = Application::GetInstance();
      Database *db = app->GetDB();
      std::stringstream sql;
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      vtkSmartPointer<vtkAlderMySQLQuery> query = db->GetQuery();

      // first make sure we have the correct relationship with the given record
      if( ActiveRecord::ManyToMany != this->GetRelationship( type ) )
        throw std::runtime_error( "Cannot add record without many-to-many relationship." );

      sql << "REPLACE INTO " << this->GetName() << "Has" << type << " ("
          << this->GetName() << "Id, " << type << "Id) "
          << "VALUES (" << this->Get( "Id" ).ToInt() << ", " << record->Get( "Id" ).ToInt() << ")";

      // execute the query, check for errors, put results in the list
      app->Log( "Querying Database: " + sql.str() );
      query->SetQuery( sql.str().c_str() );
      query->Execute();
    }

    /**
     * This method should only be used by records which have a many to many relationship
     * with another record type.  It removes the record related to this one.
     * @param record a related record to remove from this one
     * @throws       runtime_error
     */
    template <class T> void RemoveRecord( vtkSmartPointer< T > &record )
    {
      Application *app = Application::GetInstance();
      Database *db = app->GetDB();
      std::stringstream sql;
      std::string type = app->GetUnmangledClassName( typeid(T).name() );
      vtkSmartPointer<vtkAlderMySQLQuery> query = db->GetQuery();

      // first make sure we have the correct relationship with the given record
      if( ActiveRecord::ManyToMany != this->GetRelationship( type ) )
        throw std::runtime_error( "Cannot remove record without many-to-many relationship." );

      sql << "DELETE FROM " << this->GetName() << "Has" << type << " "
          << "WHERE " << this->GetName() << "Id = " << this->Get( "Id" ).ToInt() << " "
          << "AND " << type << "Id = " << record->Get( "Id" ).ToInt();

      // execute the query, check for errors, put results in the list
      app->Log( "Querying Database: " + sql.str() );
      query->SetQuery( sql.str().c_str() );
      query->Execute();
    }

    /**
     * Returns the number of records which are related to this record by foreign key.
     * @param recordType the associated table name
     */
    int GetCount( const std::string recordType );

    /**
     * Get the value of any column in the record.
     * @param column the name of the column
     * @return       the value of the column as a vtkVariant
     * @throws       runtime_error
     */
    virtual vtkVariant Get( const std::string column );

    /**
     * Get the record which has a foreign key in this table.
     * @param record  a releated record to this one
     * @param column  an alternate column name to use instead of the default <table>Id
     * @return        success or fail if the record is found
     * @throws        runtime_error
     */
    template <class T> bool GetRecord( vtkSmartPointer< T > &record, std::string column = "" )
    {
      Application *app = Application::GetInstance();
      std::string table = app->GetUnmangledClassName( typeid(T).name() );

      // if no column name was provided, use the default (table name followed by Id)
      if( column.empty() ) column = table + "Id";

      // test to see if correct foreign key exists
      if( !this->ColumnNameExists( column ) )
      {
        std::stringstream error;
        error << "Tried to get \"" << table << "\" record but column \"" << column << "\" doesn't exist";
        throw std::runtime_error( error.str() );
      }

      vtkVariant v = this->Get( column );
      if( v.IsValid() )
      { // only create the record if the foreign key is not null
        record.TakeReference( T::SafeDownCast( Application::GetInstance()->Create( table ) ) );
        record->Load( "Id", this->Get( column ).ToString() );
      }

      return v.IsValid();
    }

    //@{
    /**
     * Set the value of any column in the record.
     * Note: this will only affect the active record in memory, to update the database
     * Save() needs to be called. If you wish to set the value to NULL then use the
     * SetNull() method instead of Set().
     * @param map a collection of column names and native values
     */
    template <class T> void Set( const std::map< std::string, T > map )
    {
      for( auto it = map.cbegin(); it != map.cend(); ++it )
        this->SetVariant( it->first, vtkVariant( it->second ) );
    }
    template <class T> void Set( const std::string column, const T value )
    { this->SetVariant( column, vtkVariant( value ) ); }
    void SetNull( const std::string column )
    { this->SetVariant( column, vtkVariant() ); }
    //@}

    /**
     * Must be extended by every child class.
     * Its value is always the name of the class (identical case).
     * @return the name of this class
     */
    virtual std::string GetName() const = 0;

    /**
     * Loads values into the record from a query's current row.
     * @param query a mysql query object
     */
    void LoadFromQuery( vtkAlderMySQLQuery *query );

  protected:
    ActiveRecord();
    ~ActiveRecord() {}

    /**
     * Sets up the record with default values for all table columns.
     */
    void Initialize();

    /**
     * Runs a check to make sure the record exists in the database.
     * @throws runtime_error
     */
    inline void AssertPrimaryId() const
    {
      auto pair = this->ColumnValues.find( "Id" );
      if( this->ColumnValues.end() == pair )
        throw std::runtime_error( "Assert failed: primary id column name is not Id" );

      vtkVariant id = pair->second;
      if( !id.IsValid() || 0 == id.ToInt() )
        throw std::runtime_error( "Assert failed: primary id for record is not set" );
    }

    /**
     * Internal method used by Set().
     * @param column  the column to set a value to
     * @param value   the value to set as a vtkVariant
     * @throws        runtime_error
     */
    virtual void SetVariant( const std::string column, const vtkVariant value );

    enum RelationshipType
    {
      None = 0,
      OneToOne,
      OneToMany,
      ManyToMany
    };

    /**
     * Determines the relationship between this record and another.
     * @param table
     * @param override
     */
    int GetRelationship( const std::string table, const std::string override = "" ) const;

    std::map<std::string,vtkVariant> ColumnValues;
    bool Initialized;

  private:
    ActiveRecord( const ActiveRecord& ); // Not implemented
    void operator=( const ActiveRecord& ); // Not implemented
    void DeleteColumnValues();
  };
}

/** @} end of doxygen group */

#endif
