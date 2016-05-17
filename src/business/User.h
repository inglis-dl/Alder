/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   User.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class User
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the User table
 */

#ifndef __User_h
#define __User_h

// Alder includes
#include <ActiveRecord.h>

// C++ includes
#include <iostream>
#include <string>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class User : public ActiveRecord
  {
    public:
      static User* New();
      vtkTypeMacro(User, ActiveRecord);
      std::string GetName() const { return "User"; }

      /**
       * Reset the user password to default "password".
       */
      virtual void ResetPassword();

      /**
       * Check if the password is the user's password.
       * @param password the password to check
       */
      virtual bool IsPassword(const std::string& password);

      /**
       * Get the default password.
       * @return default password
       */
      static std::string GetDefaultPassword() { return "password"; }

      /**
       * Get a modifier to enforce user specific modality constraints
       * @param modifier the QueryModifier to initialize
       */
      void InitializeExamModifier(QueryModifier* modifier);

    protected:
      User() {}
      ~User() {}

      /**
       * Override parent class method to set a column value.  The User
       * class's Name and Password table columns require additional
       * validation checks.
       * @param column name of a column in the User table
       * @param value  the value to set the column to
       */
      virtual void SetVariant(const std::string column, const vtkVariant value);

    private:
      User(const User&);  // Not implemented
      void operator=(const User&);  // Not implemented
  };
}  // namespace Alder

/** @} end of doxygen group */

#endif
