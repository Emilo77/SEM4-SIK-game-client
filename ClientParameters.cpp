#include "ClientParameters.h"
#include <iostream>
#include <fstream>

namespace po = boost::program_options;
using namespace std;

void ClientParameters::exit_program(ClientParameters::wrong_parameters) {

}

// A helper function to simplify the main part.
template<class T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v)
{
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
	return os;
}

void ClientParameters::check_parameters() {
	try {
		int opt;
		string config_file;

		// Declare a group of options that will be
		// allowed only on command line
		po::options_description generic("Generic options");
		generic.add_options()
		        ("version,v", "print version string")
		                ("help", "produce help message")
		                        ("config,c", po::value<std::string>(&config_file)->default_value("Essa dziala"),
		                         "name of a file of a configuration.")
		        ;

		// Declare a group of options that will be
		// allowed both on command line and in
		// config file
		po::options_description config("Configuration");
		config.add_options()
		        ("optimization", po::value<int>(&opt)->default_value(10),
		         "optimization level")
		                ("include-path,I",
		                 po::value< vector<string> >()->composing(),
		                 "include path")
		        ;

		// Hidden options, will be allowed both on command line and
		// in config file, but will not be shown to the user.
		po::options_description hidden("Hidden options");
		hidden.add_options()
		        ("input-file", po::value< vector<string> >(), "input file")
		        ;


		po::options_description cmdline_options;
		cmdline_options.add(generic).add(config).add(hidden);

		po::options_description config_file_options;
		config_file_options.add(config).add(hidden);

		po::options_description visible("Allowed options");
		visible.add(generic).add(config);

		po::positional_options_description p;
		p.add("input-file", -1);

		po::variables_map vm;
		store(po::command_line_parser(argc, argv).
		      options(cmdline_options).positional(p).run(), vm);
		notify(vm);

		std::ifstream ifs(config_file.c_str());
		if (!ifs)
		{
			cout << "can not open config file: " << config_file << "\n";
			return;
		}
		else
		{
			store(parse_config_file(ifs, config_file_options), vm);
			notify(vm);
		}

		if (vm.count("help")) {
			cout << visible << "\n";
			return;
		}

		if (vm.count("version")) {
			cout << "Multiple sources example, version 1.0\n";
			return;
		}

		if (vm.count("include-path"))
		{
			cout << "Include paths are: "
			     << vm["include-path"].as< vector<string> >() << "\n";
		}

		if (vm.count("input-file"))
		{
			cout << "Input files are: "
			     << vm["input-file"].as< vector<string> >() << "\n";
		}

		cout << "Optimization level is " << opt << "\n";
	}
	catch(exception& e)
	{
		cout << e.what() << "\n";
		return;
	}
}
