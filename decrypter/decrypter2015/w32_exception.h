#pragma once

void throw_on_error(const char * error_message, DWORD error=0);

class w32_exception : public std::exception
{
public:
	w32_exception(DWORD error) : _w32_error(error), _w32_description(NULL) {}
	void description(const char * message)
	{
		clean_up();
		size_t sz = strlen(message);
		resize(sz + 1);
		strncpy_s(_w32_description, sz+1, message, sz);
	}
	virtual const char* what()
	{
		char f_message[200];
		DWORD res = FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ARGUMENT_ARRAY,
			NULL,
			GetLastError(),
			0,
			f_message,
			200,
			NULL);
		f_message[res]=0;

		if(_w32_description)
		{
			resize((size_t)(res + 2));
			strcat_s(_w32_description, strlen(_w32_description), ": ");
		}
		strcat_s(_w32_description, strlen(_w32_description), f_message);
		return _w32_description;
	}
	virtual ~w32_exception()
	{
		clean_up();
	}
private:
	void resize(size_t new_size)
	{
		char * old_descr;
		size_t old_size=0;
		if(_w32_description)
		{
			old_size = strlen(_w32_description);
			old_descr = new char[strlen(_w32_description)];
		}
		clean_up();
		_w32_description = new char[new_size + old_size];
		if (old_descr)
		{
			strncpy_s(_w32_description, new_size, old_descr, old_size);
			delete [] old_descr;
		}
	}
	void clean_up()
	{
		if(_w32_description)
			delete [] _w32_description;
	}
	DWORD  _w32_error;
	char * _w32_description;
};