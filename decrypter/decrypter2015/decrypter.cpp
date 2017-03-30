// decrypter.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "crypt.h"
#include "utils.h"
#include "task_scheduler.h"

int _tmain(int argc, _TCHAR* argv[])
{
	vector<BYTE> file_data=load_file1();

	vector<string> passwords = permute_string();
	Task task(passwords);
	task._file = std::move(file_data);

	task_scheduler t(task);

	return 0;
}

