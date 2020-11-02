#ifndef CLIOPTIONS_H
#define CLIOPTIONS_H
#include <iostream>

class CliOptions
{
private:
public:
	CliOptions(int argc, char** argv);
	char* inputNetlistPath = (char*)".";
	char* testPatternPath = (char*)".";
	char verbosity = 0;
	char analysis = 1;
};

#endif

