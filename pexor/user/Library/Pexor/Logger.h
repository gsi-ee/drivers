/*
 * Logger.h
 *
 *  Created on: 28.01.2010
 *      Author: adamczew
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <iostream>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sstream>
#include <errno.h>


#define PEXOR_LOGGER_BUFLEN 1024
#define PEXOR_LOGGER_PRE "PEXOR:"
#define PEXOR_LOGGER_INFO "-I-"
#define PEXOR_LOGGER_WARN "-W-"
#define PEXOR_LOGGER_ERR "-E-"
#define PEXOR_LOGGER_DEBUG "-D-"
#define PEXOR_LOGGER_END " ."


#define PexorPrint( args... ) \
	pexor::Logger::Instance()->Message( args )

#define SetPexorPrintLevel(X) \
	pexor::Logger::Instance()->SetMessageLevel(X)

#define PexorInfo(args...) \
	pexor::Logger::Instance()->Message(MSG_INFO, args )

#define PexorWarning(args...) \
	pexor::Logger::Instance()->Message(MSG_WARN, args )

#define PexorError(args...) \
	pexor::Logger::Instance()->Message(MSG_ERR, args )

#define PexorDebug(args...) \
	pexor::Logger::Instance()->Message(MSG_DEBUG, args )

namespace pexor {


enum MessageLevel {
MSG_DEBUG = 0, 	// show also debug output
MSG_INFO  =1,   // show info messages and above
MSG_WARN  = 2, 	// show warnings and errors only
MSG_ERR = 3		// show errors only
};

/*
 * Interface for all debug and logging output
 * */
class Logger {
public:

	virtual ~Logger();

	static pexor::Logger* Instance();

	const char* Message(pexor::MessageLevel mode, const char* text, ...);

	void EnableOutput(bool on) {fOutputEnabled=on;}

	void SetMessageLevel(pexor::MessageLevel mode){fIgnoreLevel=mode;}

private:

	Logger();

	char fMessagetext[PEXOR_LOGGER_BUFLEN];

	bool fOutputEnabled;

	pexor::MessageLevel fIgnoreLevel;

	static pexor::Logger* fInstance;


};

}

#endif /* LOGGER_H_ */
