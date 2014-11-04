#include <base64.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include </usr/include/curl/curl.h>
#include </usr/include/curl/stdcheaders.h>
#include </usr/include/curl/easy.h>

using namespace std;

void base64String( string input, string &output )
{
  output = "";

  CryptoPP::StringSource foo(
    input.c_str(),
    true,
    new CryptoPP::Base64Encoder(
      new CryptoPP::StringSink( output ) ) );
}


size_t writeToFile(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
  size_t written = fwrite( ptr, size, nmemb, stream );
  return written;
}

size_t writeToString(void *ptr, size_t size, size_t count, void *stream)
{
  ( (std::string*) stream )->append( (char*) ptr, 0, size * count );
  return size * count;
}

int main( int argc, char** argv )
{
  if( !( 6 <= argc && argc <= 7 ) )
  {
    std::cout << "Usage: OpalHttp <username> <password> <host> <port> <ws> [outfile]" << endl
              << "Example:" << endl
              << "  OpalHttp username localhost 8843 <ws> output.jpg" << endl
              << "  Where <ws> can be any of the following:" << endl
              << "    /datasources" << endl
              << "      All datasources" << endl
              << endl
              << "    /datasource/xxx" << endl
              << "      A datasource" << endl
              << endl
              << "    /datasource/xxx/tables" << endl
              << "      All tables of a datasource" << endl
              << endl
              << "    /datasource/xxx/table/yyy" << endl
              << "      A table" << endl
              << endl
              << "    /datasource/xxx/table/yyy/variables" << endl
              << "      All variables of a table" << endl
              << endl
              << "    /datasource/xxx/table/yyy/variable/vvv" << endl
              << "      A variable" << endl
              << endl
              << "    /datasource/xxx/table/yyy/entities" << endl
              << "      All entities of a table" << endl
              << endl
              << "    /datasource/xxx/table/yyy/entities?script=sss" << endl
              << "      All entities of a table matching a script" << endl
              << "      (see http://wiki.obiba.org/display/OPALDOC/Magma+Javascript+API)" << endl
              << endl
              << "    /datasource/xxx/table/yyy/valueSet/zzz" << endl
              << "      All values of a entity in a table" << endl
              << endl
              << "    /datasource/xxx/table/yyy/valueSet/zzz/variable/vvv" << endl
              << "      A variable value of a entity" << endl
              << endl
              << "    /datasource/xxx/table/yyy/valueSet/zzz/variable/vvv/value" << endl
              << "      Raw variable value of a entity" << endl
              << endl
              << "    /datasource/xxx/table/yyy/valueSet/zzz/variable/vvv/value?pos=1" << endl
              << "      Raw repeatable variable value of a entity at given position (start at 0)" << endl;
    return 1;
  }

  CURL *curl;
  FILE *fp;
  string output;
  CURLcode res;

  // get the arguments
  string username = argv[1];
  string password = argv[2];
  string host = argv[3];
  string port = argv[4];
  string ws = argv[5];
  string outfile = "";
  bool toFile = 7 == argc;
  if( toFile ) outfile = argv[6];

  // encode the credentials
  string credentials = "";
  base64String( username + ":" + password, credentials );
  credentials = "Authorization:X-Opal-Auth " + credentials;

  string url = "https://" + host + ":" + port + "/ws" + ws;
  curl = curl_easy_init();
  if (curl)
  {
    struct curl_slist *headers = NULL;
    headers = curl_slist_append( headers, "Accept: application/json" );
    headers = curl_slist_append( headers, credentials.c_str() );

    if( toFile ) fp = fopen( outfile.c_str(), "wb" );
    curl_easy_setopt( curl, CURLOPT_URL, url.c_str() );

    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writeToFile );
    else curl_easy_setopt( curl, CURLOPT_WRITEFUNCTION, writeToString );
    
    if( toFile ) curl_easy_setopt( curl, CURLOPT_WRITEDATA, fp );
    else curl_easy_setopt( curl, CURLOPT_WRITEDATA, &output );

    curl_easy_setopt( curl, CURLOPT_TIMEOUT, 10 );
    curl_easy_setopt( curl, CURLOPT_SSL_VERIFYPEER, 0 );
    curl_easy_setopt( curl, CURLOPT_HTTPHEADER, headers );
    res = curl_easy_perform( curl );
    if( 0 != res ) cout << "Error (Code " << res << "): " << curl_easy_strerror( res ) << endl;
    else
    {
      if( toFile ) cout << "Success, result written to \"" << outfile << "\"" << endl;
      else cout << output << endl;
    }

    /* always cleanup */
    curl_slist_free_all( headers );
    curl_easy_cleanup( curl );
    if( toFile ) fclose( fp );
  }
  return 0;
}
