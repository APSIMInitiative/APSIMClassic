//---------------------------------------------------------------------------
// The windows build of this file is built with gcc. It has no apsim infrastructure calls. We cannot
// pass c++ classes as arguments - the name mangling and memory managers are incompatible.
// It requires RTools (from CRAN) to compile.

#include <iostream>
#include <unistd.h>
#include <RInside.h>
#include <General/platform.h>
#include "REmbedder.h"
#include "RDataTypes.h"
#include "RDataTypesEmbedded.h"
void apsimFatalError( std::string message )
  {
  apsimFatalErrorRaw( message.c_str());
  }

void apsimSubscribe( std::string eventName, std::string type, std::string handler )
  {
  apsimSubscribeRaw( eventName.c_str(), type.c_str(), handler.c_str() );
  }

void apsimExpose( std::string varName, std::string units, SEXP value)
  {
  Rcpp::Function f = Rcpp::Function("class");
  std::string className = Rcpp::as<std::string>( f(value));
  Rcpp::Function g = Rcpp::Function("length");
  int argLength = Rcpp::as<int>( g(value));

  if (className == "numeric" )
  	if (	argLength> 1) { className = "doubleArray";} else {className = "double";}
  else if (className == "character")
  	if (	argLength> 1) { className = "stringArray";} else {className = "string";}
  else if (argLength > 1)
  	className += std::string("[") + static_cast<std::ostringstream*>( &(std::ostringstream() << argLength) )->str() + "]"; // Let the other end take care of it..
  
  apsimExposeRaw( varName.c_str(), units.c_str(), className.c_str());
  }

SEXP apsimQuery( std::string pattern )
  {
   SEXP result = Rcpp::wrap(R_NilValue);

   int numReturned = 0;
   char *returned = NULL;
   apsimQueryRaw(pattern.c_str(),&numReturned, (void**)&returned);

   char *buf = returned;
   std::vector<std::string> v;
   for (int i = 0; i < numReturned; i++) { v.push_back(buf); buf += strlen(buf)+1; buf += strlen(buf)+1; }
   if (returned) apsimFree(returned);

   result = Rcpp::wrap(v);
   return(result);
}

RCPP_MODULE(apsim){
  using namespace Rcpp ;
  function( "publish",  &apsimPublish, List::create( _["eventName"], _["type"], _["value"]),     "Publish an apsim event" ) ;
  function( "subscribe",&apsimSubscribe, List::create( _["eventName"], _["type"], _["handler"]), "Subscribe to an apsim event" ) ;
  function( "get",      &apsimGet, List::create( _["variableName"]),                       "Get an apsim variable" ) ;
  function( "set",      &apsimSet, List::create( _["variableName"], _["variableValue"] ),  "Set an apsim variable" ) ;
  function( "expose",   &apsimExpose, List::create( _["variableName"] , _["units"] = "", _["value"] = 0 ), "Expose an R variable to apsim" ) ;
  function( "query",    &apsimQuery, List::create( _["pattern"] ),                         "Query apsim for a module or variable" ) ;
  function( "fatal",    &apsimFatalError, List::create( _["message"] ),                    "Signal a fatal apsim error" ) ;
}

///////////////////START
RInside *ptrR = NULL;
void * DLLEXPORT RLinkDLLHandle = NULL;

// These "Embedded_xxxx" routines are called via dlsym() from the apsim component
// Return false on error

extern "C" bool DLLEXPORT EmbeddedR_Start(void *handle, const char *R_Home, const char *UserLibs)
{
   int argc = 0;
   char **argv = NULL;
   RLinkDLLHandle = handle;
     
#ifdef WIN32
   std::string newPath = getenv("PATH");
   std::replace(newPath.begin(), newPath.end(), '\\', '/');
   newPath = std::string(R_Home) + "/bin/i386;" + newPath;
   setenv("PATH", newPath.c_str(), 1);

   char oldWD [8192];
   getcwd(oldWD, 8192);
   std::string newWD = std::string(R_Home) + "/bin/i386";
   chdir(newWD.c_str());
#endif

   if (R_Home != NULL && strlen(R_Home) > 0) { setenv("R_HOME", R_Home, 1) ; }
   if (UserLibs != NULL && strlen(UserLibs) > 0) { setenv("R_LIBS_USER", UserLibs, 1); }
   // TMPDIR??
   // R_USER??

   try {
     ptrR = new RInside(argc, argv, true);
   } catch (std::exception& ex) {
        std::cerr << "R Exception: " << ex.what() << std::endl; std::cerr.flush();
		return false;
   }

   try {
     ptrR->parseEval("library(\"methods\")");
     (*ptrR)["apsim"] = Rf_eval( Rf_lang2( GET_MODULE_SYM, _rcpp_module_boot_apsim() ), R_GlobalEnv ); // Workaround
     //(*ptrR)["apsim"] = LOAD_RCPP_MODULE(apsim) ;
   } catch (std::exception& ex) {
        std::cerr << "R Exception: " << ex.what() << std::endl; std::cerr.flush();
        std::string msg = ptrR->parseEval("geterrmessage()");
        std::cerr << msg << std::endl; std::cerr.flush();
		return false;
   }

   ptrR->parseEval("apsimPublish<-apsim$publish");
   ptrR->parseEval("apsimSubscribe<-apsim$subscribe");
   ptrR->parseEval("apsimGet<-apsim$get");
   ptrR->parseEval("apsimSet<-apsim$set");
   ptrR->parseEval("apsimExpose<-apsim$expose");
   ptrR->parseEval("apsimFatal<-apsim$fatal");

#ifdef WIN32
   chdir(oldWD);
#endif

   return true;
}

extern "C" bool DLLEXPORT EmbeddedR_Stop(void)
{
  if (ptrR == NULL)
     return false;

  delete ptrR;

  ptrR = NULL;
  return true;
}

extern "C" bool DLLEXPORT EmbeddedR_Eval(const char *cmd)
{
  if (ptrR == NULL)
     return false;

  ptrR->parseEval(cmd);
  return true;
}

extern "C" bool DLLEXPORT EmbeddedR_GetVector(const char *variable, char *result,
                                    unsigned int resultlen, unsigned int elemwidth,
                                    unsigned int *numReturned)
{
  *numReturned = 0;
  if (ptrR == NULL)
     return false;
  try
    {
    Rcpp::CharacterVector resultVec = (*ptrR)[variable];
    char *ptr = result;
    for (int i = 0; i < resultVec.size() && ptr  < result + resultlen; i++)
        {
        const char *stringrep = resultVec[i];
        strncpy(ptr, stringrep, elemwidth);
        ptr[elemwidth-1] = '\0';
        ptr += elemwidth;
        (*numReturned)++;
        }
    return true;
    }
  catch(std::exception& ex) {
    std::cout << "R Exception getting " << variable << ":" << ex.what() << std::endl;
    std::string msg = ptrR->parseEval("geterrmessage()");
    std::cout << msg << std::endl; std::cout.flush();
  }
  return false;
}

extern "C" bool DLLEXPORT EmbeddedR_GetVectorDouble(const char *variable, double *result,
                                    unsigned int resultlen, unsigned int *numReturned)
{
  *numReturned = 0;
  if (ptrR == NULL)
     return false;
  try
    {
    Rcpp::NumericVector resultVec = (*ptrR)[variable];
    double *ptr = result;
    for (int i = 0; i < resultVec.size() && ptr  < result + sizeof(double) * resultlen; i++, ptr++)
        {
        *ptr = resultVec[i];
        (*numReturned)++;
        }
    return true;
    }
  catch(std::exception& ex) {
    std::cout << "R Exception getting " << variable << ":" << ex.what() << std::endl;
    std::string msg = ptrR->parseEval("geterrmessage()");
    std::cout << msg << std::endl; std::cout.flush();
  }
  return false;
}

extern "C" bool DLLEXPORT EmbeddedR_SimpleCharEval(const char *cmd, char *buf, int buflen)
{
  buf[0] = '\0';
  if (ptrR == NULL)
     return false;

  std::string result = ptrR->parseEval(cmd);
  strncpy(buf, result.c_str(), buflen);
  return true;
}

#ifndef WIN32

void RGetVector(const char *variable, std::vector<std::string> &result)
  {
  result.clear();
  if (ptrR != NULL)
     {
     Rcpp::CharacterVector resultVec = (*ptrR)[variable];
     for (int i = 0; i < resultVec.size(); i++)
       result.push_back(std::string(resultVec[i]));
     }
  }

std::string SimpleREval(const char *s)
  {
  if (ptrR != NULL)
     {
     std::string result = ptrR->parseEval(s);
     return result;
     }
  return "";
  }

#endif
