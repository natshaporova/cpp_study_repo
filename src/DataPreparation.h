
#pragma once
#include <vector>
#include <map>
#include <string>


namespace jirareports
{

    class DataPreparation
    {
        //Dictionaries
        std::map<std::string, int> dict_Jira_Statuses ;
        std::vector<int> id_DoneStatuses;
        std::vector<int> id_StartWorkStatuses;

        //Inner represenation
        std::vector<std::string> namesOfJiraTasks;
        std::vector<std::vector<int>> timeTaskInStatusesVec2d;

        //Auxiliary representation
        std::vector<std::string> namesDoneIssues;
        std::vector<int> idDoneIssues;


        int numOfdays(std::string &day1, std::string &day2, int &numdays);

        void parseString(std::string& statuses,
                            std::vector<std::string>& result);
        static bool compareIntStrPair(std::pair<std::string,int>& a1,std::pair<std::string,int>& a2);
        void sortMap(std::map<std::string, int>& M,int min,int max,std::vector<std::string>& result);
        //create inner representation
        void createTimeInStatusesVec2d(std::map<std::string, std::vector<std::array<std::string,3>>>& rawTimeStatusesData);

    public:
        DataPreparation(std::string& statuses,
                        std::string& done_statuses,
                        std::string& start_statuses,
                        std::map<std::string, std::vector<std::array<std::string,3>>>& rawTimeStatusesData);

         
        void dumpInnerReprData();
        void dumpDictionaryData();
        void dumpDoneId();
        void getDoneIssues();
  
        void drawCycleTime(std::vector<int>& Y_val);
        void getDataForCycleTime(std::vector<int>& Y_val);
        void drawStatusesBar(std::vector<int>& Y_val);
};
}
