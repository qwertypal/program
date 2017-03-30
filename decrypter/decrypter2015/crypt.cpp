#include "StdAfx.h"
#include "crypt.h"

void provider_deleter(HCRYPTPROV h)
{
	CryptReleaseContext(h,0);
}
void hash_deleter(HCRYPTHASH h)
{
	CryptDestroyHash(h);
}
void key_deleter(HCRYPTKEY h)
{
	CryptDestroyKey(h);
}

thread_local vector<BYTE> Task::_iv;
thread_local vector<BYTE> Task::_data;
thread_local vector<BYTE> Task::_sha256;
