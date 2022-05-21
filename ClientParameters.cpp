#include "ClientParameters.h"


namespace po = boost::program_options;

// A helper function to simplify the main part.
template<class T>
std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
	copy(v.begin(), v.end(), ostream_iterator<T>(os, " "));
	return os;
}

void exit_program(int status) {
	if (status) {
		std::cerr << "Consider using -h [--help] option" << std::endl;
	}
	exit(status);
}

void split_ip(std::string &ip, std::string &address, std::string &port) {
	size_t pos = ip.find_last_of(':');
	if (pos == std::string::npos) {
		return;
	} else {
		address = ip.substr(0, pos);
		port = ip.substr(pos + 1);
	}
}


static inline void check_port(int possible_port) {
	if (possible_port < 0 || possible_port > 65535) {
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "port");
	}
}

static inline void check_port_str(std::string &port_str) {
	try {
		boost::numeric_cast<uint16_t>(boost::lexical_cast<int>(port_str));
	} catch (boost::bad_lexical_cast &) {
		std::cerr << "Invalid port format in ip" << std::endl;
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "ip port");
	} catch (boost::bad_numeric_cast &) {
		std::cerr << "Port out of range in ip" << std::endl;
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "ip port");
	}
}

//todo może trzeba spradzić, czy porty takie same
static inline void check_address(const std::string &address) {
	if (address.empty()) {
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "address");
	}
	boost::system::error_code ec;
	boost::asio::ip::address::from_string(address, ec);
	if (ec) {
		std::cerr << "Invalid ip address" << std::endl;
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "ip address");
	}
}

static inline void check_ip(std::string ip) {
	std::string address, port;
	split_ip(ip, address, port);
//	check_address(address);
	check_port_str(port);
}

//porównywanie adresów ip
void ClientParameters::compare_ip() const {
	if (display_ip == server_ip) {
		throw po::validation_error(po::validation_error::invalid_option_value,
		                           "same display ip and server ip");
	}
}

//może zmienić parsowanie portu jako string
void ClientParameters::check_parameters() {
	int possible_port = -1;
	const po::positional_options_description p; // empty positional options
	po::options_description desc(1024, 512);
	try {
		desc.add_options()
				("help,h", "produce help message")
				("gui-address,d",
				 po::value<std::string>(&display_ip)->value_name
								 ("<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>")
						 ->required()->notifier(&check_ip),
				 "set the gui address")
				("player-name,n",
				 po::value<std::string>(&player_name)->value_name("String")
						 ->required(),
				 "set the player name")
				("port,p", po::value<int>(&possible_port)->value_name("u16")
						 ->required()->notifier(&check_port),
				 "set the port number")
				("server-address,s",
				 po::value<std::string>(&server_ip)->value_name
								 ("<(hostname):(port) or (IPv4):(port) or (IPv6):(port)>")
						 ->required()->notifier(&check_ip),
				 "set the server address");

		po::variables_map vm;
		po::store(po::command_line_parser(argc, argv).
				options(desc).
				positional(p).
				run(), vm);

		if (vm.count("help")) {
			std::cout << "Program Usage: ./" << argv[0] << "\n" << desc << "\n";
			exit_program(0);
		}

		po::notify(vm);

		compare_ip();

		std::string server_port_str, display_port_str;
		split_ip(server_ip, server_address, server_port_str);
		split_ip(display_ip, display_address, display_port_str);

		server_port = boost::lexical_cast<uint16_t>(server_port_str);
		display_port = boost::lexical_cast<uint16_t>(display_port_str);

	}
	catch (std::exception &e) {
		std::cerr << "Error: " << e.what() << "\n";
		exit_program(1);
	}
	catch (...) {
		std::cerr << "Unknown error!" << "\n";
		exit_program(1);
	}
	port = (uint16_t) possible_port;
}
