#include "StdAfx.h"
#include "utils.h"


vector<string> permute_string(int offset_from, int offset_to)
{
	string str;
	// upper
	for (unsigned char c = 65+26-1; c >= 65; c--)
		str += c;
	// digits
	for (unsigned char c = 48+10-1; c >= 48; c--)
		str += c;
	// lower
	for (unsigned char c = 97+26-1; c >= 97; c--)
		str += c;

	//// upper
	//for (unsigned char c = 65 + 26 - 1; c >= 65; c--)
	//	str += c;
	//// lower
	//for (unsigned char c = 97 + 26 - 1; c >= 97; c--)
	//	str += c;
	//// digits
	//for (unsigned char c = 48 + 10 - 1; c >= 48; c--)
	//	str += c;

	cout << "String='" << str << "'" << endl;

	vector<string> vec;
	vec.reserve(238328);

	// = "ZYXWVUTSRQPONMLKJIHGFEDCBA9876543210zyxwvutsrqponmlkjihgfedcba"
	// offset_to offset_from in reverse order
	for(unsigned i= offset_to; i<str.size()-offset_from ; i++)
//	for (char a : str)
		for (char b : str)
			for (char c : str)
			{
				string res(string{ str[i] } +string{ b } +string{ c });
				vec.push_back(res);
			}

	return vec;
}

vector<BYTE> load_file1(const string & filename)
{
	ifstream f(filename, ios::binary);
	vector<BYTE> vec;
	if (f)
	{
		f.unsetf(ios::skipws);

		f.seekg(0, ios::end);
		streampos fz = f.tellg();

		vec.reserve(fz);

		f.seekg(0, ios::beg);

		vec.insert(vec.begin(),
			std::istream_iterator<BYTE>(f),
			std::istream_iterator<BYTE>());
	}

	return vec;
}
//void load_file2(vector<BYTE> & vec)
//{
//	string filename( "C:\\_Data\\Personal\\Development\\C++\\decrypter\\decrypter\\interview\\test.bin" );
//	ifstream f( filename, ios::binary );
//
//	f.unsetf(ios::skipws);
//
//	f.seekg(0,ios::end);
//	streampos fz = f.tellg();
//
//	vec.reserve(fz);
//
//	f.seekg(0, ios::beg);
//
//	vec.insert(vec.begin(),
//		std::istream_iterator<BYTE>(f),
//		std::istream_iterator<BYTE>());
//}
//
//void load_data()
//{
//	vector<BYTE> v;
//	load_file2(v);
//
//	vector<BYTE> iv, data, sha256;
//	move(v.begin(), v.begin() + 8, back_inserter(iv));
//	move(v.begin() + 8, v.end() - 16, back_inserter(data));
//	move(v.end() - 16, v.end(), back_inserter(sha256));
//}

void serialize(const string & filename, vector<BYTE> & v)
{

	ofstream myFile(filename, ios::out | ios::binary);
//	char utf8[] = { 0xFE, 0xFF };
//	myFile.write(utf8, 2);
//	myFile.write((const char *)pbResult, cbData);
	myFile.write((const char *)&v[0], v.size());
}