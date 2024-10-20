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

	bool isClient = true;
	if (agrc == 2 && strcmp(argv[1], "-server") == 0) {
		isClient = false;
	}

//#ifdef __linux__
//	isClient = false;
//	char cwd[PATH_MAX];
//	if (getcwd(cwd, sizeof(cwd)) != NULL) {
//		printf("Current working dir: %s\n", cwd);
//	}
//#endif

	if (isClient)
	{
		UserInterface currentInterface;
		currentInterface.MainMenu();
	}
	else
	{
		std::cout << "Running as server" << std::endl;
		Server::GetInstance();
	}

	std::cout << "Finished program for: " << (isClient ? "client" : "server") << std::endl;
	return 0;
}