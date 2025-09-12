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
    string login;
    string password;

    // Сколько дней назад смотреть при распределении учеников по кружкам
    long window_days;

    static const char* config_file_name;
    static const char* students_file;
    static const char* date_format;
};



#endif //CONFIGURATION_H
