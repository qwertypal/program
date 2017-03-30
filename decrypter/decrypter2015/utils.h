#pragma once

#include <fstream>
#include <iterator>

vector<BYTE> load_file1(const string & str);
//void load_file2(vector<BYTE> & fd);
//void load_data();
vector<string> permute_string(int offset_from=0, int offset_to=0);
void serialize(const string & filename, vector<BYTE> & v);

template<typename T, typename Deleter=void(*)(T)>
class Handle
{
	T _handle{0};
	Deleter _d;
public:
	explicit Handle(Deleter d) : _d(d) {}
	operator T() const { return _handle; }
	T* get() { return &_handle; }
	~Handle() { if(_handle) _d(_handle); }
};