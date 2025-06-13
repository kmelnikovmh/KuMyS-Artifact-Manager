//
// Created by Kymus-team on 2/22/25.
//

#include "ServerApp.h"
#include <mongocxx/instance.hpp>

int main() {
	mongocxx::instance instance{}; 
	return runServerApp();

};
