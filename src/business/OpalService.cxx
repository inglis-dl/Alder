/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   OpalService.cxx
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/
#include <OpalService.h>

// Alder includes
#include <Application.h>
#include <Configuration.h>
#include <Utilities.h>

// VTK includes
#include <vtkCommand.h>
#include <vtkObjectFactory.h>

// curl includes
#include </usr/include/curl/easy.h>

// C++ includes
#include <algorithm>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace Alder
{
  vtkStandardNewMacro(OpalService);

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  OpalService::OpalService()
  {
    this->Username = "";
    this->Password = "";
    this->Host = "localhost";
    this->Port = 8843;
    this->Timeout = 10;
    this->Verbose = 0;
    this->SustainConnection = 0;
    this->CurlConnection = NULL;
    this->CurlHeaders = NULL;
    this->CurlCredentials = "";
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  OpalService::~OpalService()
  {
    if (NULL != this->CurlHeaders)
    {
      curl_slist_free_all(this->CurlHeaders);
    }
    curl_global_cleanup();
    this->CurlConnection = NULL;
    this->CurlHeaders = NULL;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::Setup(
    const std::string &username,
    const std::string &password,
    const std::string &host,
    const int &port,
    const int &timeout,
    const int &verbose)
  {
    this->Username = username;
    this->Password = password;
    this->Host = host;
    this->Port = port;
    this->Timeout = timeout;
    this->Verbose = verbose;
    Application *app = Application::GetInstance();
    app->Log(
      "Setup Opal service using cURL version: " + std::string(curl_version()));
    curl_global_init(CURL_GLOBAL_SSL);
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::SetSustainConnection(int sustain)
  {
    if (sustain == this->SustainConnection) return;

    if (0 == sustain)
    {
      if (NULL != this->CurlHeaders)
        curl_slist_free_all(this->CurlHeaders);
      if (NULL != this->CurlConnection)
        curl_easy_cleanup(this->CurlConnection);
      this->CurlCredentials.clear();
      this->CurlHeaders = NULL;
      this->CurlConnection = NULL;
    }
    else
    {
      // encode the credentials
      this->CurlCredentials.clear();
      Utilities::base64String(
        this->Username + ":" + this->Password, this->CurlCredentials);
      this->CurlCredentials =
        "Authorization:X-Opal-Auth " + this->CurlCredentials;

      this->CurlConnection = curl_easy_init();
      if (!this->CurlConnection)
        throw std::runtime_error("Unable to create cURL connection to Opal");

      // put the credentials in a header
      // and the option to return data in json format
      if (NULL != this->CurlHeaders)
      {
        curl_slist_free_all(this->CurlHeaders);
        this->CurlHeaders = NULL;
      }
      this->CurlHeaders =
        curl_slist_append(this->CurlHeaders, "Accept: application/json");
      this->CurlHeaders =
        curl_slist_append(this->CurlHeaders, this->CurlCredentials.c_str());

      curl_easy_setopt(
        this->CurlConnection, CURLOPT_VERBOSE, this->Verbose);
      curl_easy_setopt(
        this->CurlConnection, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(
        this->CurlConnection, CURLOPT_HTTPHEADER, this->CurlHeaders);
    }

    this->SustainConnection = sustain;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  Json::Value OpalService::Read(
    const std::string &servicePath, const std::string &fileName) const
  {
    bool toFile = !fileName.empty();
    FILE *file;
    CURL *curl = NULL;
    std::stringstream urlStream;
    std::string credentials, url, result;
    struct curl_slist *headers = NULL;
    CURLcode res = CURLE_OK;
    Json::Value root;
    Json::Reader reader;
    Application *app = Application::GetInstance();

    urlStream << "https://" << this->Host << ":"
              << this->Port << "/ws" + servicePath;
    url = urlStream.str();
    app->Log("Querying Opal: " + url);

    if (!this->SustainConnection)
    {
      curl = curl_easy_init();
      if (!curl)
        throw std::runtime_error("Unable to create cURL connection to Opal");

      // encode the credentials
      Utilities::base64String(
        this->Username + ":" + this->Password, credentials);
      credentials = "Authorization:X-Opal-Auth " + credentials;

      // put the credentials in a header
      // and the option to return data in json format
      headers = curl_slist_append(headers, "Accept: application/json");
      headers = curl_slist_append(headers, credentials.c_str());

      curl_easy_setopt(curl, CURLOPT_VERBOSE, this->Verbose);
      curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
      curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    }
    else
    {
      curl = this->CurlConnection;
    }

    // if we are writing to a file, open it
    if (toFile)
    {
      file = fopen(fileName.c_str(), "wb");

      if (NULL == file)
      {
        std::stringstream stream;
        stream << "Unable to open file \""
               << fileName << "\" for writing." << endl;
        throw std::runtime_error(stream.str().c_str());
      }
      curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToFile);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    }
    else
    {
      curl_easy_setopt(
        curl, CURLOPT_WRITEFUNCTION, Utilities::writePointerToString);
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    res = curl_easy_perform(curl);

    if (toFile) fclose(file);

    // clean up
    if (!this->SustainConnection)
    {
      curl_slist_free_all(headers);
      curl_easy_cleanup(curl);
    }

    if (CURLE_OK != res)
    {
      // don't display abort errors (code 42) when the user initiated the abort
      if (!(CURLE_ABORTED_BY_CALLBACK == res && app->GetAbortFlag()))
      {
        std::stringstream stream;
        stream << "Received cURL error "
               << res
               << " when attempting to contact Opal: "
               << curl_easy_strerror(res);
        throw std::runtime_error(stream.str().c_str());
      }
    }

    if (!toFile)
    {
      if (0 == result.size())
        throw std::runtime_error("Empty response from Opal service");
      else if (!reader.parse(result.c_str(), root))
        throw std::runtime_error("Unable to parse result from Opal service");
    }

    return root;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> OpalService::GetIdentifiers(
    const std::string &dataSource, const std::string &table) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table << "/entities";
    Json::Value root = this->Read(stream.str());
    try
    {
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    std::vector<std::string> list;
    for (int i = 0; i < root.size(); ++i)
    {
      std::string identifier = root[i].get("identifier", "").asString();
      if (0 < identifier.size()) list.push_back(identifier);
    }

    // Opal doesn't sort results, do so now
    std::sort(list.begin(), list.end());
    return list;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<std::string, std::map<std::string, std::string>> OpalService::GetRows(
    const std::string &dataSource, const std::string &table,
    const int &offset, const int &limit) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSets?offset=" << offset << "&limit=" << limit;
    Json::Value root;
    try
    {
      root = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    std::map<std::string, std::map<std::string, std::string>> list;
    std::string identifier, key, value;
    for (int i = 0; i < root["valueSets"].size(); ++i)
    {
      identifier = root["valueSets"][i].get("identifier", "").asString();

      if (0 < identifier.size())
      {
        std::map<std::string, std::string> map;
        for (int j = 0; j < root["valueSets"][i]["values"].size(); ++j)
        {
          key = root["variables"][j].asString();
          value = root["valueSets"][i]["values"][j].get("value", "").asString();
          map[key] = value;
        }
        list[identifier] = map;
      }
    }

    return list;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<std::string, std::string> OpalService::GetRow(
    const std::string &dataSource, const std::string &table,
    const std::string &identifier) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier;
    Json::Value root;
    try
    {
      root = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    std::map<std::string, std::string> map;
    std::string key, value;
    if (0 < root["valueSets"][0].get("identifier", "").asString().size())
    {
      for (int j = 0; j < root["valueSets"][0]["values"].size(); ++j)
      {
        key = root["variables"][j].asString();
        value = root["valueSets"][0]["values"][j].get("value", "").asString();
        map[key] = value;
      }
    }

    return map;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::map<std::string, std::string> OpalService::GetColumn(
    const std::string &dataSource, const std::string &table,
    const std::string &variable, const int &offset, const int &limit)
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSets?offset=" << offset << "&limit=" << limit
           << "&select=name().eq('" << variable << "')";
    Json::Value root;
    try
    {
      root = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    std::map<std::string, std::string> map;
    std::string identifier, value;
    for (int i = 0; i < root["valueSets"].size(); ++i)
    {
      identifier = root["valueSets"][i].get("identifier", "").asString();

      if (0 < identifier.size())
      {
        value = root["valueSets"][i]["values"][0].get("value", "").asString();
        map[identifier] = value;
      }
    }

    return map;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::string OpalService::GetValue(
    const std::string &dataSource, const std::string &table,
    const std::string &identifier, const std::string &variable) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier << "/variable/" << variable;
    Json::Value root;
    try
    {
      root = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    return root.get("value", "").asString();
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> OpalService::GetValues(
    const std::string &dataSource, const std::string &table,
    const std::string &identifier, const std::string &variable) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier << "/variable/" << variable;
    Json::Value root;
    try
    {
      root = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    // loop through the values array and get all the values
    Json::Value values = root.get("values", "");
    std::vector<std::string> retValues;
    for (int i = 0; i < values.size(); ++i)
      retValues.push_back(values[i].get("value", "").asString());

    return retValues;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> OpalService::GetVariables(
    const std::string &dataSource, const std::string &table) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/variables";
    Json::Value values;
    try
    {
      values = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    // loop through the values array and get all the values
    std::vector<std::string> retValues;
    for (int i = 0; i < values.size(); ++i)
      retValues.push_back(values[i].get("name", "").asString());

    return retValues;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  std::vector<std::string> OpalService::GetTables(
    const std::string &dataSource) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/tables";
    Json::Value values;
    try
    {
      values = this->Read(stream.str());
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }

    // loop through the values array and get all the values
    std::vector<std::string> retValues;
    for (int i = 0; i < values.size(); ++i)
      retValues.push_back(values[i].get("name", "").asString());

    return retValues;
  }

  // -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
  void OpalService::SaveFile(
    const std::string &fileName,
    const std::string &dataSource,
    const std::string &table,
    const std::string &identifier,
    const std::string &variable,
    const int &position) const
  {
    std::stringstream stream;
    stream << "/datasource/" << dataSource << "/table/" << table
           << "/valueSet/" << identifier
           << "/variable/" << variable << "/value";

    // add on the position
    if (0 <= position) stream << "?pos=" << position;

    try
    {
      this->Read(stream.str(), fileName);
    }
    catch(std::runtime_error &e)
    {
      throw e;
    }
  }
}  // namespace Alder
