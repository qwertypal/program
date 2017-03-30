#pragma once

#include <mutex>
#include <Wincrypt.h>

#include "w32_exception.h"
#include "utils.h"


void provider_deleter(HCRYPTPROV h);
void hash_deleter(HCRYPTHASH h);
void key_deleter(HCRYPTKEY h);

struct Task
{
public:
	vector<BYTE>  _result_data;
	string _result_password;

private:
	thread_local static vector<BYTE> _iv;
	thread_local static vector<BYTE> _data;
	thread_local static vector<BYTE> _sha256;

	vector<string> & _passwords;
	mutex _m_password;

	vector<BYTE> & _file;


	void init()
	{
		//move(_file.begin(),		_file.begin() + 8,	back_inserter(_iv));
		//move(_file.begin() + 8, _file.end() - 32,	back_inserter(_data));
		//move(_file.end() - 32,	_file.end(),		back_inserter(_sha256));
		copy(_file.begin(),		_file.begin() + 8,	back_inserter(_iv));
		copy(_file.begin() + 8, _file.end() - 32,	back_inserter(_data));
		copy(_file.end() - 32,	_file.end(),		back_inserter(_sha256));
	}
	string safe_load_password()
	{
		std::lock_guard<mutex> l(_m_password);
		string password = _passwords.back();
		_passwords.pop_back();
		if ((password[1] == password[2]) && (password[1]  == 'a'))
			cout << "Starting passwords <" << password[0] << "**> (id=" << this_thread::get_id() << ")" << endl;
		return password;
	}
	bool check_sha256(Handle<HCRYPTPROV> &hProvider,
		vector<BYTE> &data)
	{
		BOOL ret;
		Handle<HCRYPTHASH> hHash256(hash_deleter);
		ret = CryptCreateHash(
			hProvider,
			CALG_SHA_256,
			0,
			0,
			hHash256.get());
		if(!ret) throw_on_error("CryptCreateHash_256");
		ret = CryptHashData(
			hHash256,
			&data[0],
			data.size(),
			0);
		if(!ret) throw_on_error("CryptHashData_256");

		DWORD hash_size;
		DWORD sz=sizeof(DWORD);
		ret = CryptGetHashParam(
			hHash256, 
			HP_HASHSIZE,
			reinterpret_cast<BYTE*>(&hash_size),
			&sz,
			0);
		if(!ret) throw_on_error("CryptGetHashParam_sz256");

		vector<BYTE> buffer(hash_size);
		sz=buffer.size();
		ret = CryptGetHashParam(
			hHash256, 
			HP_HASHVAL,
			&buffer[0],
			&sz,
			0);
		if(!ret) throw_on_error("CryptGetHashParam_Val256");

		return buffer == _sha256;
	}
public:

	Task(vector<string> & passwords, vector<BYTE> & file) : _passwords(passwords), _file(file) 	{	}

	bool operator()()
	{
		string _password = safe_load_password();
		init();
		// open provider
		Handle<HCRYPTPROV> hProvider(provider_deleter);
		BOOL ret = CryptAcquireContext(
			hProvider.get(),
			0,
			MS_ENH_RSA_AES_PROV,
			PROV_RSA_AES,
			CRYPT_VERIFYCONTEXT);
		if (!ret) throw_on_error("CryptAcquireContext");

		// create hash
		Handle<HCRYPTHASH> hHash(hash_deleter);
		ret = CryptCreateHash(
			hProvider,
			CALG_MD5,
			0,
			0,
			hHash.get());
		if (!ret) throw_on_error("CryptCreateHash");
		ret = CryptHashData(
			hHash,
			reinterpret_cast<const BYTE*>(_password.c_str()),
			_password.length(),
			0);
		if (!ret) throw_on_error("CryptHashData");

		// create key
		Handle<HCRYPTKEY> hKey(key_deleter);
		ret = CryptDeriveKey(
			hProvider,
			CALG_3DES_112,
			hHash,
			0,
			hKey.get());
		if (!ret) throw_on_error("CryptDeriveKey");

		//DWORD dwKeyLen = 40;
		//ret = CryptSetKeyParam(
		//	hKey,
		//	KP_EFFECTIVE_KEYLEN,
		//	reinterpret_cast<const BYTE*>(&dwKeyLen),
		//	0);
		//throw_on_error("CryptDeriveKey");

		// Set IV
		ret = CryptSetKeyParam(
			hKey,
			KP_IV,
			&_iv[0],
			0);
		if (!ret) throw_on_error("CryptSetKeyParam");


		Handle<HCRYPTKEY> hKeyCopy(key_deleter);
		ret = CryptDuplicateKey(hKey, NULL, 0, hKeyCopy.get());
		if (!ret) throw_on_error("CryptDuplicateKey");

		// decrypt
		vector<BYTE> decoded(_data.size());

		const int BLOCK_SZ_BYTES = 16;
		int num_block = _data.size() / BLOCK_SZ_BYTES;
		bool proceed = true;
		for (int current_block = 0; (current_block < num_block) && proceed; current_block++)
		{
			DWORD current_block_sz;

			// make final block 64 bytes
			if (_data.size() - current_block*BLOCK_SZ_BYTES == 64)
				current_block_sz = 64;
			else
				current_block_sz = BLOCK_SZ_BYTES;

			vector<BYTE> buffer(current_block_sz);
			move(_data.begin() + current_block*BLOCK_SZ_BYTES,
				_data.begin() + current_block*BLOCK_SZ_BYTES + current_block_sz,
				buffer.begin());

			bool bFinal = current_block_sz == 64 ? true : false;

			//			ret = CryptDecrypt(hKey, 0, bFinal, 0, &buffer[0], &current_block_sz);
			ret = CryptDecrypt(hKeyCopy, 0, bFinal, 0, &buffer[0], &current_block_sz);
			DWORD error = GetLastError();
			if (error && (bFinal && (error != 0x80090005)))
				// in final block do not throw on 0x80090005
				// on other errors do
				if (!ret) throw_on_error("CryptDecrypt", error);

			move(buffer.begin(),
				buffer.end(),
				decoded.begin() + current_block * BLOCK_SZ_BYTES);

			if (bFinal)
				// quit looping if final block is processed
				proceed = false;

			//
			//			vector<BYTE> buffer(BLOCK_SZ_BYTES);
			//			move(_data.begin()+current_block*BLOCK_SZ_BYTES,
			//				 _data.begin()+(current_block+1)*BLOCK_SZ_BYTES,
			//				 buffer.begin());
			//
			//			DWORD sz = BLOCK_SZ_BYTES;
			//			bool bFinal = current_block + 1 == num_block ? true : false;
			//
			//if(current_block==100 || bFinal == true)
			//{
			//DWORD dData;
			//DWORD sz = sizeof(DWORD);
			//CryptGetKeyParam(hKey, KP_BLOCKLEN, (BYTE*)&dData, &sz,0);
			//sz=sz;
			//}
			//				
			////bFinal = false;
			//			ret = CryptDecrypt(hKey, 0, bFinal, 0, &buffer[0], &sz);

						//DWORD error = GetLastError();
						//if(error && (bFinal || error != 0x80009003))
						//	// in final block throw on any error but 0x80009003
						//	throw_on_error("CryptDecrypt",error);
			//			move(buffer.begin(),
			//				 buffer.end(),
			//				 decoded.begin() + current_block * BLOCK_SZ_BYTES);
		}

		bool return_result = check_sha256(hProvider, decoded);
		if (return_result)
		{
			_result_data = move(decoded);
			_result_password = _password;
		}

		// empty vars
		vector<BYTE>().swap(_data);
		vector<BYTE>().swap(_iv);
		vector<BYTE>().swap(_sha256);

		return return_result;
	}
};




