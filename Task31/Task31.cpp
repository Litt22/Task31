#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <string.h>

BOOL ProcessFile(HANDLE hTargetBuffer, const char* filename, BOOL hideErrors)
{
	HANDLE hFile = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE)
	{
		if (hTargetBuffer != NULL && !hideErrors)
		{
			char errorMsg[256];
			wsprintfA(errorMsg, "Error: File '%s' not found!\n", filename);
			DWORD dwWritten;
			WriteConsoleA(hTargetBuffer, errorMsg, lstrlenA(errorMsg), &dwWritten, NULL);
		}
		return FALSE;
	}

	if (hTargetBuffer == NULL)
	{
		CloseHandle(hFile);
		return TRUE;
	}

	char fileBuffer[1024];
	DWORD bytesRead = 0;
	DWORD bytesWritten = 0;

	while (ReadFile(hFile, fileBuffer, sizeof(fileBuffer) - 1, &bytesRead, NULL) && bytesRead > 0)
	{
		fileBuffer[bytesRead] = '\0';
		WriteConsoleA(hTargetBuffer, fileBuffer, bytesRead, &bytesWritten, NULL);
	}

	CloseHandle(hFile);
	return TRUE;
}

int main(int argc, char* argv[])
{
	BOOL suppressErrors = FALSE;
	int fileArgsCount = 0;
	int fileIndices[10];

	for (int i = 1; i < argc; i++)
	{
		if (strcmp(argv[i], "-s") == 0)
		{
			suppressErrors = TRUE;
		}
		else
		{
			if (fileArgsCount < 10)
			{
				fileIndices[fileArgsCount++] = i;
			}
		}
	}

	HANDLE hStdOutOld = GetStdHandle(STD_OUTPUT_HANDLE);

	if (fileArgsCount > 0)
	{
		for (int i = 0; i < fileArgsCount; i++)
		{
			BOOL exists = ProcessFile(NULL, argv[fileIndices[i]], suppressErrors);
			if (!exists && !suppressErrors)
			{
				ProcessFile(hStdOutOld, argv[fileIndices[i]], suppressErrors);
			}
		}
	}

	HANDLE hStdOutNew = CreateConsoleScreenBuffer(
		GENERIC_READ | GENERIC_WRITE,
		0,
		NULL,
		CONSOLE_TEXTMODE_BUFFER,
		NULL);

	if (hStdOutNew == INVALID_HANDLE_VALUE)
	{
		_cputs("Create console screen buffer failed.\n");
		_getch();
		return FALSE;
	}

	_cputs("Press any key to activate NEW screen buffer...\n");
	_getch();

	if (!SetConsoleActiveScreenBuffer(hStdOutNew))
	{
		_cputs("Set new console active screen buffer failed.\n");
		CloseHandle(hStdOutNew);
		_getch();
		return FALSE;
	}

	DWORD dwWritten;
	char outBuffer[256];

	WORD wAttribute = BACKGROUND_GREEN | BACKGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_INTENSITY;
	DWORD dwAttrsWritten = 0;

	COORD startLineCoord = { 0, 0 };

	wsprintfA(outBuffer, "--- NEW SCREEN BUFFER OUTPUT ---");
	DWORD titleLength = lstrlenA(outBuffer);

	WriteConsoleA(hStdOutNew, outBuffer, titleLength, &dwWritten, NULL);
	FillConsoleOutputAttribute(hStdOutNew, wAttribute, titleLength, startLineCoord, &dwAttrsWritten);

	WriteConsoleA(hStdOutNew, "\n", 2, &dwWritten, NULL);

	const char* fileToDisplay = (fileArgsCount > 0) ? argv[fileIndices[0]] : "test.txt";
	wsprintfA(outBuffer, "Content of file '%s':\n", fileToDisplay);
	WriteConsoleA(hStdOutNew, outBuffer, lstrlenA(outBuffer), &dwWritten, NULL);

	ProcessFile(hStdOutNew, fileToDisplay, FALSE);

	CONSOLE_SCREEN_BUFFER_INFO csbi;
	GetConsoleScreenBufferInfo(hStdOutNew, &csbi);
	COORD endLineCoord = csbi.dwCursorPosition;

	WriteConsoleA(hStdOutNew, "\n", 1, &dwWritten, NULL);

	wsprintfA(outBuffer, "--------------------------------");
	DWORD lineLength = lstrlenA(outBuffer);

	GetConsoleScreenBufferInfo(hStdOutNew, &csbi);
	endLineCoord = csbi.dwCursorPosition;

	WriteConsoleA(hStdOutNew, outBuffer, lineLength, &dwWritten, NULL);
	FillConsoleOutputAttribute(hStdOutNew, wAttribute, lineLength, endLineCoord, &dwAttrsWritten);

	WriteConsoleA(hStdOutNew, "\n\n", 2, &dwWritten, NULL);

	wsprintfA(outBuffer, "Press any key to restore old screen buffer.");
	WriteConsoleA(hStdOutNew, outBuffer, lstrlenA(outBuffer), &dwWritten, NULL);

	_getch();

	SetConsoleActiveScreenBuffer(hStdOutOld);
	CloseHandle(hStdOutNew);

	_cputs("Returned back. Press any key to finish.\n");
	_getch();

	return TRUE;
}
