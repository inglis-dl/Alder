/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   QueryModifier.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class QueryModifier
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief A class used to modifier a database query.
 *
 * QueryModifier provides a way to modify a database query including where, limit and offset keywords.
 * This object can be passed to the ActiveRecord class when querying records.
 */

#ifndef __QueryModifier_h
#define __QueryModifier_h

#include <ModelObject.h>

#include <vtkVariant.h>

#include <map>
#include <vector>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class QueryModifier : public ModelObject
  {
    enum BracketType
    {
      NONE,
      OPEN,
      CLOSE
    };

    enum JoinType {
      PLAIN,
      LEFT,
      RIGHT,
      CROSS,
      INNER
    };

    struct WhereParameter
    {
      WhereParameter() : format( true ), logicalOr( false ), bracket( QueryModifier::NONE ) {}
      std::string column;
      std::string oper;
      vtkVariant value;
      bool format;
      bool logicalOr;
      BracketType bracket;
    };

    struct JoinParameter
    {
      JoinParameter() : type( QueryModifier::PLAIN ) {}
      std::string table;
      std::string on_left;
      std::string on_right;
      JoinType type;
    };

  public:
    static QueryModifier *New();
    vtkTypeMacro( QueryModifier, ModelObject );

    /**
     *  Convenience method to reset the modifier.     *
     */
    virtual void Reset();

    /**
     * Add a where statement to the modifier.
     * This method appends where clauses onto the end of already existing where clauses.
     */
    virtual void Where(
      const std::string column, const std::string oper, const vtkVariant value,
      const bool format = true, const bool logicalOr = false );

    /**
     * Add where statement which will be "or" combined to the modifier.
     * This is a convenience method which makes where() calls more readable.
     */
    virtual void OrWhere(
      const std::string column, const std::string oper, const vtkVariant value, const bool format = true )
    { this->Where( column, oper, value, format, true ); }

    /**
     * Add a bracket to a where statement
     */
    virtual void WhereBracket( const bool open, const bool logicalOr = false );

    /**
     * Add a group by statement to the modifier.
     * This method appends group by clauses onto the end of already existing group by clauses.
     */
    virtual void Group( const std::string column );

    /**
     * Adds an order statement to the modifier.
     * This method appends order clauses onto the end of already existing order clauses.
     */
    virtual void Order( const std::string column, const bool desc = false );

    /**
     * Add order descending statement to the modifier.
     * This is a convenience method which makes order() calls more readable.
     */
    virtual void OrderDesc( const std::string column )
    { this->Order( column, true ); }

    /**
     * Sets a limit to how many rows are returned.
     * This method sets the total number of rows and offset to begin selecting by.
     */
    virtual void Limit( const int count, const int offset = 0 );

    /**
     * Returns the modifier as an SQL statement (same as calling each individual get_*() method.
     */
    virtual std::string GetSql( const bool appending = false ) const;

    /**
     * Returns an SQL join statement.
     * This method should only be called by a record class and only after all modifications
     * have been set.
     */
    virtual std::string GetJoin() const;

    /**
     * Returns an SQL where statement.
     * This method should only be called by a record class and only after all modifications
     * have been set.
     */
    virtual std::string GetWhere( const bool appending = false ) const;

    /**
     * Returns an SQL group statement.
     * This method should only be called by a record class and only after all modifications
     * have been set.
     */
    virtual std::string GetGroup() const;

    /**
     * Returns an SQL order statement.
     * This method should only be called by a record class and only after all modifications
     * have been set.
     */
    virtual std::string GetOrder() const;

    /**
     * Returns an SQL limit statement.
     * This method should only be called by a record class and only after all modifications
     * have been set.
     */
    virtual std::string GetLimit() const;

    /**
     * Merges another modifier with this one.  Merging only includes where, group and order items.
     */
    virtual void Merge( QueryModifier *modifier );

    /**
     * Add a join statement to the modifier.
     * This method prepends join clauses onto already existing join clauses.
     */
    virtual void Join( const std::string table,
                       const std::string on_left,
                       const std::string on_right,
                       const JoinType type = QueryModifier::PLAIN );

  protected:
    QueryModifier();
    ~QueryModifier() {}

    std::vector<WhereParameter> WhereList;
    std::vector<JoinParameter> JoinList;
    std::map<std::string,bool> OrderList;
    std::vector<std::string> GroupList;
    int LimitCount;
    int LimitOffset;

  private:
    QueryModifier( const QueryModifier& ); // Not implemented
    void operator=( const QueryModifier& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
