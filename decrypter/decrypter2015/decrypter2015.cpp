// decrypter2015.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "crypt.h"
#include "utils.h"
#include "task_scheduler.h"

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cout << "Usage: decrypter.exe <file path>" << endl;
		return 0;
	}

	string file_path{ argv[1] };
	int offset_from{ argc > 2 && argv[2] ? atoi(argv[2]) : 0 };
	int offset_to{ argc > 3 && argv[3] ? atoi(argv[3]) : 0 };

	vector<BYTE> file_data = load_file1(file_path);
	if (file_data.size())
	{
		vector<string> passwords = permute_string(offset_from, offset_to);

		Task task(passwords, file_data);
		{
			Task_scheduler scheduler(task);
		}

		if (task._result_data.size())
		{
			serialize("out.txt", task._result_data);
			cout << "Password=" << task._result_password << ". Text saved to out.txt" << endl;
		}
		else
		{
			cout << "Password not found"<< endl;
		}
	}
	else
	{
		cout << "File not found." << endl;
	}

	return 0;
}

