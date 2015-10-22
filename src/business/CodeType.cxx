/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeType.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <CodeType.h>

#include <Utilities.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( CodeType );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int CodeType::GetUsage()
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "JOIN Code ON Code.CodeTypeId=CodeType.Id "
           << "JOIN Image ON Image.Id=Code.ImageId "
           << "JOIN User ON User.Id=Code.UserId "
           << "JOIN Rating ON Rating.ImageId=Image.Id "
           << "AND Rating.UserId=User.Id "
           << "WHERE CodeType.Id=" << this->Get("Id").ToString();

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
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
  void CodeType::GetUsageById( std::map<int,int>& codeIdUsageMap )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT CodeType.Id, SUM(IF(Code.Id IS NULL, 0, 1)) AS Count "
           << "FROM CodeType "
           << "LEFT JOIN Code ON Code.CodeTypeId=CodeType.Id "
           << "LEFT JOIN Image ON Image.Id=Code.ImageId "
           << "LEFT JOIN User ON User.Id=Code.UserId "
           << "LEFT JOIN Rating ON Rating.ImageId=Image.Id "
           << "AND Rating.UserId=User.Id "
           << "GROUP BY CodeType.Id";

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }

    codeIdUsageMap.clear();
    while( query->NextRow() )
    {
      codeIdUsageMap[query->DataValue(0).ToInt()] = query->DataValue(1).ToInt();
    }
  }

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool CodeType::IsUnique( const std::string& code, const int& value, const int& groupId )
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "WHERE Code='" << code << "' "
           << "AND Value=" << vtkVariant(value).ToString() << " " 
           << "AND GroupId=" << vtkVariant(groupId).ToString();

    app->Log( "Querying Database: " + stream.str() );
    vtkSmartPointer<vtkAlderMySQLQuery> query = app->GetDB()->GetQuery();
    query->SetQuery( stream.str().c_str() );
    query->Execute();
    if( query->HasError() )
    {   
      app->Log( query->GetLastErrorText() );
      throw std::runtime_error( "There was an error while trying to query the database." );
    }   
    // only has one row
    query->NextRow();
    return (0 == query->DataValue( 0 ).ToInt());
  }
}
