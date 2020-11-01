#include "../include/CliOptions.h"

CliOptions::CliOptions(int argc, char** argv) {
	CliOptions::convention = (char*)"0";
	bool gotTPPath = false;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'c':
				CliOptions::convention = argv[++i];
				break;
			case 'v':
				CliOptions::verbose = true;
				break;
			case 'r':
				CliOptions::remove_after = false;
			default:
				break;
			}
		}
		else{
			if (!gotTPPath) {
				CliOptions::testPatternPath = argv[i];
				gotTPPath = true;
			}
			else {
				CliOptions::inputNetlistPath = argv[i];
			}
		}
	}
}