/*
 * Logger.cpp
 *
 *  Created on: 28.01.2010
 *      Author: adamczew
 */

#include "Logger.h"

namespace pexor {

pexor::Logger* Logger::fInstance=0;
//char pexor::Logger::fMessagetext[PEXOR_LOGGER_BUFLEN];

Logger::Logger() :
		fOutputEnabled(true), fIgnoreLevel(MSG_INFO)
{

}

Logger::~Logger()
{

}

pexor::Logger* Logger::Instance()
{
    if(fInstance == 0)
       fInstance = new pexor::Logger();

    return fInstance;
}


const char* Logger::Message(pexor::MessageLevel prio, const char* text,...)
{
if(prio<fIgnoreLevel) return 0;
char txtbuf[PEXOR_LOGGER_BUFLEN];
va_list args;
va_start(args, text);
vsnprintf(txtbuf, PEXOR_LOGGER_BUFLEN, text, args);
va_end(args);
const char* prefix(PEXOR_LOGGER_INFO);
switch(prio) {
       case  MSG_DEBUG: prefix=PEXOR_LOGGER_DEBUG; break;
       case  MSG_INFO: prefix=PEXOR_LOGGER_INFO;  break;
       case  MSG_WARN: prefix=PEXOR_LOGGER_WARN;  break;
       case  MSG_ERR: prefix=PEXOR_LOGGER_ERR;   break;
  } // switch()

/* TODO: message output to file
if(fgbLogfileEnabled) {
       // message format for logfile is different:
       snprintf(fMessagetext,PEXOR_LOGGER_BUFLEN,"%s %s",prefix,txtbuf);
       WriteLogfile(fMessagetext);
    }
*/

    // we compose the full messagetext anyway, for further use outside
     snprintf(fMessagetext,PEXOR_LOGGER_BUFLEN, "%s%s> %s %s",
    		 PEXOR_LOGGER_PRE, prefix, txtbuf,PEXOR_LOGGER_END);

    if(fOutputEnabled) {
      std::cout << fMessagetext << std::endl;
   }

    return fMessagetext;

}

}
