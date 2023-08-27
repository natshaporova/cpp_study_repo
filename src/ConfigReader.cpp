#include "ConfigReader.h"

#include <fstream>
#include <algorithm>


#define NULL_PTR 0

namespace jirareports
{
	ConfigReader* ConfigReader::m_pConfReaderInc = NULL_PTR;

	ConfigReader::ConfigReader()
	{
    		m_Conf_settings_map.clear();
	}
	ConfigReader::~ConfigReader()
	{
    		m_Conf_settings_map.clear();
	}

	ConfigReader* ConfigReader::getInstance()
	{
    		if( m_pConfReaderInc == NULL_PTR){
        		m_pConfReaderInc = new ConfigReader;
    		}
   	 	return m_pConfReaderInc;
	}

	bool ConfigReader::getValue(std::string tag, int& value){
    		std::map<std::string, std::string>::iterator it;
    		it = m_Conf_settings_map.find(tag);
    		if(it != m_Conf_settings_map.end()){
        		//c_str() 
        		// Returns a pointer to an array that contains
        		// a null-terminated sequence of characters
        		// (i.e., a C-string) representing the current value
        		// of the string object.
        		// atoi gets an argument
        		// const char * str;
        		value = atoi((it->second).c_str());
        		return true;
    		}
    		return false;
	}

	bool ConfigReader::getValue(std::string tag, std::string& value)
	{
	    	std::map<std::string,std::string>::iterator it;
    		it=m_Conf_settings_map.find(tag);
    		if(it != m_Conf_settings_map.end())
    		{
        		value = it->second;
        		return true;
    		}
    		return false;
	}

	bool ConfigReader::parseFile(std::string_view& filename){
		
    		std::ifstream inputFile (std::string(filename).c_str(),std::ifstream::in);
    		std::string delimiter = "=";
    		int initPos=0;
    
    		if (inputFile.fail()){
        		std::cout << "Unable to find config file" << std::endl;
       			 return false;
    		} 
    		std::string line;
 
		while(std::getline(inputFile,line)){
        		// size_t is the type used to represent sizes 
        		//(as its names implies). 
        		//Its platform (and even potentially implementation) dependent,
        		// and should be used only for this purpose.
        		// Obviously, representing a size, size_t is unsigned.


        		size_t found = line.find_first_of("#");
        		std::string configData = line.substr(0,found);

        		configData.erase(std::remove(configData.begin(),configData.end(),'\r'),
                                        configData.end());

        		// go to the next loop interation in case the line is empty
        		if(configData.empty())
            			continue;
       			unsigned int length = configData.find(delimiter);
        		std::string tag,value;

        //string :: npos - This value, when used as the value for a len 
        //(or sublen) parameter in string's member functions,
        // means "until the end of the string".
        //
       			if (length!=std::string::npos)
        		{
           			 tag   = configData.substr(initPos, length);
			         value = configData.substr(length+1);
        		}
        
        		// Trim white spaces
        		tag   =  reduce(tag);
        		value =  reduce(value);

        		if(tag.empty() || value.empty())
            			continue;

        //check if any of the tags is re[resenting more than one times
        //it needs to peek the latest one instead of old one.
        
        		std::map<std::string, std::string>::iterator itr = m_Conf_settings_map.find(tag);
        		if(itr != m_Conf_settings_map.end()){
            			m_Conf_settings_map.erase(tag);
        		}
       			 //std::cout << "tag" << tag <<" value:"<<value;
        		m_Conf_settings_map.insert(std::pair<std::string,std::string>(tag, value));
    

    		}
    		return true;
	}

	std::string ConfigReader::trim(const std::string& str,
     		const std::string& whitespace)
     	{
        // define first position of whitespace 
        // before the string Begin
        	size_t strBegin = str.find_first_not_of(whitespace);
        	if (strBegin == std::string::npos)
            		return "";
        //define the last position of the string before
        //the whitepaces Begin    
        	size_t strEnd = str.find_last_not_of(whitespace);
        
        //calculate the length of substring 
        //without whitespaces
       		 size_t strRange = strEnd - strBegin +1;
        	return str.substr(strBegin, strRange);

     	}
	std::string ConfigReader::reduce(const std::string& str,
    		const std::string& fill,
    		const std::string& whitespace)
    	{
        //trim 
        	std::string result = trim(str);
        //std::cout << "trim " << result << std::endl;

        	size_t beginSpace = result.find_first_of(whitespace);
        	while (beginSpace != std::string::npos)
        	{
            		size_t endSpace = result.find_first_not_of(whitespace, beginSpace);
            		size_t range = endSpace - beginSpace;

            		result.replace(beginSpace, range, fill);
            		size_t newStart = beginSpace + fill.length();

            		beginSpace = result.find_first_of(whitespace,newStart);
        	}
        	return result;
    	}

	void ConfigReader::dumpFileValues(){
   		std::map<std::string,std::string>::iterator it;
    		for(it=m_Conf_settings_map.begin(); it!=m_Conf_settings_map.end(); ++it){
        		std::cout << it->first << "=" <<it->second << std::endl;
    		}

	}
}
