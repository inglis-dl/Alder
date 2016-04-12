/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Wave.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Wave
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief An active record for the Wave table
 */

#ifndef __Wave_h
#define __Wave_h

#include <ActiveRecord.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Wave : public ActiveRecord
  {
  public:
    static Wave *New();
    vtkTypeMacro( Wave, ActiveRecord );
    std::string GetName() const { return "Wave"; }

    /**
     * Updates the Wave db table with Opal information.
     * @param aSource Opal project containing wave configuration data.
     */
    static void UpdateWaveData( const std::string &aSource );

    /**
     * Get the maximum UpdateTimestamp from the Interview table as an estimate
     * of when Interview data was updated from Opal.
     * @return timestamp string
     * @throws runtime_error
     */
    std::string GetMaximumInterviewUpdateTimestamp();

    /**
     * Updates the ScanType, Modality and WaveHasScanType db tables with Opal information
     * from this Wave record.
     */
    void UpdateScanTypeData();

    /**
     * Get the number of Interviews this Wave can have based on the number of Opal identifiers.
     * @return identifier count
     * @throws runtime_error
     */
    int GetIdentifierCount();

    /**
     * Get the maximum number of Exams that an Interview in this Wave can have.
     * @return exam count
     * @throws runtime_error
     */
    int GetMaximumExamCount();

  protected:
    Wave() {}
    ~Wave() {}

  private:
    Wave( const Wave& ); // Not implemented
    void operator=( const Wave& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
