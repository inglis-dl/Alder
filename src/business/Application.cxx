/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Application.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <Application.h>

// Alder includes
#include <Code.h>
#include <CodeType.h>
#include <CodeGroup.h>
#include <Common.h>
#include <Configuration.h>
#include <Database.h>
#include <Exam.h>
#include <Image.h>
#include <ImageNote.h>
#include <Interview.h>
#include <Modality.h>
#include <OpalService.h>
#include <ParticipantData.h>
#include <Rating.h>
#include <ScanType.h>
#include <Site.h>
#include <User.h>
#include <Utilities.h>
#include <Wave.h>

// VTK includes
#include <vtkDirectory.h>
#include <vtkObjectFactory.h>
#include <vtkVariant.h>

// C includes
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>

// C++ includes
#include <stdexcept>
#include <string>

namespace Alder
{
  Application* Application::Instance = NULL;  // set the initial application

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::Application()
  {
    this->AbortFlag = false;
    this->Config = Configuration::New();
    this->DB = Database::New();
    this->Opal = OpalService::New();
    this->ActiveUser = NULL;

    // populate the constructor and class name registries with all active record classes
    this->ConstructorRegistry["Exam"] = &createInstance<Exam>;
    this->ClassNameRegistry["Exam"] = typeid(Exam).name();
    this->ConstructorRegistry["Image"] = &createInstance<Image>;
    this->ClassNameRegistry["Image"] = typeid(Image).name();
    this->ConstructorRegistry["Interview"] = &createInstance<Interview>;
    this->ClassNameRegistry["Interview"] = typeid(Interview).name();
    this->ConstructorRegistry["Modality"] = &createInstance<Modality>;
    this->ClassNameRegistry["Modality"] = typeid(Modality).name();
    this->ConstructorRegistry["Rating"] = &createInstance<Rating>;
    this->ClassNameRegistry["Rating"] = typeid(Rating).name();
    this->ConstructorRegistry["User"] = &createInstance<User>;
    this->ClassNameRegistry["User"] = typeid(User).name();
    this->ConstructorRegistry["Code"] = &createInstance<Code>;
    this->ClassNameRegistry["Code"] = typeid(Code).name();
    this->ConstructorRegistry["CodeType"] = &createInstance<CodeType>;
    this->ClassNameRegistry["CodeType"] = typeid(CodeType).name();
    this->ConstructorRegistry["CodeGroup"] = &createInstance<CodeGroup>;
    this->ClassNameRegistry["CodeGroup"] = typeid(CodeGroup).name();
    this->ConstructorRegistry["ScanType"] = &createInstance<ScanType>;
    this->ClassNameRegistry["ScanType"] = typeid(ScanType).name();
    this->ConstructorRegistry["Site"] = &createInstance<Site>;
    this->ClassNameRegistry["Site"] = typeid(Site).name();
    this->ConstructorRegistry["Wave"] = &createInstance<Wave>;
    this->ClassNameRegistry["Wave"] = typeid(Wave).name();
    this->ConstructorRegistry["ImageNote"] = &createInstance<ImageNote>;
    this->ClassNameRegistry["ImageNote"] = typeid(ImageNote).name();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application::~Application()
  {
    this->LogStream.close();

    if (NULL != this->Config)
    {
      this->Config->Delete();
      this->Config = NULL;
    }

    if (NULL != this->DB)
    {
      this->DB->Delete();
      this->DB = NULL;
    }

    if (NULL != this->Opal)
    {
      this->Opal->Delete();
      this->Opal = NULL;
    }

    if (NULL != this->ActiveUser)
    {
      this->ActiveUser->Delete();
      this->ActiveUser = NULL;
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  // this must be implemented instead of the standard new macro since the New()
  // method is protected (ie: we do not want an instantiator new function)
  Application* Application::New()
  {
    vtkObject* ret = vtkObjectFactory::CreateInstance("Application");
    return ret ? static_cast< Application* >(ret) : new Application;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Application* Application::GetInstance()
  {
    if (NULL == Application::Instance) Application::Instance = Application::New();
    return Application::Instance;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::DeleteInstance()
  {
    if (NULL != Application::Instance)
    {
      Application::Instance->Delete();
      Application::Instance = NULL;
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::OpenLogFile()
  {
    std::string logPath = this->Config->GetValue("Path", "Log");
    this->LogStream.open(logPath, std::ofstream::out | std::ofstream::app);
    return this->LogStream.is_open();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::Log(const std::string &message)
  {
    this->LogStream << "[" << Utilities::getTime("%y-%m-%d %T") << "] " << message << std::endl;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::LogBacktrace()
  {
    int status;
    Dl_info dlinfo;
    void *trace[ALDER_STACK_DEPTH];
    int trace_size = backtrace(trace, ALDER_STACK_DEPTH);

    for (int i = 0; i < trace_size; ++i)
    {
      if (!dladdr(trace[i], &dlinfo)) continue;

      const char* symname = dlinfo.dli_sname;
      char* demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
      if (0 == status && NULL != demangled) symname = demangled;
      this->LogStream << dlinfo.dli_fname << "::" << symname << std::endl;

      if (demangled) free(demangled);
    }
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string Application::GetUnmangledClassName(const std::string &mangledName) const
  {
    for (auto it = this->ClassNameRegistry.begin(); it != this->ClassNameRegistry.end(); ++it)
      if (mangledName == it->second) return it->first;

    throw std::runtime_error("Tried to unmangle class name which isn't registered.");
    return "";  // this will never happen because of the throw
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ReadConfiguration(const std::string &fileName)
  {
    // make sure the file exists
    ifstream ifile(fileName.c_str());
    if (!ifile) return false;
    return this->Config->Read(fileName);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::ConnectToDatabase()
  {
    std::string name = this->Config->GetValue("Database", "Name");
    std::string user = this->Config->GetValue("Database", "Username");
    std::string pass = this->Config->GetValue("Database", "Password");
    std::string host = this->Config->GetValue("Database", "Host");
    std::string port = this->Config->GetValue("Database", "Port");

    // make sure the database and user names are provided
    if (0 == name.size() || 0 == user.size())
    {
      cerr << "ERROR: database name and database user name must be included in "
           << "configuration file" << endl;
      return false;
    }

    // default host and port
    if (0 == host.size()) host = "localhost";
    if (0 == port.size()) port = "3306";

    return this->DB->Connect(name, user, pass, host, vtkVariant(port).ToInt());
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  bool Application::TestImageDataPath()
  {
    std::string imageDataPath = this->Config->GetValue("Path", "ImageData");
    return vtkDirectory::MakeDirectory(imageDataPath.c_str());
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetupOpalService()
  {
    // setup the opal service
    std::string user = this->Config->GetValue("Opal", "Username");
    std::string pass = this->Config->GetValue("Opal", "Password");
    std::string host = this->Config->GetValue("Opal", "Host");
    std::string port = this->Config->GetValue("Opal", "Port");
    std::string timeout = this->Config->GetValue("Opal", "Timeout");
    this->Opal->Setup(user, pass, host);
    if (0 < port.size()) this->Opal->SetPort(vtkVariant(port).ToInt());
    if (0 < timeout.size()) this->Opal->SetTimeout(vtkVariant(timeout).ToInt());
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::UpdateDatabase()
  {
    std::string waveSource = this->Config->GetValue("Opal", "WaveSource");
    Wave::UpdateWaveData(waveSource);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::ResetApplication()
  {
    this->SetActiveUser(NULL);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void Application::SetActiveUser(User *user)
  {
    if (user != this->ActiveUser)
    {
      if (this->ActiveUser)
      {
        this->ActiveUser->UnRegister(this);
      }
      this->ActiveUser = user;
      if (this->ActiveUser)
      {
        this->ActiveUser->Register(this);
      }
      this->Modified();
      this->InvokeEvent(Common::UserChangedEvent);
    }
  }
}  // namespace Alder
