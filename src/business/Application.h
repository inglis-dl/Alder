/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Application.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Application
 * @namespace Alder
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Central class used to share information throughout the application
 *
 * This class is a singleton which is meant to be used anywhere throughout
 * the application as a means of accessing global application information.
 * It includes links to the image viewer, configuration, database, connection
 * to Opal and tracks the state of the application such as active user and
 * study.
 */

#ifndef __Application_h
#define __Application_h

#include <ModelObject.h>
#include <Utilities.h>

#include <vtkCommand.h>

#include <iostream>
#include <ostream>
#include <stdexcept>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Configuration;
  class Database;
  class Image;
  class Interview;
  class OpalService;
  class User;
  class Application : public ModelObject
  {
  public:
    vtkTypeMacro( Application, ModelObject );
    static Application *GetInstance();
    static void DeleteInstance();

    enum CustomEvents
    {
      ActiveUserEvent = vtkCommand::UserEvent + 100,
      ActiveInterviewEvent,
      ActiveInterviewUpdateImageDataEvent,
      ActiveImageEvent,
      ActiveAtlasImageEvent
    };

    //@{
    /**
     * Logging functions.
     */
    bool OpenLogFile();
    void Log( const std::string &message );
    void LogBacktrace();
    //@}

    /**
     * Reads configuration variables from a given file.
     * @param filename the file to read the configuration from.
     */
    bool ReadConfiguration( const std::string &fileName );

    /**
     * Uses database values in the configuration to connect to a database.
     */
    bool ConnectToDatabase();

    /**
     * Tests whether the application has read/write access to the image data path.
     */
    bool TestImageDataPath();

    /**
     * Uses opal values in the configuration to set up a connection to Opal.
     */
    void SetupOpalService();

    /**
     * Uses an opal datasource to update alder db tables.
     */
    void UpdateDatabase();

    /**
     * Resets the state of the application to its initial state.
     */
    void ResetApplication();

    vtkGetObjectMacro( Config, Configuration );
    vtkGetObjectMacro( DB, Database );
    vtkGetObjectMacro( Opal, OpalService );
    vtkGetObjectMacro( ActiveUser, User );
    vtkGetObjectMacro( ActiveInterview, Interview );
    vtkGetObjectMacro( ActiveImage, Image );
    vtkGetObjectMacro( ActiveAtlasImage, Image );

    /**
     * When setting the active user the active interview will be set to the interview stored in the user's
     * record if the user being set is not null.
     * @param user a User record object
     */
    virtual void SetActiveUser( User *user );

    /**
     * When setting the active interview the active image is automatically removed and,
     * if there is an active user, the active interview is stored in the user's record.
     * @param interview an Interview record object
     */
    virtual void SetActiveInterview( Interview *interview );

    /**
     * Sets the active image.
     * @param image an Image record object
     */
    virtual void SetActiveImage( Image *image );

    /**
     * Sets the active atlas image.
     * @param image an Image record object
     */
    virtual void SetActiveAtlasImage( Image *image );

    /**
     * Creates a new instance of a model object given its class name.
     * @param className  the class name of the object to create
     * @return           a model object
     * @throws           runtime_error
     */
    ModelObject* Create( const std::string &className ) const
    {
      // make sure the constructor registry has the class name being asked for
      auto pair = this->ConstructorRegistry.find( className );
      if( pair == this->ConstructorRegistry.end() )
      {
        std::stringstream stream;
        stream << "Tried to create object of type \""
               << className << "\" which doesn't exist in the constructor registry";
        throw std::runtime_error( stream.str() );
      }
      return pair->second();
    }

    /**
     * Compilers mangle class names at compile time.  This method provides the
     * unmangled name (without namespace).  In order for this to work all classes
     * must be registered in this class' constructor.
     * @param mangledName  a mangled class name
     * @return             the unmangled class name
     * @throws             runtime_error
     */
    std::string GetUnmangledClassName( const std::string &mangledName ) const;

    vtkSetMacro( AbortFlag, bool );
    vtkGetMacro( AbortFlag, bool );

  protected:
    Application();
    ~Application();

    static Application *New();
    static Application *Instance;

    Configuration *Config;
    Database *DB;
    OpalService *Opal;
    User *ActiveUser;
    Interview *ActiveInterview;
    Image *ActiveImage;
    Image *ActiveAtlasImage;
    bool AbortFlag;

  private:
    Application( const Application& );  // Not implemented.
    void operator=( const Application& );  // Not implemented.

    std::map< std::string, ModelObject*(*)() > ConstructorRegistry;
    std::map< std::string, std::string > ClassNameRegistry;
    std::ofstream LogStream;
  };

  template <class T> ModelObject* createInstance() { return T::New(); }
}

/** @} end of doxygen group */

#endif
