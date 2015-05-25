/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   ProgressProxy.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*

/**
 * @class ProgressProxy
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief A proxy for communicating progress events between businesss and UI layers
 */

#ifndef __ProgressProxy_h
#define __ProgressProxy_h

#include <Application.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class ProgressProxy
  {
  public:
    ProgressProxy() : progressType(ProgressProxy::Global), curlProgress(true), busyProgress(false) {
      this->application = Application::GetInstance();
    }
    ~ProgressProxy();

    /**
     * Update the progress as a decimal percent [0.0,1.0].  Typically
     * call this method during a loop iteration.
     */
    void UpdateProgress( const double& percent );

    /**
     * Configure progress elements at the UI layer.
     */
    void ConfigureProgress();

    /**
     * Start progress monitoring.  Typically call this method prior to loop iterations.
     */
    void StartProgress();

    /**
     * End progress monitoring.  Typically call this method after all loop iterations.
     */
    void EndProgress();

    /**
     * Get the application's abort status.
     */
    int GetAbortStatus() {
      return this->application->GetAbortFlag();
    }

    /**
     * Turn on curl progress monitoring.
     */
    void SetCurlProgressOn() {
      this->curlProgress = true;
    }

    /**
     * Turn off curl progress monitoring.
     */
    void SetCurlProgressOff() {
      this->curlProgress = false;
    }

    /**
     * Set the use of curl progress monitoring during OpalService Read calls.
     */
    void SetCurlProgress( const bool& state ) {
      this->curlProgress = state;
    }

    /**
     * Convenience method to turn on the progress as a busy signal.
     */
    void SetBusyProgressOn() {
      this->busyProgress = true;
    }

    /**
     * Convenience method to turn off the progress as a busy signal.
     */
    void SetBusyProgressOff() {
      this->busyProgress = false;
    }

    /**
     * Set the progress expressed at the UI layer as a busy signal rather than
     * an incremental percentage.
     */
    void SetBusyProgress( const bool& state ) {
      this->busyProgress = state;
    }

    /**
     * Get the state of progress expressed as a busy signal.
     */
    bool GetBusyProgress() {
      return this->busyProgress;
    }

    /**
     * Progress type enum.
     */
    enum ProgressType
    {
      Local = 0,
      Global
    };

    /**
     * Set the progress type to local.  Local progress events
     * are expressed at the UI layer via the top QProgressBar of QVTKProgressDialog.
     */
    void SetProgressTypeLocal() {
      this->progressType = ProgressProxy::Local;
    }

    /**
     * Set the progress type to global.  Global progress events
     * are expressed at the UI layer via the bottom QProgressBar of QVTKProgressDialog.
     */
    void SetProgressTypeGlobal() {
      this->progressType = ProgressProxy::Global;
    }

    /**
     * Set the progress type to either global (eg., outer loop) or local (eg., inner loop or
     * fine grained progress measurement of progress).  The local progress is typically
     * used by the OpalService during a Read to report on curl progress of data download.
     */
    void SetProgressType( const int& type ) {
      this->progressType =
        ProgressProxy::Local == type ? ProgressProxy::Local : ProgressProxy::Global;
    }

    /**
     * Get the progress type.
     */
    int GetProgressType() {
      return this->progressType;
    }

  protected:
    /**
     *  The application singleton
     */
    Application* application;

    /**
     * ivars
     */
    int progressType;  /** default Global */
    bool busyProgress; /** default false */
    bool curlProgress; /** default true */
    std::pair<int, bool>   configurationData;
    std::pair<int, double> progressData;

  private:
    ProgressProxy( const ProgressProxy& ); // Not implemented
    void operator=( const ProgressProxy& ); // Not implemented
  };
}

/** @} end of doxygen group */

#endif
