//
// Created by shestero on 8/26/25.
//

#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <string>
using namespace std;

class Configuration {
public:
    Configuration();

    string teach_server;

    static const char* config_file_name;
};



#endif //CONFIGURATION_H
