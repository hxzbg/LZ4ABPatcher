#include <io.h>
#include "uthash.h"
#include "MergeParser.h"
#include "BundleFile.h"
#include "AsignParser.h"
#include "EndianBinaryWriter.h"

struct FileInfoItem
{
	int type;
	char* filename;
	UT_hash_handle hh;
};

int main(int argc, char** argv)
{
	if (argc < 5)
	{
		return 0;
	}

	char* buffer = (char*)malloc(512);
	if (_stricmp(argv[1], "compare") == 0)
	{
		FileInfoItem* files = NULL;
		for (int i = 1; i < 3; i++)
		{
			long Handle;
			struct _finddata_t fileinfo;
			sprintf(buffer, "%s*.*", argv[i + 1]);
			Handle = _findfirst(buffer, &fileinfo);
			if (-1 == Handle)
				return -1;
			do 
			{
				FileInfoItem* item;
				if ((fileinfo.attrib == _A_NORMAL || (fileinfo.attrib & _A_ARCH) == _A_ARCH) && fileinfo.size > 6)
				{
					sprintf(buffer, "%s%s", argv[i + 1], fileinfo.name);
					if (BundleFileParserForAsign::AsignBundle(buffer))
					{
						HASH_FIND_STR(files, fileinfo.name, item);
						if (item == NULL)
						{
							item = new FileInfoItem();
							int namelen = strlen(fileinfo.name) + 1;
							item->filename = (char*)malloc(namelen);
							memcpy(item->filename, fileinfo.name, namelen);
							HASH_ADD_STR(files, filename, item);
						}
						item->type |= i;
					}
				}
			} while (!_findnext(Handle, &fileinfo));
			_findclose(Handle);
		}

		char name_to[512];
		char name_patch[512];
		while (files != NULL)
		{
			FileInfoItem* next = (FileInfoItem*)files->hh.next;
			sprintf(buffer, "%s%s", argv[2], files->filename);
			sprintf(name_to, "%s%s", argv[3], files->filename);
			sprintf(name_patch, "%s%s.patch", argv[4], files->filename);
			BundleFileParserForCompress::BuildPatch(buffer, name_to, name_patch, files->filename);
			delete(files);
			files = next;
		}
	}
	else if (_stricmp(argv[1], "merge") == 0)
	{
		long Handle;
		struct _finddata_t fileinfo;
		sprintf(buffer, "%s*.patch", argv[3]);
		Handle = _findfirst(buffer, &fileinfo);
		if (-1 == Handle)
			return -1;
		do 
		{
			if ((fileinfo.attrib == _A_NORMAL || (fileinfo.attrib & _A_ARCH) == _A_ARCH) && fileinfo.size > 6)
			{
				sprintf(buffer, "%s%s", argv[3], fileinfo.name);
				ParserForWriteBundleHeader::MergeFormPatch(buffer, argv[2], argv[4]);
			}
		} while (!_findnext(Handle, &fileinfo));
		_findclose(Handle);
	}
	SAFE_FREE(buffer);
}