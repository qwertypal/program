#include "StdAfx.h"
#include "w32_exception.h"
#include "WinBase.h" 

void throw_on_error(const char * error_message, DWORD error)
{
		DWORD e;
		(error==0) ? e=GetLastError() : e=error ;

		if(e)
		{
			CHAR buffer[1024];
			FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
				NULL,
				e,
				0,
				buffer,
				1024,
				NULL);
			throw std::exception(string(string(error_message) + string(": ") + string(buffer)).c_str());
		}
}
