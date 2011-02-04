/*
 * User_Buffer.h
 *
 *  Created on: 28.01.2010
 *      Author: adamczew
 */

#ifndef USER_BUFFER_H_
#define USER_BUFFER_H_

#include "Buffer.h"

namespace pexor {

class User_Buffer: public pexor::Buffer {
public:

	User_Buffer(size_t size);

	virtual ~User_Buffer();

};

}



#endif /* USER_BUFFER_H_ */
