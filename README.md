# cpp_study_repo
The goal of my study project is to collect data about tasks statuses in the sprints and visualize it through charts.
What is already done for current moment.
- the main.cpp get commandline option --file_config to obtain the link to the jira config file
- class ConfigReader parse jiraconfig.txt
- via cpp library cpr the request to the jira is made
- via jsoncpp the result is parsed 
- obtained data is pushed to the DataPreparation class, which perfom data in some inner representation, which consist of three parts
	        status1  status2	
task1      t11      t12  
task2      t21      t22 
task3      t31      t32 

- these parts are: vector of statuses, vector of tasks, vector of vectors of how much time the task was in a particular status
- charts are built via the mathplotcpp



what is planed to do
- add other charts for deeper analysis
- save charts to file
- upload file to google disk
- execute the application once a week automatically
