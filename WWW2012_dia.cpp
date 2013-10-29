#include "author.h"
#include "csv.h"
#include "edges.h"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <ctime>
#include <sstream>
using namespace std;

class skill
{
public:
	skill(){};
	vector<string> holders;
	void push_back(string s){holders.push_back(s);}
	void setFreq(double d){frequency = d;}
	bool isCover;// false;
	double getFreq(){return frequency;}
	string getName(){return name;}
	void setName(string n){name = n;}
	string i_prem;	//d(i*,s(a)) = d(i*,i')
	//vector<string>::size_type index;
	skill(string name, double freq):name(name),frequency(freq){holders.clear();isCover=false;};
private:
	string name;
	
	double frequency;
};

class author_skill : public author
{
public:
	author_skill(string name = NULL):author(name){};
	vector<skill*> skill_set;
};

class shortest
{
public:
	vector<string> road;
	double weight;
	string owner;
	shortest(double weight=100,string name=NULL):weight(weight),owner(name){};
};

void getskills(const float high,const float low ,vector<skill*>& collection);
void getIndex(map<string,vector<skill*>::size_type>& index);
double getWorkLoad(const int max_workload,int* workload);

template <class T> 
string ConvertToString(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}

int main()
{
	const float FREQ_THRESHOLD_HIGH = 0.05;
	const float FREQ_THRESHOLD_LOW = 0.001;
	const unsigned int NUM_OF_TASK = 100;
	//{2,4,6,8,10,12,14,16,18,20};
	int num[] = {2,4,6,8,10,12,14,16,18,20};
	const double UPPERBOUND = 2135.06;
	const double BS_UpperBound[] = {3.60,5.45,5.96,6.17,6.43,6.66,7.00,7.14,7.40,7.45};
	
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SKILL_PATH = "E:\\ProgramData\\Java\\DBLP\\20130730\\_";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	int max_workload = 2;
	const double lambda[] = {0.0001,0.0004,0.0016,0.0064,0.0256,0.1024,0.4096,1.6384,6.4};
	for(int lambda_index = 0;lambda_index<9;lambda_index++)
	{
		int workload [7159]={0};
		output.open(PATH+"resultWWW12_3\\"+"result_"+ConvertToString(lambda_index)+SUFFIX,ios::out|ios::binary|ios::trunc);
		/*
		* read skill index
		*/
		string pline;
		vector<string> prow;
		ifstream input (PATH+"resultWWW12_1\\"+"skill_2_8"/*+ConvertToString(lambda_index)*/+SUFFIX,ios::binary);
		map<int,vector<int> > task_list;
		int listDex = 0;
		while(getline(input,pline),input.good())
		{
			csvRead::csvline_populate(prow,pline,',');
			vector<int> vec;
			for(vector<string>::iterator it = prow.begin();	it!=prow.end()&&*it!="";it++)
			{
				int required_skill_index = atoi(it->c_str());
				vec.push_back(required_skill_index);
			}
			task_list[listDex++] = vec;
		}
		input.close();
		for(unsigned int times=0;times!=NUM_OF_TASK;++times)
		{
			clock_t begin_time = clock();
			clock_t total_time = 0L;
			clock_t file_time = 0L;
			cout<<times+1<<endl;
			map<string,skill*> TASK;
			srand(time(0)+times);
			skill* rarest_skill;
			set<string> TASK_HOLDER_SET;
			double currentMinFreq = 1;
			short pos = rand()%10;
			for(vector<int>::iterator i_Task = task_list[times].begin();i_Task!=task_list[times].end();i_Task++)
			{
				int required_skill_index = *i_Task;
				SKILLSET[required_skill_index]->isCover = false;
				TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
				if(SKILLSET[required_skill_index]->getFreq()<currentMinFreq)
				{
					currentMinFreq = SKILLSET[required_skill_index]->getFreq();
					rarest_skill = SKILLSET[required_skill_index];
				}
			}
			ifstream holderPathFile;
			map<string,string> holderMap;
			double DIA=0;
			string i_star;
			clock_t read_start = clock();
			set<string> T;
			double teamCost = 99999;
			map<string,author_skill*> AUTHORSET;
			int teamChangeTimes = 0;
			for(vector<string>::iterator it_name = rarest_skill->holders.begin();it_name!=rarest_skill->holders.end();it_name++)
			{
				//for loop to find the min(R_i), line 8 in the algorithm;
				string p = PATH+*it_name+SUFFIX;
				//cout<<p;
				vector<string> row;
				string line;
				map<string,double> shortestDist;
				map<string,vector<string> >UNIVERSE_PATH;

				//map<string,shortest*> holderRoad;			//use for path, line 9 in the algorithm
				//shortest* way = 
				read_start = clock();
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
				if(holderPathFile.fail())
				{
					p = PATH + ConvertToString(index[*it_name])+SUFFIX;
					holderPathFile.open(p.c_str(),ios::in|ios::binary);
				}
				if(holderPathFile.fail())
				{
					cout<<*it_name<<"OPEN FAIL"<<endl;
					return 0;
				}
				while(getline(holderPathFile,line),holderPathFile.good())
				{
					csvRead::csvline_populate(row,line,',');
					/*if(row[1]=="DISCONNECTED")
						shortestDist.insert(make_pair(row[0],99999));
					else
					{*/
					if(row[1]!="DISCONNECTED")//&&atof(row[1].c_str())<BS_UpperBound[TASK.size()/2-1])
					{
						shortestDist.insert(make_pair(row[0],atof(row[1].c_str())));
						vector<string> vs;
						for(int vs_dex = 2;vs_dex<row.size()-1&&row[vs_dex]!="";vs_dex++)
						{
							vs.push_back(row[vs_dex]);
						}
						UNIVERSE_PATH.insert(make_pair(row[0],vs));
					}
				}
				holderPathFile.close();
				read_start = clock()-read_start;
				file_time += read_start;

				AUTHORSET.clear();
				author_skill* root = new author_skill(*it_name);
				AUTHORSET.insert(make_pair(*it_name,root));

				for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				{
					it_task->second->isCover =false;
					for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
					{
						if(*it_holder=="")
							continue;
						if(*it_holder==*it_name)
							AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
						if(shortestDist.find(*it_holder)!=shortestDist.end())
						{
							if(AUTHORSET.find(*it_holder)==AUTHORSET.end())
							{
								author_skill* au = new author_skill(*it_holder);
								AUTHORSET.insert(make_pair(*it_holder,au));
								AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
							}
							else
							{
								AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
							}
						}
					}

				}
				cout<<"AUTHOR SIZE:"<<AUTHORSET.size()<<endl;
				



				bool RET = false;
				vector<string> Q;
				Q.push_back(*it_name);
				for(vector<skill*>::iterator it = AUTHORSET[*it_name]->skill_set.begin();it!=AUTHORSET[*it_name]->skill_set.end();++it)
				{
					(*it)->isCover=true;
				}
				AUTHORSET.erase(*it_name);

				while(!RET&&AUTHORSET.size()>0)
				{
					set<string> skillToBeCovered;
					int cover = 0;
					double mostCostEffective = 0;
					double benefit=99999;
					vector<string> SKILL;
					string auth;
					string i_star;
					map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();
					while(it_au_s!=AUTHORSET.end())
					{
						set<string> skillToBeCovered;
						
						int cover = 0;
						for(vector<skill*>::iterator it_skill=it_au_s->second->skill_set.begin();it_skill!=it_au_s->second->skill_set.end();++it_skill)
						{
							if (!(*it_skill)->isCover)
							{
								cover++;
								skillToBeCovered.insert((*it_skill)->getName());
							}
						}
						bool fail=false;
						if(cover==0)
						{
							it_au_s=AUTHORSET.erase(it_au_s);
							continue;
						}
						double cost =shortestDist[it_au_s->first];

						double dist = 9999;
						set<edges> graph;
						for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first].begin();it_sol!=UNIVERSE_PATH[it_au_s->first].end();it_sol++)
						{
							vector<string>::iterator position = find(Q.begin(),Q.end(),*it_sol);
							if(position==Q.end())
							{
								int* data = new int[3];
								data[0] = workload[index[*it_sol]];
								data[1] = TASK.size();
								data[2] = index.size();
								cost+=lambda[lambda_index]* getWorkLoad(max_workload,data);
								delete data;
							}
						}
						if(cost/skillToBeCovered.size()<benefit)
						{
							//cout<<"lambda_1: "<< lambda_1<<" workload_cost: "<<workload_cost<<endl;
							benefit = cost/skillToBeCovered.size();
							//cout<<"benefit: "<<benefit<<endl;
							auth = it_au_s->first;
							SKILL.clear();
							for(set<string>::iterator it = skillToBeCovered.begin();it!=skillToBeCovered.end();++it)
							{
								SKILL.push_back(*it);
							}
						}
						++it_au_s;
					}
					Q.push_back(auth);
					//cout<<"workload[index[auth]] "<<workload[index[auth]]<<endl;
					AUTHORSET.erase(auth);
					for(vector<string>::iterator it_sol = UNIVERSE_PATH[auth].begin();it_sol!=UNIVERSE_PATH[auth].end();it_sol++)
					{
						vector<string>::iterator position = find(Q.begin(),Q.end(),*it_sol);
						if(position==Q.end())
						{
							Q.push_back(*it_sol);
							//cout<<"workload[index[*it_sol]] "<<workload[index[*it_sol]]<<endl;
						}
						if(AUTHORSET.find(*it_sol)!=AUTHORSET.end())
							AUTHORSET.erase(AUTHORSET.find(*it_sol));
					}
				
					for(vector<string>::iterator it = SKILL.begin();it!=SKILL.end();++it)
					{
						TASK[*it]->isCover = true;
					}
					RET = true;
					for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
					{
						if(!it_task->second->isCover)
						{
							RET = false;
							break;
						}
					}

				}
				double TC = 0;
				double shortest_distance = 0;
				set<string> Q_T;
				if(RET)
				{
					for(vector<string>::iterator it_Q = Q.begin();it_Q!=Q.end();++it_Q)
					{
						Q_T.insert(*it_Q);
					}

					for(set<string>::iterator it_Q = Q_T.begin();it_Q!=Q_T.end();++it_Q)
					{
						int* data = new int[3];
						data[0] = workload[index[*it_Q]];
						data[1] = TASK.size();
						data[2] = index.size();
						TC+=getWorkLoad(max_workload,data);
						delete data;
						if(shortestDist.find(*it_Q)!=shortestDist.end()&&shortestDist[*it_Q]>shortest_distance)
							shortest_distance=shortestDist[*it_Q];
					}
					if(TC<teamCost&&TC!=0)
					{
						T.clear();
						cout<<"teamChangeTimes: "<<++teamChangeTimes<<" "<<TC<<endl;
						for(vector<string>::iterator it_Q = Q.begin();it_Q!=Q.end();++it_Q)
							T.insert(*it_Q);

						DIA = shortest_distance;
						teamCost = TC;
					}
				}
			}
			total_time = clock()-begin_time-file_time;
			

			string p_a_t_h="";
			string s_k_i_l_l="";
			for(set<string>::iterator i_X_P = T.begin();i_X_P!=T.end();++i_X_P)
			{
				p_a_t_h+=*i_X_P+",";
				++workload[index[*i_X_P]];
			}
			
			double max_personal_workload=0;

			for(int load_dex=0;load_dex<index.size();load_dex++)
			{
				if(max_personal_workload<workload[load_dex]&&workload[load_dex]<=100)
					max_personal_workload = workload[load_dex];
			}
			cout<<"max_personal_workload: "<<max_personal_workload<<endl;
			double A = 2*log(TASK.size()*1.0)*max_workload*log(2*index.size()*1.0);
			if(A<max_personal_workload)
				system("pause");

			for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<<"SKILL:"<<it_skill->first<<endl;
				s_k_i_l_l+=it_skill->first+",";
			}
			cout<<"Computational time: "<<total_time<<endl;
			cout <<teamCost<<endl;
			cout<<DIA<<endl;
			if(teamCost>0)
				output<<i_star<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<DIA<<","<<teamCost<<endl;
		}
		double max_personal_workload = 0;
		/*for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
			++workload[index[*it_NE]];
		*/
		int maxDex = 0;
		for(unsigned int dex =0;dex<index.size();dex++)
		{
			if(workload[dex]>max_personal_workload&&workload[dex]<=100)
			{
				max_personal_workload=workload[dex];
				maxDex = dex;
			}
			if(max_personal_workload>100)
				cout<<"FFFFFFFFFFFF"<<endl;
		}
		output<<max_personal_workload<<","<<maxDex<<endl;
		output.close();
	}
}

void getIndex(map<string,vector<skill*>::size_type>& index)
{
	vector<string> row;
	ifstream in("E:\\ProgramData\\Java\\DBLP\\20130725\\Test\\dblp_coauthor_l_r.csv000",ios::binary);
    if (in.fail())  { cout << "File not found" <<endl; return ; }
	unsigned int count=0;
	string line;
    while(getline(in, line)  && in.good() )
    {
        csvRead::csvline_populate(row, line, ',');

			index.insert(make_pair(row[0],count++));
    }
    in.close();
}

void getskills(const float high,const float low ,vector<skill*>& collection)
{
	ifstream readSkill ("E:\\ProgramData\\Java\\DBLP\\20130727\\Test\\skill_l.csv",ios::binary);
	vector<string> row;
	string line;
	vector<string>::size_type line_no=0;
	while(getline(readSkill,line)&&readSkill.good())
	{
		csvRead::csvline_populate(row,line,',');
		if(atof(row[1].c_str())<=high&&atof(row[1].c_str())>=low&&row[0].size()>1)
		{
			string name = row[0];
			if(name.find('/')<name.size())
				continue;
			double frequency = atof(row[1].c_str());
			skill *s = new skill(name,frequency);
			//s->index=line_no++;
			for(vector<string>::size_type i = 2;i!=row.size()&&row[i]!="";++i)
			{
				s->push_back(row[i]);
			}
			collection.push_back(s);
		}

	}
}

double getWorkLoad(const int max_workload,int*data)
{
	//data[0] workload
	//data[1] numofskills
	//data[2] numberofperson
	double A = 2*log(data[1]*1.0)*max_workload*log(2*data[2]*1.0);
	double EXPLoad;
	EXPLoad = pow(2*data[2]*1.0,(data[0]/A));
	return EXPLoad;
}