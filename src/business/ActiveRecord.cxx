/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ActiveRecord.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

#include <ActiveRecord.h>

#include <Application.h>
#include <Database.h>

#include <vtkAlderMySQLQuery.h>

#include <sstream>
#include <stdexcept>
#include <vector>

namespace Alder
{
  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  ActiveRecord::ActiveRecord()
  {
    this->Initialized = false;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecord::ColumnNameExists( const std::string &column )
  {
    // make sure the record is initialized
    if( !this->Initialized )
      this->Initialize();

    auto pair = this->ColumnValues.find( column );
    return this->ColumnValues.cend() != pair;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Initialize()
  {
    this->ColumnValues.clear();

    Database *db = Application::GetInstance()->GetDB();
    std::string name = this->GetName();
    std::vector<std::string> columns = db->GetColumnNames( name );

    // When first creating an active record we want the ColumnValues ivar to have an empty
    // value for every column in the active record's table.  We use mysql's information_schema
    // database for this purpose.  This is all implemented by the Database model
    for( auto it = columns.cbegin(); it != columns.cend(); ++it )
    {
      const std::string column = *it;
      vtkVariant columnDefault = db->GetColumnDefault( name, column );
      this->ColumnValues.insert( std::pair< std::string, vtkVariant >( column, columnDefault ) );
    }

    this->Initialized = true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::LoadFromQuery( vtkAlderMySQLQuery *query )
  {
    for( int c = 0; c < query->GetNumberOfFields(); ++c )
    {
      std::string column = query->GetFieldName( c );
      if( "CreateTimestamp" != column && "UpdateTimestamp" != column )
        this->ColumnValues.insert(
          std::pair< std::string, vtkVariant >( column, query->DataValue( c ) ) );
    }
    this->Initialized = true;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool ActiveRecord::Load( const std::map< std::string, std::string > &map )
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    this->ColumnValues.clear();

    // create an sql statement using the provided map
    std::string name = this->GetName();
    std::stringstream stream;
    stream << "SELECT * FROM " << name;
    for( auto it = map.cbegin(); it != map.cend(); ++it )
      stream << ( map.cbegin() == it ? " WHERE " : " AND " )
             << it->first << " = " << query->EscapeString( it->second );

    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    bool first = true;
    while( query->NextRow() )
    {
      // make sure we only have one record, throw exception if not
      if( !first )
      {
        std::stringstream error;
        error << "Loading " << name << " record resulted in multiple rows";
        throw std::runtime_error( error.str() );
      }

      this->LoadFromQuery( query );
      if( first ) first = false;
    }

    // if we didn't find a row then first is still true
    return !first;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Save( const bool &replace )
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    std::string name = this->GetName();
    std::stringstream stream;
    bool first = true;
    for( auto it = this->ColumnValues.cbegin(); it != this->ColumnValues.cend(); ++it )
    {
      if( "Id" != it->first )
      {
        stream << ( first ? "" :  ", " ) << it->first
               << " = " << ( it->second.IsValid() ? query->EscapeString( it->second.ToString() ) : "NULL" );
        if( first ) first = false;
      }
    }

    // different sql based on whether the record already exists or not
    if( !this->Get( "Id" ).IsValid() || 0 == this->Get( "Id" ).ToInt() )
    {
      // add the CreateTimestamp column
      stream << ( first ? "" :  ", " ) << "CreateTimestamp = NULL";

      // add a new record
      std::string s = stream.str();
      stream.str( "" );
      stream << ( replace ? "REPLACE" : "INSERT" ) << " INTO " << name << " SET " << s;
    }
    else
    {
      // update the existing record
      std::string s = stream.str();
      stream.str( "" );
      stream << "UPDATE " << name << " SET " << s
             << " WHERE Id = " << query->EscapeString( this->Get( "Id" ).ToString() );
    }

    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    // if the record's Id isn't set, get it based on the auto-increment property of the Id column
    if( !this->Get( "Id" ).IsValid() || 0 == this->Get( "Id" ).ToInt() )
    {
      this->Set( "Id", this->GetLastInsertId() );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetLastInsertId() const
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    std::stringstream stream;
    std::string name = this->GetName();
    stream << "SELECT Max( Id ) FROM " << name;

    app->Log( "Getting last insert id for table: " + name );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    // only has one row
    query->NextRow();
    return query->DataValue( 0 ).ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::Remove()
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    this->AssertPrimaryId();

    std::stringstream stream;
    stream << "DELETE FROM " << this->GetName() << " "
           << "WHERE Id = " << query->EscapeString( this->Get( "Id" ).ToString() );
    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetCount( const std::string &recordType )
  {
    Application *app = Application::GetInstance();
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    std::stringstream stream;
    stream << "SELECT COUNT(*) FROM " << recordType << " "
           << "WHERE " << this->GetName() << "Id = " << this->Get( "Id" ).ToString();

    app->Log( "Querying Database: " + stream.str() );
    query->SetQuery( stream.str().c_str() );
    query->Execute();

    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    // only has one row
    query->NextRow();
    return query->DataValue( 0 ).ToInt();
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkVariant ActiveRecord::Get( const std::string &column )
  {
    // make sure the column exists
    if( !this->ColumnNameExists( column ) )
    {
      std::stringstream error;
      error << "Tried to get column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    return this->ColumnValues.find( column )->second;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::SetVariant( const std::string column, const vtkVariant value )
  {
    // make sure the column exists
    if( !this->ColumnNameExists( column ) )
    {
      std::stringstream error;
      error << "Tried to set column \"" << this->GetName() << "." << column << "\" which doesn't exist";
      throw std::runtime_error( error.str() );
    }

    this->ColumnValues.find( column )->second = value;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int ActiveRecord::GetRelationship( const std::string &table, const std::string &override ) const
  {
    Database *db = Application::GetInstance()->GetDB();

    // if no override is provided, figure out necessary table/column names
    std::string name = this->GetName();
    std::string joiningTable = override.empty() ? name + "Has" + table : override;
    std::string column = override.empty() ? name + "Id" : override;

    if( db->TableExists( joiningTable ) )
    {
      return ActiveRecord::ManyToMany;
    }
    else if( db->ColumnExists( table, column ) )
    {
      return ActiveRecord::OneToMany;
    }

    return ActiveRecord::None;
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void ActiveRecord::PrintSelf( ostream& os, vtkIndent indent )
  {
    this->Superclass::PrintSelf( os, indent );

    os << indent << "Initialized: " << ( this->Initialized ? "Yes" : "No" ) << endl;
    os << indent << "Column Values:" << endl;
    for( auto it = this->ColumnValues.cbegin(); it != this->ColumnValues.cend(); ++it )
    {
      os << indent.GetNextIndent() << it->first << ": " << it->second
         << ( it->second.IsValid() ? it->second.ToString() : "NULL" ) << endl;
    }
  }
}
