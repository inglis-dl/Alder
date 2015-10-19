/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   CodeGroup.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <CodeGroup.h>

#include <Utilities.h>

#include <vtkObjectFactory.h>

namespace Alder
{
  vtkStandardNewMacro( CodeGroup );

  //-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  int CodeGroup::GetUsage()
  {
    Application *app = Application::GetInstance();
    std::stringstream stream;
    stream << "SELECT COUNT(*) "
           << "FROM CodeType "
           << "WHERE CodeGroupId=" << this->Get("Id").ToString();

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
}
