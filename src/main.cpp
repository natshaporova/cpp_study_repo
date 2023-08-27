
#include "DataPreparation.h"
#include "ConfigReader.h"


#include <cpr/cpr.h>

#include <iostream>
#include <string>
#include <string_view>

using namespace nn;
using namespace jirareports;


// function to work with  command line  options
std::string_view get_option(
    const std::vector<std::string_view>& args,
    const std::string_view& option_name){
    for( auto i = args.begin(), end=args.end(); i!=end; ++i){
        if (*i == option_name)
            if ( i+1 !=end ){
                std::cout << *(i+1) << std::endl;
                return *(i+1);
            }
    }
    return "";
}

//function to work with command line options
bool has_option(
    const std::vector<std::string_view>& args,
    const std::string_view& option_name){
    for (auto i = args.begin(),end = args.end(); i!=args.end(); ++i){
        if(*i == option_name){
            return true;
            }
    }
    return false;
}



int main(int argc, char** argv) {

    //read the commandline arguments
    const std::vector<std::string_view> args(argv,argv + argc);
    const std::string_view h {"--help"};
    const std::string_view p_conf_path {"--jira_config_file"};
    std::string_view file_path;
 
    if (has_option(args,h)){
        std::cout << "--jira_config_file : option to set the link to config file\n" <<std::endl;
	return 0;
    }
    if (!has_option(args,p_conf_path)){
        std::cout << "Incorrect options.Please see --help.\n" << std::endl;
	return 1;
    }
    if (has_option(args,p_conf_path)){
	file_path=get_option(args,p_conf_path);
    }

    //parse config file
    ConfigReader* p = ConfigReader::getInstance();
    p->parseFile(file_path);
    p->dumpFileValues();


    //make request to jira_resource
    Json::Reader reader;
    Json::Value allJiraTasks;

    std::string baseurl, sprint, user, token;

    p->getValue("baseurl",baseurl);
    p->getValue("sprint",sprint);
    p->getValue("user",user);
    p->getValue("token",token);
    std::string str_request = baseurl + sprint + "&expand=changelog";

    cpr::Response r = cpr::Get(cpr::Url{str_request},
                               cpr::Authentication{user, token, cpr::AuthMode::BASIC},
                               cpr::Header{{"Accept", "application/json"}, {"Content-Type", "application/json"}});


    //get data from json

    reader.parse(r.text, allJiraTasks);
    const Json::Value &allJiraIssues = allJiraTasks["issues"];
    std::map<std::string, std::vector<std::array<std::string, 3>>> issuesStates;
    for (int i = 0; i < allJiraIssues.size(); ++i)
    {
        //std::cout << "key: " << allJiraIssues[i]["key"] << std::endl;
        //std::cout << "---------------------------------" << std::endl;
        const Json::Value &issueHistory = allJiraIssues[i]["changelog"]["histories"];
        std::vector<std::array<std::string, 3>> statuses;
        for (int j = 0; j < issueHistory.size(); ++j)
        {

            // collect statuses for the issue into vector statuses
            for (int z = 0; z < issueHistory[j]["items"].size(); ++z)
            {
                // "fieldtype":"jira" # "fieldId":"status",
                if (issueHistory[j]["items"][z]["fieldtype"].asString() == "jira" &&
                    issueHistory[j]["items"][z]["fieldId"].asString() == "status")
                {
                    statuses.push_back({issueHistory[j]["created"].asString(),
                                        issueHistory[j]["items"][z]["fromString"].asString(),
                                        issueHistory[j]["items"][z]["toString"].asString()});
                }
            }

            issuesStates[allJiraIssues[i]["key"].asString()] = statuses;
        }
    }

    	
    std::string json_all_jira_statuses, json_done_statuses, json_start_work_statuses;
    p->getValue("jira_statuses",json_all_jira_statuses);
    p->getValue("done_statuses",json_done_statuses);
    p->getValue("start_work_statuses",json_start_work_statuses);

    DataPreparation prepdata(json_all_jira_statuses,
                                           json_done_statuses,
                                           json_start_work_statuses,
                                           issuesStates);
    //prepdata.dumpDictionaryData();
    //prepdata.dumpInnerReprData();

    //preapare data for data Cycle Time 
    std::vector<int> y_avarage;
    prepdata.getDataForCycleTime(y_avarage);
    //prepdata.drawCycleTime(y_avarage);
    prepdata.drawStatusesBar(y_avarage);
    return 0;
}
