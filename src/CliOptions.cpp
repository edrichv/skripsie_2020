#include "../include/CliOptions.h"

CliOptions::CliOptions(int argc, char** argv) {
	bool gotTPPath = false;
	for (int i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
			case 'v':
				CliOptions::verbosity = atoi(argv[++i]);
				if (CliOptions::verbosity < 0) CliOptions::verbosity = 0;
				if (CliOptions::verbosity > 2) CliOptions::verbosity = 2;
				break;
			case 'a':
				CliOptions::analysis = atoi(argv[++i]);
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