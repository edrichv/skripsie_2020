#ifndef CLIOPTIONS_H
#define CLIOPTIONS_H
#include <iostream>

class CliOptions
{
private:
public:
	CliOptions(int argc, char** argv);
	char* inputNetlistPath = (char*)".";
	char* convention = (char*)"0";
	char* testPatternPath = (char*)".";
	bool verbose = false;
	bool remove_after = true;
};

#endif

