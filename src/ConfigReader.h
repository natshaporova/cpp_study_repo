#pragma once

#include <iostream>
#include <map>

//define namespace to avoid collisions 
namespace jirareports
{
    //Type of class is singleton.
    //The responsobilty of this class is to parse the config file
    //add store it in the std::map, 
    //define the getter function getInstance()
    //to get the data fron the std::map.
    class ConfigReader
    {
        private:
            //Define the map to store data from the config file.
            std::map<std::string,std::string> m_Conf_settings_map;

            // Static pointer instance to make this class singleton.
            static ConfigReader* m_pConfReaderInc;

        public:

            //Public static method for object creation.
            static ConfigReader* getInstance();

            //Parse the config file.
            bool parseFile(std::string_view& filename);
            
            //Overloaded getValue() function
            //to get int values and string values

            bool getValue(std::string tag, int& value);
            bool getValue(std::string tag, std::string& value);
            void dumpFileValues();

        private:
            //Constructor is in the private section to make class
            //as singleton.
            ConfigReader();
            ~ConfigReader();

            //Define copy constructor:
            //we use copy constructor 
            //--if there is dynamic memory 
            //allocation in the class;
            //--if we use pointers variables inside the class; 
            //in this case we need to define copy constructor 
            //and overload the assignment operator.
            ConfigReader(const ConfigReader& obj);
            void operator=(const ConfigReader&obj){};

            //Helper functions
            std::string trim(const std::string& str, const std::string& whitespace = " \t" );
            std::string reduce(const std::string& str,
                               const std::string& fill = " ",
                               const std::string& whitespace = " \t");                         

    };

}
