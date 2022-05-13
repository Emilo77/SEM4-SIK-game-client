#include "Client.h"

void Client::initialize() {}

void Client::run() {}


DrawMessage Client::handle_message_from_display() {
	switch (buffer.get_message_id()) {
		case 0:
			break;
	}
}

ServerMessage Client::handle_message_from_server() {
}
