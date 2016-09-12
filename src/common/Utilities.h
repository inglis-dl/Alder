/*=========================================================================

  Program:  Alder (CLSA Medical Image Quality Assessment Tool)
  Module:   Utilities.h
  Language: C++

  Author: Patrick Emond <emondpd AT mcmaster DOT ca>
  Author: Dean Inglis <inglisd AT mcmaster DOT ca>

=========================================================================*/

/**
 * @class Utilities
 *
 * @author Patrick Emond <emondpd AT mcmaster DOT ca>
 * @author Dean Inglis <inglisd AT mcmaster DOT ca>
 *
 * @brief Utilities class includes typedefs, macros, global functions, etc.
 *
 * All methods in this class are static.  There is no need to instantiate
 * and instance of the Utilities class.  The class implementation file
 * exists solely for the creation of a library.
 *
 */
#ifndef __Utilities_h
#define __Utilities_h

// C includes
#include <base64.h>
#include <sha.h>
#include <sys/stat.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

// C++ includes
#include <algorithm>
#include <cctype>
#include <fstream>
#include <functional>
#include <string>
#include <sstream>
#include <vector>

// Alder includes
#include <AlderConfig.h>

/**
 * @addtogroup Alder
 * @{
 */

namespace Alder
{
  class Utilities
  {
  public:
    /**
     *  Encrypt a string with SHA256 hash algorithm
     */
    inline static void hashString(std::string input, std::string& output)
    {
      input += ALDER_SALT_STRING;
      output = "";

      CryptoPP::SHA256 hash;
      CryptoPP::StringSource foo(
        input.c_str(),
        true,
        new CryptoPP::HashFilter(hash,
          new CryptoPP::Base64Encoder(
            new CryptoPP::StringSink(output))));
    }

    /**
     *  Base64 encode a string
     */
    inline static void base64String(std::string input, std::string& output)
    {
      output = "";

      CryptoPP::StringSource foo(
        input.c_str(),
        true,
        new CryptoPP::Base64Encoder(
          new CryptoPP::StringSink(output)));
    }

    /**
     *  Write data to a FILE stream via pointer access
     */
    inline static size_t writePointerToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
    {
      size_t written = fwrite(ptr, size, nmemb, stream);
      return written;
    }

    /**
     *  Append data to a string via pointer access
     */
    inline static size_t writePointerToString(void *ptr, size_t size, size_t count, void *stream)
    {
      ((std::string*)stream)->append(reinterpret_cast<char*>(ptr), 0, size * count);
      return size * count;
    }

    /**
     *  Execute a command using popen/pclose
     */
    inline static std::string exec(std::string const& command)
    {
      FILE* pipe = popen(command.c_str(), "r");
      if (!pipe) return "ERROR";
      char buffer[128];
      std::string result = "";
      while (!feof(pipe)) if (fgets(buffer, 128, pipe) != NULL) result += buffer;
      pclose(pipe);
      return result;
    }

    /**
     *  Gets the time in a specified format using strftime
     */
    inline static std::string getTime(std::string const& format)
    {
      char buffer[256];
      time_t rawtime;
      time(&rawtime);
      strftime(buffer, 256, format.c_str(), localtime(&rawtime));
      return std::string(buffer);
    }

    /**
     *  Converts a string to lowercase
     */
    inline static std::string toLower(std::string const& str)
    {
      std::string returnString = str;
      std::transform(str.begin(), str.end(), returnString.begin(), tolower);
      return returnString;
    }

    /**
     *  Converts a string to uppercase
     */
    inline static std::string toUpper(std::string const& str)
    {
      std::string returnString = str;
      std::transform(str.begin(), str.end(), returnString.begin(), toupper);
      return returnString;
    }

    /**
     *  Removes leading and trailing occurances of a specified char (default is space)
     */
    inline static std::string removeLeadingTrailing(std::string const& str,
      const char ch = ' ')
    {
      std::string result(str);
      std::string::iterator first = result.begin(), last = result.end();
      while (first != last && *first == ch) first++;
      if (first != result.begin()) result.erase(result.begin(), first);
      first = result.begin();
      last = result.end();
      if (first != last)
      {
        last--;
        while (first != last && *last == ch) last--;
        if (*last != ch) last++;
      }
      if (last != result.end()) result.erase(last, result.end());
      return result;
    }

    /**
     *  Tests if a file exists
     */
    inline static bool fileExists(std::string const& filename)
    {
      if (filename.empty()) return false;
      return 0 == access(filename.c_str(), R_OK);
    }

    /**
     * Gets the last . separated portion of a file name
     */
    inline static std::string getFileExtension(std::string const& filename)
    {
      std::string::size_type dot_pos = filename.rfind(".");
      std::string extension = (dot_pos == std::string::npos) ? "" :
        filename.substr(dot_pos);
      return extension;
    }

    /**
     * Gets the path portion of a file name
     */
    inline static std::string getFilenamePath(std::string const& filename)
    {
      std::string::size_type slash_pos = filename.rfind("/");
      if (slash_pos != std::string::npos)
      {
        std::string path = filename.substr(0, slash_pos);
        if (2 == path.size() && ':' == path[1])
        {
          return path + '/';
        }
        if (0 == path.size())
        {
          return "/";
        }
        return path;
      }
      else
      {
        return "";
      }
    }

    /**
     * Gets the size in bytes of a file
     */
    inline static unsigned long int getFileLength(std::string const& filename)
    {
      struct stat fs;
      return 0 != stat(filename.c_str(), &fs) ? 0 : static_cast<unsigned long int>(fs.st_size);
    }

    /**
     * Gets the last / separated part of a unix file name
     */
    inline static std::string getFilenameName(std::string filename)
    {
      std::string::size_type slash_pos = filename.find_last_of("/");
      if (slash_pos != std::string::npos)
      {
        return filename.substr(slash_pos + 1);
      }
      else
      {
        return filename;
      }
    }

    /**
     * Divides a string by the provided separator, returning the results as a vector of strings
     */
    inline static std::vector<std::string> explode(std::string str, std::string separator)
    {
      std::vector<std::string> results;
      int found = str.find_first_of(separator);
      while (found != std::string::npos)
      {
        if (0 < found) results.push_back(str.substr(0, found));
        str = str.substr(found + 1);
        found = str.find_first_of(separator);
      }
      if (0 < str.size()) results.push_back(str);
      return results;
    }

    /**
     * Concatenates a vector of strings by the provided separator, returning the results as string
     */
    inline static std::string implode(const std::vector<std::string>& vec,
      const std::string& separator)
    {
      std::string result;
      for (std::vector<std::string>::const_iterator it = vec.begin(); it != vec.end(); ++it)
      {
        result += *it;
        if (1 + it != vec.end()) result += separator;
      }
      return result;
    }

    /**
     * Removes all space characters (as defined by std::isspace) from the left side of a string
     */
    inline static std::string& ltrim(std::string& s)
    {
      s.erase(
        s.begin(), std::find_if(
          s.begin(), s.end(), std::not1(
            std::ptr_fun<int, int>(std::isspace))));
      return s;
    }

    /**
     * Removes all space characters (as defined by std::isspace) from the right side of a string
     */
    inline static std::string& rtrim(std::string& s)
    {
      s.erase(
        std::find_if(
          s.rbegin(), s.rend(), std::not1(
            std::ptr_fun<int, int>(std::isspace))).base(), s.end());
      return s;
    }

    /**
     * Removes all space characters (as defined by std::isspace) from both sides of a string
     */
    inline static std::string& trim(std::string& s)
    {
      return ltrim(rtrim(s));
    }

    /**
     * Retain n characters from the left side of a string
     */
    inline static void left(std::string& s, const int& n)
    {
      if (0 > n) return;
      size_t pos = n;
      s.erase(pos, std::string::npos);
    }

    /**
     * Retain n characters from the right side of a string
     */
    inline static void right(std::string& s, const int& n)
    {
      if (0 > n ) return;
      size_t pos = 0;
      size_t len = s.size();
      len = n >= len ? std::string::npos : len - n;
      s.erase(pos, len);
    }

  protected:
    Utilities() {}
    ~Utilities() {}
  };
}  // namespace Alder

#endif  // __Utilities_h
