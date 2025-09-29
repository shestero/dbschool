#include <cstdlib>
#include <toml.hpp> // #include <toml++/toml.hpp>
#include <iostream>
#include "Configuration.h"

using namespace std;

const char* Configuration::config_file_name = "dbschool.toml";

const char* Configuration::students_file = "students.tsv";
const char* Configuration::teachers_file = "teachers.tsv";

const char* Configuration::date_format = "dd.MM.yyyy";

Configuration::Configuration()
{
    try
    {
        auto config = toml::parse(config_file_name);
        window_days = config.contains("window-days")? config.at("window-days").as_integer(): 120;
        teach_server = config.at("teachserver").as_string();
        login = config.at("api").at("login").as_string();
        password = config.at("api").at("password").as_string();

        cout << "teach_server = " << teach_server << endl;
        cout << "configuration " << config_file_name << " was successfully parsed!" << endl;
    }
    catch (...) // (const toml::parse_error& err)
    {
        cerr << "Parsing failed:\n";// << err << endl;
        exit(1);
    }
}

static Configuration gl_configuration;
