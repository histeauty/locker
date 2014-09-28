#include <stdio.h>
#include <string>
#include <windows.h>
#include <iostream>
#include <sys/stat.h>
#include <vector>
#include <boost/filesystem.hpp>

using namespace boost::filesystem;
using std::vector;
using std::cout;
using std::endl;
using std::cin;
using std::string;
using std::advance;
struct file
{
	string path;
	HANDLE handle;
};
struct folder
{
	string path;
	vector<HANDLE> handles;
};
bool lockFolder(const path& path,vector<folder>& folders)
{
	directory_iterator lastFile;

	for(directory_iterator file(path);file!=lastFile;++file)
	{
		try
		{
			if(is_directory(file->status()))
			{
				if(lockFolder(file->path(),folders))
				{
					return true;
				}
			}
			else
			{
				folders[folders.size()-1].handles.push_back(CreateFile(file->path().string().c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL));
			}
		}
		catch(const filesystem_error& error)
		{}
	}
	return false;
}
bool verifyFile(vector<file>& files,const string& path)
{
	for(unsigned long file=0;file!=files.size();++file)
	{
		if(path==files[file].path)
		{
			if(MessageBox(NULL,"Are you sure you want to unlock this file?","Unlock File",MB_ICONWARNING|MB_YESNO)==IDYES)
			{
				CloseHandle(files[file].handle);
				vector<::file>::iterator deleteFile=files.begin();
				advance(deleteFile,file);
				files.erase(deleteFile);
			}

			return false;
		}
	}

	return true;
}
bool verifyFolder(vector<folder>& folders,const string& path)
{
	for(unsigned long folder=0;folder!=folders.size();++folder)
	{
		if(path==folders[folder].path)
		{
			unsigned short response;

			if(path.size()==3)
			{
				response=MessageBox(NULL,"Are you sure you want to unlock this drive?","Unlock Drive",MB_ICONWARNING|MB_YESNO);
			}
			else
			{
				response=MessageBox(NULL,"Are you sure you want to unlock this folder?","Unlock Folder",MB_ICONWARNING|MB_YESNO);
			}
			if(response==IDYES)
			{
				for(unsigned long handle=0;handle!=folders[folder].handles.size();++handle)
				{
					CloseHandle(folders[folder].handles[handle]);
				}
				vector<::folder>::iterator deleteFolder=folders.begin();
				advance(deleteFolder,folder);
				folders.erase(deleteFolder);
			}

			return false;
		}
	}

	return true;
}
int main(int argc,char* argv[])
{
	vector<file> files;
	vector<folder> folders;

	SetConsoleTitle("Locker - Samuel S. Sohn");
	CreateFile(argv[0],GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	while(true)
	{
		string path;
		struct stat buffer;

		cout<<"Drag and drop a file or directory to lock or unlock,"<<endl
		<<"or press Enter to unlock all locked files and directories, and exit."<<endl;
		getline(cin,path);
		if(!path.size())
		{
			return 0;
		}
		cout<<endl;
		if(path[0]=='"')
		{
			path=path.substr(1,path.size()-2);
		}
		stat(path.c_str(),&buffer);
		if(buffer.st_mode&S_IFDIR)
		{
			if(verifyFolder(folders,path))
			{
				folders.push_back(folder());
				folders[folders.size()-1].path=path;
				lockFolder(path,folders);
			}
		}
		else if(buffer.st_mode&S_IFREG)
		{
			if(verifyFile(files,path))
			{
				files.push_back(file());
				files[files.size()-1].path=path;
				files[files.size()-1].handle=CreateFile(path.c_str(),GENERIC_READ,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
			}
		}
	}

	return 0;
}
