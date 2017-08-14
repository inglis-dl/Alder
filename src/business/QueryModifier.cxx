/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QueryModifier.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <QueryModifier.h>

// Alder includes
#include <Application.h>
#include <Database.h>

// VTK includes
#include <vtkMySQLQuery.h>
#include <vtkObjectFactory.h>
#include <vtkSmartPointer.h>

// C++ includes
#include <sstream>
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(QueryModifier);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  QueryModifier::QueryModifier()
  {
    this->LimitCount = 0;
    this->LimitOffset = 0;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Reset()
  {
    this->LimitCount = 0;
    this->LimitOffset = 0;
    this->WhereList.clear();
    this->GroupList.clear();
    this->OrderList.clear();
    this->JoinList.clear();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Join(const std::string table, const std::string onLeft,
    const std::string onRight, const JoinType type, const bool append)
  {
    JoinParameter p;
    p.type = type;
    p.table = table;
    p.onLeft = onLeft;
    p.onRight = onRight;
    if (append)
      this->JoinList.push_back(p);
    else
      this->JoinList.insert(this->JoinList.begin(),p);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetJoin() const
  {
    std::string statement;
    std::stringstream stream;
    // loop through each join parameter
    for (auto it = this->JoinList.cbegin(); it != this->JoinList.cend(); ++it)
    {
      statement = "";
      switch (it->type)
      {
        case QueryModifier::LEFT:  statement = "LEFT "; break;
        case QueryModifier::RIGHT: statement = "RIGHT "; break;
        case QueryModifier::INNER: statement = "INNER "; break;
        case QueryModifier::CROSS: statement = "CROSS "; break;
      }
      statement += "JOIN ";
      statement += it->table;
      statement += " ON ";
      statement += it->onLeft;
      statement += "=";
      statement += it->onRight;
      statement += " ";

      stream << statement;
    }

    return stream.str();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Where(
    const std::string column, const std::string oper,
    const vtkVariant value, const bool format, const bool logicalOr)
  {
    WhereParameter p;
    p.column = column;
    p.oper = oper;
    p.value = value;
    p.format = format;
    p.logicalOr = logicalOr;
    this->WhereList.push_back(p);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::WhereBracket(const bool open, const bool logicalOr)
  {
    WhereParameter p;
    p.bracket = open ? QueryModifier::OPEN : QueryModifier::CLOSE;
    p.logicalOr = logicalOr;
    this->WhereList.push_back(p);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Group(const std::string column)
  {
    this->GroupList.push_back(column);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Order(const std::string column, const bool desc)
  {
    this->OrderList[column] = desc;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Limit(const int count, const int offset)
  {
    this->LimitCount = count;
    this->LimitOffset = offset;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetSql(const bool appending) const
  {
    std::string retVal;
    retVal += appending ? "" : this->GetJoin();
    if (!retVal.empty() && !isspace(*retVal.rbegin())) retVal += " ";
    retVal += this->GetWhere(appending);
    if (!retVal.empty() && !isspace(*retVal.rbegin())) retVal += " ";
    retVal += this->GetGroup();
    if (!retVal.empty() && !isspace(*retVal.rbegin())) retVal += " ";
    retVal += this->GetOrder();
    if (!retVal.empty() && !isspace(*retVal.rbegin())) retVal += " ";
    retVal += this->GetLimit();
    return retVal;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetWhere(const bool appending) const
  {
    if (this->WhereList.empty()) return "";

    bool firstItem = true, lastOpenBracket = false;
    std::string statement;
    std::stringstream stream;

    // loop through each where parameter
    for (auto it = this->WhereList.cbegin(); it != this->WhereList.cend(); ++it)
    {
      if (QueryModifier::NONE != it->bracket)
      {
        statement = (QueryModifier::OPEN == it->bracket ? "(" : ")");
      }
      else
      {
        // format the value (unless specified not to)
        vtkVariant value = it->value;
        if (it->format && value.IsValid())
        {
          // we need a query object to escape the sql :(
          vtkSmartPointer<vtkMySQLQuery> query =
            Application::GetInstance()->GetDB()->GetQuery();
          value = query->EscapeString(value.ToString());
        }

        if (!value.IsValid())
        {
          statement = it->column;
          statement += ("=" == it->oper ? " IS NULL" : " IS NOT NULL");
        }
        else
        {
          statement = it->column;
          statement += " ";
          statement += it->oper;
          statement += ("IN" == it->oper ? " (" : " ");
          statement += value.ToString();
          statement += ("IN" == it->oper ? ")" : "");
        }
      }

      // add the logical connecting statement if necessary
      if ((!firstItem || appending) &&
          QueryModifier::CLOSE != it->bracket && !lastOpenBracket)
        stream << (it->logicalOr ? " OR" : " AND");

      stream << " " << statement;

      // keep track of whether this statement is an open bracket
      if (QueryModifier::OPEN == it->bracket) lastOpenBracket = true;
      if (firstItem) firstItem = false;
    }

    // add "WHERE" at the front, if necessary
    std::string sql = stream.str();
    if (!appending || 0 < sql.size()) sql = "WHERE" + sql;

    return sql;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetGroup() const
  {
    std::stringstream sql;
    bool first = true;

    for (auto it = this->GroupList.cbegin(); it != this->GroupList.cend(); ++it)
    {
      sql << (first ? "GROUP BY " : ", ") << *it;
      if (first) first = false;
    }

    return sql.str();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetOrder() const
  {
    std::stringstream sql;
    bool first = true;

    for (auto it = this->OrderList.cbegin(); it != this->OrderList.cend(); ++it)
    {
      sql << (first ? "ORDER BY " : ", ")
          << it->first << " " << (it->second ? "DESC" : "");
      if (first) first = false;
    }

    return sql.str();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string QueryModifier::GetLimit() const
  {
    std::stringstream sql;
    if (0 < this->LimitCount)
      sql << "LIMIT " << this->LimitCount << " OFFSET " << this->LimitOffset;

    return sql.str();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void QueryModifier::Merge(QueryModifier *modifier)
  {
    this->JoinList.insert(
      this->JoinList.end(), modifier->JoinList.begin(),
      modifier->JoinList.end());
    this->WhereList.insert(
      this->WhereList.end(), modifier->WhereList.begin(),
      modifier->WhereList.end());
    this->GroupList.insert(
      this->GroupList.end(), modifier->GroupList.begin(),
      modifier->GroupList.end());
    this->OrderList.insert(
      modifier->OrderList.begin(), modifier->OrderList.end());
  }
}  // namespace Alder
