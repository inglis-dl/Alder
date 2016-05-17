// C includes
#include <stdio.h>
#include <signal.h>
#include <execinfo.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <stdlib.h>

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void c()
{
  enum
  {
    MAX_DEPTH = 10
  };

  void* trace[MAX_DEPTH];
  Dl_info dlinfo;
  int status;
  const char* symname;
  char* demangled;

  int trace_size = backtrace(trace, MAX_DEPTH);
  printf("Call stack: \n");
  for (int i = 0; i < trace_size; ++i)
  {
    if (!dladdr(trace[i], &dlinfo))
        continue;

    symname = dlinfo.dli_sname;
    demangled = abi::__cxa_demangle(symname, NULL, 0, &status);
    if (status == 0 && demangled)
        symname = demangled;

    printf("object: %s, function: %s\n", dlinfo.dli_fname, symname);

    if (demangled)
      free(demangled);
  }
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void b()
{
  c();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
void a()
{
  b();
}

// -+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-+#+-
int main(int argc, char** argv)
{
  a();
  return 0;
}
