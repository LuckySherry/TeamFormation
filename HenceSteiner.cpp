#include "author.h"
#include "csv.h"
#include "edges.h"
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <set>
#include <map>
#include <vector>
#include <iterator>
#include <cmath>
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
	const unsigned short REQUIRED_SKILL_NUM = 20;
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SKILL_PATH = "E:\\ProgramData\\Java\\DBLP\\20130730\\_";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;
	map<string,author_skill*> AUTHORSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	output.open(PATH+"resultHS\\"+"result_"+ConvertToString( REQUIRED_SKILL_NUM)+SUFFIX,ios::out|ios::binary|ios::trunc);
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
		while(TASK.size()!=REQUIRED_SKILL_NUM)
		{
			int required_skill_index = rand()%SKILLSET.size();
			cout<<required_skill_index<<endl;
			SKILLSET[required_skill_index]->isCover = false;
			TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
		}
		map<string,author_skill*> AUTHORSET;
		//vector<string> HENCECOVER;
		map<string,map<string,double> > HENCECOVER;
		vector<string> GREEDYCOVER;
		for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
		{
			for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
			{
				if(*it_holder=="")
					continue;
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
			vector<string> row;
			string line;
			map<string,double> shortestDist;
			ifstream skillPathFile;
			string p = SKILL_PATH+it_task->first+SUFFIX;
			//map<string,shortest*> holderRoad;			//use for path, line 9 in the algorithm
			//shortest* way = 
			skillPathFile.open(p.c_str(),ios::in|ios::binary);
			clock_t read_start = clock();
			if(skillPathFile.fail())
			{
				cout<<it_task->first<<"OPEN FAIL"<<endl;
				return 0;
			}
			while(getline(skillPathFile,line),skillPathFile.good())
			{
				csvRead::csvline_populate(row,line,',');
				if(row[1]=="DISCONNECTED")
					shortestDist.insert(make_pair(row[0],30000));
				else
				{
					shortestDist.insert(make_pair(row[0],atof(row[1].c_str())));
				}
			}
			skillPathFile.close();
			file_time+=clock()-read_start;
			HENCECOVER.insert(make_pair("_"+it_task->first,shortestDist));
			GREEDYCOVER.push_back("_"+it_task->first);
		}
		cout<<HENCECOVER.size()<<endl;

		vector<string> X_PREM;
		int randIndex = rand()%HENCECOVER.size();
		
		X_PREM.push_back(GREEDYCOVER[randIndex]);
		GREEDYCOVER.erase(GREEDYCOVER.begin()+randIndex);
		ifstream holderPathFile;
		map<string,string> holderMap;

		string i_star;
		
		bool fail=false;
		double cost_MST=0;
		vector<string> printSolution;
		while(GREEDYCOVER.size()!=0)
		{
			/*
			*
			* Find the nearest vertex to X_PREM among HENCECOVER
			*
			*/
			clock_t read_start = clock();
			double R_i=999999;
			string mediator;
			string destination;
			
			for(vector<string>::iterator it_name = GREEDYCOVER.begin();it_name!=GREEDYCOVER.end();++it_name)
			{
				
				double R_ia = 99999999;
				for(vector<string>::iterator it_X_PREM =X_PREM.begin();it_X_PREM!=X_PREM.end();++it_X_PREM)
				{
					if(it_X_PREM->at(0)=='_')
					{
						for(map<string,double>::iterator Hdex = HENCECOVER[*it_name].begin();Hdex!=HENCECOVER[*it_name].end();++Hdex)
						{
							if(HENCECOVER[*it_name][Hdex->first] !=30000.0 &&HENCECOVER[*it_X_PREM][Hdex->first]!=30000.0)
							{
								if (HENCECOVER[*it_name][Hdex->first]==9999.0&&HENCECOVER[*it_name][Hdex->first]==HENCECOVER[*it_X_PREM][Hdex->first])
								{
									if(9999.0*2<R_ia)
									{
										R_ia = 9999.0*2;
										if(R_ia<R_i)
										{
											R_i = R_ia;
											mediator = Hdex->first;
											destination = *it_X_PREM;
											i_star = *it_name;
										break;
										}
									}
								}
								else
								{
									if(HENCECOVER[*it_name][Hdex->first]+HENCECOVER[*it_X_PREM][Hdex->first]<R_ia)
									{
										R_ia = HENCECOVER[*it_name][Hdex->first]+HENCECOVER[*it_X_PREM][Hdex->first];
										if(R_ia<R_i)
										{
											R_i = R_ia;
											mediator = Hdex->first;
											destination = *it_X_PREM;
											i_star = *it_name;
										}										
									}
								}
							}
						}
					}
					else
					{
						if(HENCECOVER[*it_name][*it_X_PREM]<30000.0)
						{
							if(HENCECOVER[*it_name][*it_X_PREM]==9999.0)
							{
								R_ia = 9999.0;
								if(R_ia<R_i)
								{
									R_i = R_ia;
									i_star = *it_name;
									mediator = *it_X_PREM;
								}
								break;
							}
							else if(HENCECOVER[*it_name][*it_X_PREM] <R_ia)
							{
								R_ia = HENCECOVER[*it_name][*it_X_PREM];
							}
						}
					}
					if(R_ia<R_i)
					{
						R_i = R_ia;
						i_star = *it_name;
						if(it_X_PREM->at(0)!='_')
						{
							mediator = *it_X_PREM;
							destination = "";
						}

						else
							destination = *it_X_PREM;
					}

				}
				if(R_i==9999.0)
					break;
			}
			double dist = 0;
			
			

			if(destination=="")//shortest distance is from a skill vertex to an author
			{
				string p = SKILL_PATH+i_star.substr(1,i_star.size())+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);

				string line;
				vector<string> row;
				set<string> solution;

				read_start = clock();
				while(getline(holderPathFile,line),holderPathFile.good())
				{
					csvRead::csvline_populate(row,line,',');
					if(row[0] != mediator)
						continue;
					if(row[1]!="DISCONNECTED")
					{
						if(row.size()>2&&row[2]!="")
						{
							solution.insert(row[2]);
							printSolution.push_back(row[2]);
							if(row.size()!=3&&row[row.size()-1]!="")
							printSolution.push_back(row[row.size()-1]);

						}

						for(int i = 3;i<(row.size()-1)&&row[i]!="";i++)
						{
							solution.insert(row[i]);
							printSolution.push_back(row[i]);
						}
					}
				}	
				holderPathFile.close();
				file_time+=clock()-read_start;
				X_PREM.push_back(i_star);
				if(!solution.empty())
				{
					for(set<string>::iterator dex=solution.begin();dex!=solution.end();++dex)
					{
						if(find(X_PREM.begin(),X_PREM.end(),*dex)==X_PREM.end())
							X_PREM.push_back(*dex);
					}
					
				}
				else
				{
					fail = true;
					break;
				}
				dist = R_i-9999.0;
			}
			else			//shortest dist is from skill vertex to skill vertix
			{
				string p = SKILL_PATH+i_star.substr(1,i_star.size())+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);

				string line;
				vector<string> row;
				set<string> solution;
				read_start = clock();
				solution.insert(mediator);
				printSolution.push_back(mediator);
				while(getline(holderPathFile,line),holderPathFile.good())
				{
					csvRead::csvline_populate(row,line,',');
					if(row[0] != mediator)
						continue;
					if(row[1]!="DISCONNECTED")
					{
						if(row.size()>2&&row[2]!="")
						{
							solution.insert(row[2]);
							printSolution.push_back(row[2]);
							if(row.size()!=3&&row[row.size()-1]!="")
							printSolution.push_back(row[row.size()-1]);
						}

							
						for(int i = 3;i<(row.size()-1)&&row[i]!="";i++)
						{
							solution.insert(row[i]);
							printSolution.push_back(row[i]);
						}
						break;
					}
				}
				holderPathFile.close();
				file_time+=clock()-read_start;
				if(!solution.empty())
				{
					for(set<string>::iterator dex=solution.begin();dex!=solution.end();++dex)
					{
						if(find(X_PREM.begin(),X_PREM.end(),*dex)==X_PREM.end())
							X_PREM.push_back(*dex);
					}
				}
				else
				{
					fail = true;
					break;
				}

				p = SKILL_PATH+destination.substr(1,destination.size())+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
				read_start=clock();
				while(getline(holderPathFile,line),holderPathFile.good())
				{
					csvRead::csvline_populate(row,line,',');
					if(row[0] != mediator)
						continue;
					if(row[1]!="DISCONNECTED")
					{
						if(row.size()>2&&row[2]!="")
						{
							solution.insert(row[2]);
							printSolution.push_back(row[2]);
							if(row.size()!=3&&row[row.size()-1]!="")
							printSolution.push_back(row[row.size()-1]);
						}
						for(int i = 3;i<(row.size()-1)&&row[i]!="";i++)
						{
							solution.insert(row[i]);
							printSolution.push_back(row[i]);
						}
						break;
					}
				}
				holderPathFile.close();
				file_time+=clock()-read_start;
				if(!solution.empty())
				{
					for(set<string>::iterator dex=solution.begin();dex!=solution.end();++dex)
					{
						if(find(X_PREM.begin(),X_PREM.end(),*dex)==X_PREM.end())
							X_PREM.push_back(*dex);
					}
				}
				else
				{
					fail = true;
					break;
				}
				dist = R_i-9999.0*2;
			}	
			for(vector<string>::iterator it = GREEDYCOVER.begin();it!=GREEDYCOVER.end();++it)
			{
				if(*it==i_star)
				{
					GREEDYCOVER.erase(it);
					break;
				}
			}
			cost_MST+= dist;
		}
		vector<string> final_solution;
		for(vector<string>::size_type i = 0;i!=printSolution.size();i++)
		{
			if(printSolution[i].at(0)!='_')
				final_solution.push_back(printSolution[i]);
		}
		if(fail)
		{
			continue;
		}
		total_time = clock()-begin_time-file_time;

		string p_a_t_h="";
		string s_k_i_l_l="";
		for(vector<string>::iterator i_X_P = final_solution.begin();i_X_P!=final_solution.end();++i_X_P)
		{
			p_a_t_h+=*i_X_P+",";
		}

		for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<<"SKILL:"<<it_skill->first<<endl;
				s_k_i_l_l+=it_skill->first+",";
				
			}
		cout<<"Computational time: "<<total_time<<endl;
		cout <<cost_MST<<endl;
		if(cost_MST>=0)
		output<<i_star<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost_MST<<endl;
		

	}
	output.close();
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