/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Database.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Database.h>

// Alder includes
#include <Configuration.h>
#include <User.h>

// VTK includes
#include <vtkMySQLDatabase.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>
#include <vtkMySQLQuery.h>
#include <vtkTable.h>
#include <vtkVariant.h>

// C++ includes
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(Database);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Database::Database()
  {
    this->MySQLDatabase = vtkSmartPointer<vtkMySQLDatabase>::New();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::Connect(
    const std::string& name,
    const std::string& user,
    const std::string& pass,
    const std::string& host,
    const int& port)
  {
    // set the database parameters using the configuration object
    this->MySQLDatabase->SetDatabaseName(name.c_str());
    this->MySQLDatabase->SetUser(user.c_str());
    this->MySQLDatabase->SetHostName(host.c_str());
    this->MySQLDatabase->SetServerPort(port);
    bool success = this->MySQLDatabase->Open(pass.c_str());
    if (success) this->ReadInformationSchema();

    return success;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Database::ReadInformationSchema()
  {
    Application* app = Application::GetInstance();
    vtkSmartPointer<vtkMySQLQuery> query = this->GetQuery();

    std::stringstream stream;
    // the following query's first column MUST be table_name (index 0)
    // and second column MUST be table_column (index 1)
    stream << "SELECT table_name, column_name, column_type, "
           << "data_type, column_default, is_nullable "
           << "FROM information_schema.columns "
           << "WHERE table_schema = "
           << query->EscapeString(this->MySQLDatabase->GetDatabaseName()) << " "
           << "AND column_name != 'UpdateTimestamp' "
           << "AND column_name != 'CreateTimestamp' "
           << "ORDER BY table_name, ordinal_position";
    query->SetQuery(stream.str().c_str());
    query->Execute();

    if (query->HasError())
    {
      app->Log(query->GetLastErrorText());
      throw std::runtime_error(
        "There was an error while trying to query the database.");
    }

    std::string tableName = "";
    std::map<std::string, std::map<std::string, vtkVariant>> tableMap;
    while (query->NextRow())
    {
      // if we are starting a new table save the old one and start over
      if (query->DataValue(0).ToString() != tableName)
      {
        if (0 != tableName.size()) this->Columns.insert(
          std::pair<std::string,
            std::map<std::string,
              std::map<std::string, vtkVariant>>>(
                tableName, tableMap));
        tableName = query->DataValue(0).ToString();
        tableMap.clear();
      }

      // get this column's details
      std::map<std::string, vtkVariant> columnMap;
      for (int c = 2; c < query->GetNumberOfFields(); ++c)
        columnMap.insert(std::pair<std::string, vtkVariant>(
          query->GetFieldName(c), query->DataValue(c)));

      // add the column to the current table
      std::string columnName = query->DataValue(1).ToString();
      tableMap.insert(std::pair<std::string, std::map<std::string, vtkVariant>>(
        columnName, columnMap));
    }

    // save the last table
    if (0 != tableName.size()) this->Columns.insert(
      std::pair<std::string,
        std::map<std::string,
          std::map<std::string, vtkVariant>>>(
            tableName, tableMap));
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> Database::GetColumnNames(const std::string& table) const
  {
    auto tablePair = this->Columns.find(table);
    if (tablePair == this->Columns.cend())
    {
      std::stringstream error;
      error << "Tried to get column names for table \""
            << table
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    std::map<std::string,
      std::map<std::string, vtkVariant>> tableMap = tablePair->second;
    std::vector<std::string> columns;
    for (auto it = tableMap.cbegin(); it != tableMap.cend(); ++it)
      columns.push_back(it->first);

    return columns;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::TableExists(const std::string& table) const
  {
    return this->Columns.cend() != this->Columns.find(table);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::ColumnExists(const std::string& table,
    const std::string& column) const
  {
    auto tablePair = this->Columns.find(table);
    if (tablePair == this->Columns.cend())
    {
      std::stringstream error;
      error << "Tried to get whether a column exists from table \""
            << table
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    return tablePair->second.cend() != tablePair->second.find(column);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkVariant Database::GetColumnDefault(const std::string& table,
    const std::string& column) const
  {
    auto tablePair = this->Columns.find(table);
    if (tablePair == this->Columns.cend())
    {
      std::stringstream error;
      error << "Tried to get default column value from table \""
            << table
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    auto columnPair = tablePair->second.find(column);
    if (columnPair == tablePair->second.cend())
    {
      std::stringstream error;
      error << "Tried to get default column value for \""
            << table << "." << column << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    std::map<std::string, vtkVariant> columnMap = columnPair->second;
    return columnMap.find("column_default")->second;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::IsColumnNullable(const std::string& table,
    const std::string& column) const
  {
    auto tablePair = this->Columns.find(table);
    if (tablePair == this->Columns.cend())
    {
      std::stringstream error;
      error << "Tried to get column nullable from table \""
            << table
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    auto columnPair = tablePair->second.find(column);
    if (columnPair == tablePair->second.cend())
    {
      std::stringstream error;
      error << "Tried to get column nullable for \""
            << table << "." << column
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    std::map<std::string, vtkVariant> columnMap = columnPair->second;
    return "YES" == columnMap.find("is_nullable")->second.ToString();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Database::IsColumnForeignKey(const std::string& table,
    const std::string& column) const
  {
    auto tablePair = this->Columns.find(table);
    if (tablePair == this->Columns.cend())
    {
      std::stringstream error;
      error << "Tried to get column foreign key from table \""
            << table
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    auto columnPair = tablePair->second.find(column);
    if (columnPair == tablePair->second.cend())
    {
      std::stringstream error;
      error << "Tried to get column foreign key for \""
            << table << "." << column
            << "\" which doesn't exist";
      throw std::runtime_error(error.str());
    }

    return 3 <= column.size() &&
           0 == column.compare(column.size() - 2, 2, "Id");
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  vtkSmartPointer<vtkMySQLQuery> Database::GetQuery() const
  {
    return vtkSmartPointer<vtkMySQLQuery>::Take(
      vtkMySQLQuery::SafeDownCast(this->MySQLDatabase->GetQueryInstance()));
  }
}  // namespace Alder
