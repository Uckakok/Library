

#pragma comment(lib, "ws2_32.lib")

#include "Library.h"
#include "LibrarySystem.h"
#include "iostream"
#include <string.h>
#include "UserInterface.h"
#include "Server.h"

#ifdef __linux__
	#include <unistd.h>
	#include <linux/limits.h>
#endif

int main(int agrc, char* argv[]) {

	srand(static_cast<unsigned int>(std::time(nullptr)));

	bool bIsClient = true;
	if (agrc == 2 && strcmp(argv[1], "-server") == 0) {
		bIsClient = false;
	}

#ifdef __linux__
	bIsClient = false;
	char cwd[PATH_MAX];
	if (getcwd(cwd, sizeof(cwd)) != NULL) {
		printf("Current working dir: %s\n", cwd);
	}
#endif

	if (bIsClient)
	{
		UserInterface CurrentInterface;
		CurrentInterface.MainMenu();
	}
	else
	{
		cout << "Running as server" << endl;
		Server::GetInstance();
	}

	cout << "Finished program for: " << (bIsClient ? "client" : "server") << endl;
	return 0;
}