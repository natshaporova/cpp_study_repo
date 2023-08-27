//Local
#include "DataPreparation.h"

//external
#include "boost/date_time/gregorian/gregorian.hpp"

// Attention! algorithm is a header only library 
// Boost:: algorith needs only header for Boost in CMake file

#include "boost/algorithm/string.hpp"
#include "matplot/matplot.h"

//standard

#include <cstddef>
#include <regex>
#include <string>


namespace jirareports
{

void DataPreparation::parseString(std::string& statuses,
                            std::vector<std::string>& result)
{   
    boost::trim(statuses);
 
    std::string tmp{};
    int strlength =statuses.length();
    if (strlength >= 2 
      && statuses[0] == '\"' && statuses[--strlength] == '\"') {
            statuses=statuses.substr(1,--strlength);
        }
    strlength = statuses.length();

    int i = 0;

    while (strlength > 0){
       size_t pos = statuses.find(",");

        if (pos == std::string::npos)
        {   
            tmp = statuses;
            strlength = 0;

        }
        else
        { 
            if ((pos+1) == strlength){
                tmp = statuses.substr(0,pos);
                strlength=0;
            }
            if (pos > 0){
                tmp = statuses.substr(0,pos);
            
                statuses = statuses.substr(pos+1,strlength);
                strlength = statuses.length();
            }
        }
        boost::trim(tmp);

        if (tmp.length()!=0)
            result.push_back(tmp);
    }


}

DataPreparation::DataPreparation(std::string& all_statuses, // all jira statuses from config_file
                        std::string& done_statuses, // done jira statuses
                        std::string& start_statuses, // start jira statuses
                        std::map<std::string, std::vector<std::array<std::string,3>>>& rawTimeStatusesData)
                        
{
    // prepare dict_Jira_Statuses
    std::vector<std::string> parseresult;
    parseString(all_statuses,parseresult);

    for (int i=0; i< parseresult.size(); ++i){
        dict_Jira_Statuses[parseresult[i]]=i;
    }

    parseresult.clear();
    
    //  prepare    std::vector<int> id_DoneStatuses;
    parseString(done_statuses,parseresult);
    for (int i=0; i< parseresult.size(); ++i){
        id_DoneStatuses.push_back(dict_Jira_Statuses[parseresult[i]]);
    }
    parseresult.clear();

    //  prepare    std::vector<int> id_StartWorkStatuses;
    parseString(start_statuses,parseresult);
    for (int i=0; i< parseresult.size(); ++i){

        id_StartWorkStatuses.push_back(dict_Jira_Statuses[parseresult[i]]);
    }

    // create inner representation 
    createTimeInStatusesVec2d(rawTimeStatusesData);
    getDoneIssues();
   
}



int DataPreparation::numOfdays(std::string &day1, std::string &day2, int &numdays)
{

    auto subRowString = [](std::string &rawday) -> bool
    {
        size_t posT = rawday.find("T");
        if (posT != std::string::npos)
        {
            rawday = rawday.substr(0, posT);

            return true;
        }
        std::regex const re("([0-9]+)-([0-9]+)-([0-9]*)?");
        if (std::regex_search(rawday, re))
        {
            return true;
        }
        return false;
    };

    if (subRowString(day1) && subRowString(day2))
    {
        boost::gregorian::date d1(boost::gregorian::from_string(day1));
        boost::gregorian::date d2(boost::gregorian::from_string(day2));

        numdays = (d2 - d1).days();

        return 0;
    }

    return 1;
}

void DataPreparation::createTimeInStatusesVec2d(std::map<std::string, std::vector<std::array<std::string, 3>>>& rawTimeStatusesData)
{

    /* the function will devide the map of row data
      tvo 3 parts:
      - list of names of jira tasks;
      - (already prepared list of statuses);
      - array with times for all tasks


       std::map<std::string, std::vector<std::array<std::string,3>>>& rawData;
       const std::vector<std::string> dict_Jira_Statuses = {
       "Open","In Progress","Blocked","Postponed","Review","Review II",
       "Ready to Merge", "Ready for Release","Resolved","In Beta","In Rollout","Done"
       };
       std::vector<std:string> namesOfJiraTasks;
       std::vector<std::vector<int>> timeTaskInStatusesVec2d;

   */
    // position_number to indicate the relationship between dict_Jira_Statuses and
    // namesOfJiraTasks and timeTaskInStatuses;
    // for example, currentPos =1
    // that is mean that name of jirs task is saved in the namesOfJiraTasks in the 1 position
    // and in the  timeTaskInStatusesArray the row with number 1 correspond the values for
    // any status which we described in the dict_Jira_Statuses

    for (auto rawItem = rawTimeStatusesData.begin(); rawItem != rawTimeStatusesData.end(); ++rawItem)
    {
        namesOfJiraTasks.push_back(rawItem->first);

        // prepare the timeTaskInStatusesArray row with number currentPos
        std::vector<std::array<std::string, 3>> recordStatuses = rawItem->second;
        std::vector<std::array<std::string, 3>>::iterator item;

        int numStatuses = dict_Jira_Statuses.size();

        std::vector<int> timeInStatuses(numStatuses, -1);

        std::string from_ = "", to_ = "", datetime = "";
        int num = 0;

        // array <created, from, to>
        for (int i = recordStatuses.size() - 1; i >= 0; --i)
        {
            // std::cout <<" i :"<<i<< " from::"<<from_ <<"  "<<"to::"<<to_<<std::endl;
            // std::cout <<" -- from_array 1 "<<recordStatuses[i][1] << ":: 2" <<recordStatuses[i][2] <<std::endl;

            if (i == recordStatuses.size() - 1)
            {
                int statusPos = dict_Jira_Statuses[recordStatuses[i][1]];
                timeInStatuses[statusPos] = 0;
            }
            if (i == 0)
            {
                int statusPos = dict_Jira_Statuses[recordStatuses[i][2]];
                timeInStatuses[statusPos] = 0;
            }

            if (to_ == recordStatuses[i][1])
            {

                if (numOfdays(datetime, recordStatuses[i][0], num) == 0)
                {
                    // get the Position of status in array
                    int statusPos = dict_Jira_Statuses[recordStatuses[i][1]];
                    // std::cout << " ---"<<statusPos<< std::endl;

                    if (timeInStatuses[statusPos] == -1)
                    {
                        timeInStatuses[statusPos] = num;
                    }
                    else
                    {
                        timeInStatuses[statusPos] += num;
                    }
                }
                else
                {
                    std::cout << "problems with status data";
                }
            }

            datetime = recordStatuses[i][0];
            from_ = recordStatuses[i][1];
            to_ = recordStatuses[i][2];
        }

        timeTaskInStatusesVec2d.push_back(timeInStatuses);
    }
}

void DataPreparation::dumpInnerReprData()
{
    // timeTaskInStatusesVec2d -time n statuses;
    // namesOfJiraTasks - jira tasks;
    // const std::map<std::string,int> dict_Jira_Statuses;

    for (int i = 0; i < namesOfJiraTasks.size(); ++i)
    {
        std::cout << "\n\ntask " << namesOfJiraTasks[i] << std::endl;
        std::map<std::string, int>::iterator iter;
        for (iter = dict_Jira_Statuses.begin(); iter != dict_Jira_Statuses.end(); ++iter)
        {
            std::cout << iter->first << " : ";
            std::cout << timeTaskInStatusesVec2d[i][iter->second] << std::endl;
        }
    }
}

void DataPreparation::dumpDictionaryData(){
    // print std::map<std::string, int> dict_Jira_Statuses ;
    // std::vector<int> id_DoneStatuses;
    // std::vector<int> id_StartWorkStatuses;

    std::map<std::string, int>::iterator it;
    std::cout << "JIra statuses dictionary" << std::endl;
    for( it = dict_Jira_Statuses.begin(); it!= dict_Jira_Statuses.end();++it){
        std::cout << it->first <<" : "<<it->second <<std::endl; 
    }
    std::cout << "Id of Done Statuses " << std::endl;
    for( int i=0; i< id_DoneStatuses.size(); ++i){
        std::cout << id_DoneStatuses[i] <<std::endl;
    }

    std::cout << "Id of Start Work Statuses " << std::endl;
    for( int i=0; i< id_StartWorkStatuses.size(); ++i){
        std::cout << id_StartWorkStatuses[i] <<std::endl;
    }




}

void DataPreparation::dumpDoneId()
{
    std::cout << "done statuses" << std::endl;
    for (int i = 0; i < idDoneIssues.size(); ++i)
    {
        std::cout << idDoneIssues[i] << std::endl;
        std::cout << namesDoneIssues[i] << std::endl;
    }
}
/*
void DataPreparation::prepareStatusesId()
{
    // std::vector<std::string> id_DoneStatuses;
    // std::vector<std::string> id_StartWorkStatuses;
    for (int i = 0; i < dict_DoneStatuses.size(); ++i)
    {
        int tmpid = dict_Jira_Statuses[dict_DoneStatuses[i]];
        // std::cout << " " << tmpid <<std::endl;
        id_DoneStatuses.push_back(tmpid);
    }
    
    //for (int i = 0; i < dict_StartWorkStatuses.size(); ++i)
    //{
    //    id_StartWorkStatuses.push_back(dict_Jira_Statuses[dict_DoneStatuses[i]]);
    //}
    
}
*/
void DataPreparation::getDoneIssues()
{
    // get tasks, which have status one of this
    // std::vector<std::string> DoneStatuses -- number of Jira status, which we assume
    // as Done
    // std::vector<std::string> dict_StartWorkStatuses

    for (int i = 0; i < timeTaskInStatusesVec2d.size(); ++i)
    {
        for (int item : id_DoneStatuses)
        {
            if (timeTaskInStatusesVec2d[i][item] != -1)
            {
                std::cout << "--" << i << std::endl;
                std::cout << "size: " << timeTaskInStatusesVec2d.size() << std::endl;
                idDoneIssues.push_back(i);
                namesDoneIssues.push_back(namesOfJiraTasks[i]);
                break;
            }
        }
    }
}

void DataPreparation::getDataForCycleTime(std::vector<int> &Y_val)
{ // X values are the id of Done Issues - we already have them
    // Y values  are their sum of times being the statuses :
    // sum all statuses for the Done issues with num not equal -1;
    for (int i = 0; i < idDoneIssues.size(); ++i)
    {
        int sum = 0;
        for (int j = 0; j < timeTaskInStatusesVec2d[0].size(); ++j)
        {
            if (timeTaskInStatusesVec2d[i][j] != -1)
            {
                sum += timeTaskInStatusesVec2d[i][j];
            }
        }
        Y_val.push_back(sum);
    }
}

void DataPreparation::drawCycleTime(std::vector<int> &Y_val)
{
    // std::vector<int> tmp (idDoneIssues.size(),0);
    // matplot::vector_1d ticks = std::iota(tmp.begin(),tmp.end(),0);
    matplot::plot(idDoneIssues, Y_val, "o");
    // for (int i=0; i<idDoneIssues.size();++i){
    matplot::text(idDoneIssues, Y_val, namesDoneIssues);

    //}
    // Json::Value v;
    // fromVectToJson(v,namesDoneIssues);
    // matplot::xticks(ticks);
    // matplot::xticklabels(v.getMemberNames());

    matplot::show();
}

bool DataPreparation::compareIntStrPair(std::pair<std::string, int> &a1,
                                        std::pair<std::string, int> &a2)
{

    return a1.second < a2.second;
}
void DataPreparation::sortMap(std::map<std::string, int> &M,
                              int min, int max, std::vector<std::string> &result)
{
    // create vector of pairs <string::int>
    // sort vector
    // form the result vector
    std::vector<std::pair<std::string, int>> tmpvect;
    for (auto &pair : M)
    {
        tmpvect.push_back(pair);
    }
    std::sort(tmpvect.begin(), tmpvect.end(), compareIntStrPair);

    std::vector<std::pair<std::string, int>>::iterator iit;
    for (iit = tmpvect.begin(); iit != tmpvect.end(); ++iit)
    {
        if (iit->second >= min && iit->second <= max)
            result.push_back(iit->first);
    }
}
void DataPreparation::drawStatusesBar(std::vector<int> &Y_val)
{
    // Y_val - this is the cycle time for earch value
    // Y - this is the vector for costructing bar chart
    // Y_merge - this is the vector for merging
    std::vector<std::vector<double>> Y;
    std::vector<std::tuple<int, int>> mergedIdIssueTimeStatus;
    // merge the time_in_statuses vector with tasks_id to sort
    for (int i = 0; i < Y_val.size(); ++i)
    {
        mergedIdIssueTimeStatus.push_back(std::make_tuple(idDoneIssues[i], Y_val[i]));
    }
    std::sort(mergedIdIssueTimeStatus.begin(), mergedIdIssueTimeStatus.end(),
              [](std::tuple<int, int> &a1, std::tuple<int, int> &a2)
              {
                  return std::get<1>(a1) > std::get<1>(a2);
              });
    /*
    we know, that our work is lasted
    from
    dict_StartWorkStatuses (min value)
    to dict_DoneStatuses (max value)
    so we build the vector of arrays
    array1 [In progress_for_issue1, In progress_for_issue2, ...]
    array2 [In Review_for_issue1, In Review_for_issue2, ...]
    */
    std::vector<std::vector<double>> YbarValues;
    std::vector<std::string> XbarValues;
    std::vector<std::string> nameOfStatuses;
    std::vector<int> idStatuses; // what statuses we analyze to build bar(In Progress, In Review, etc)
    // take min value [earliest status] for start

    int start, end; // id statuses to analyze
    std::string nmBegin, nmEnd;
    for (int i = 0, tmp; i < id_StartWorkStatuses.size(); ++i)
    {
        tmp = id_StartWorkStatuses[i];
        if (i == 0 || start > tmp)
        {
            start = tmp;
            nmBegin = id_StartWorkStatuses[i];
        }
    }

    // take max value [lates status] for finish
    for (int i = 0, tmp; i < id_DoneStatuses.size(); ++i)
    {
        tmp = id_DoneStatuses[i];
        if (i == 0 || end < tmp)
        {
            end = tmp;
            nmEnd = id_DoneStatuses[i];
        }
    }

    std::cout << "start status : " << start << std::endl;
    std::cout << "end status : " << end << std::endl;
    // collect name of statuses for legend
    sortMap(dict_Jira_Statuses, start, end, nameOfStatuses);

    for (int h = start; h < end; ++h)
    {
        std::cout << "status :" << h << std::endl;
        std::vector<double> tmpIssuesValuesInStatus;
        for (int i = 0; i < mergedIdIssueTimeStatus.size(); ++i)
        {
            int idtask = std::get<0>(mergedIdIssueTimeStatus[i]);
            std::cout << "----issue id:" << idtask << std::endl;
            // timeTaskInStatusesVec2d [i][j];
            //  i - id task, j - id status
            int timeInStatus = timeTaskInStatusesVec2d[idtask][h];
            std::cout << "***" << timeInStatus << std::endl;
            if (timeInStatus == -1)
                timeInStatus = 0;
            else if (timeInStatus == 0)
                timeInStatus = 0.5;
            std::cout << "----timeInStatus:" << timeInStatus << std::endl;
            tmpIssuesValuesInStatus.push_back(timeInStatus);
        }
        YbarValues.push_back(tmpIssuesValuesInStatus);
    }

    std::cout << "\n Names:";
    for (int i = 0; i < mergedIdIssueTimeStatus.size(); ++i)
    {
        XbarValues.push_back(namesOfJiraTasks[std::get<0>(mergedIdIssueTimeStatus[i])]);
        std::cout << namesOfJiraTasks[std::get<0>(mergedIdIssueTimeStatus[i])];
    }

    matplot::barstacked(YbarValues);

    std::vector<std::vector<double>> y = {
        {10, 30, 50}, {15, 35, 55}, {20, 40, 62}};
    auto b = matplot::bar(y);
    b->face_colors()[2] = {0.f, .2f, .6f, .5f};
    

    //face_colors()[12] = {0.f, .2f, .6f, .5f, .7f, .4f, .3f, .76f, .49f, .43f, .65f, .36f};
    matplot::gca()->x_axis().ticklabels(XbarValues);
    matplot::legend(nameOfStatuses);
    matplot::show();
    /*
    std::cout<< "Check statuses array:"<< std::endl;
    for (auto k : nameOfStatuses){
        std::cout << k << std::endl;
    }
    std::cout << "My map for statuses:" << std::endl;
    std::map<std::string, int>::iterator it1;

    for (it1=dict_Jira_Statuses.begin(); it1!=dict_Jira_Statuses.end(); ++it1 ){
        std::cout << "first :: "<< it1->first <<std::endl;
        std::cout << "second :: " << it1->second <<std::endl;
    }
    */
}
}
