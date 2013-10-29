#include "author.h"
#include "csv.h"
#include "edges.h"
#include "Dinic.h"
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
#include <algorithm>
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

class Radius
{
public:
	string name;
	double dist;
	Radius(string n=NULL,double c=0):name(n),dist(c){};
	bool smaller(const Radius* v) const 
    { 
		return dist < v->dist; 
    } 
};

bool compare(const Radius* r1,const Radius* r2)
{
	return r1->smaller(r2);
}


void getskills(const float high,const float low ,vector<skill*>& collection);
void getIndex(map<string,vector<skill*>::size_type>& index);
void getCapacity(map<string,double>& capacity);

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
	const unsigned short REQUIRED_SKILL_NUM = 18;
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	map<string,double> capacity;
	getCapacity(capacity);
	map<string,double> E;
	vector<skill*> SKILLSET;
	map<string,author_skill*> AUTHORSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	output.open(PATH+"resultMAS\\"+"result_"+ConvertToString( REQUIRED_SKILL_NUM)+SUFFIX,ios::out|ios::binary|ios::trunc);
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
		vector<Radius*> radius;
		while(TASK.size()!=REQUIRED_SKILL_NUM)
		{
			int required_skill_index = rand()%SKILLSET.size();
			cout<<required_skill_index<<endl;
			SKILLSET[required_skill_index]->isCover = false;
			TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
		}
		map<string,author_skill*> AUTHORSET;
		vector<string> AUTHOR;
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
					AUTHOR.push_back(*it_holder);
				}
				else
				{
					AUTHORSET[*it_holder]->skill_set.push_back(it_task->second);
				}
			}

		}
		unsigned int discon=0;
		int ss = rand()%AUTHOR.size();
		string user_v = AUTHOR[ss];
		do{
			E.clear();
			E.insert(make_pair(user_v,0));
			vector<string> row;
			string line;
			ifstream holderPathFile;
			string p = PATH+user_v+SUFFIX;
			clock_t read_start = clock();
			holderPathFile.open(p.c_str(),ios::in|ios::binary);
			if(holderPathFile.fail())
			{
				p = PATH + ConvertToString(index[user_v])+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
			}
			if(holderPathFile.fail())
			{
				cout<<user_v<<"OPEN FAIL"<<endl;
				return 0;
			}
			while(getline(holderPathFile,line),holderPathFile.good())
			{
				csvRead::csvline_populate(row,line,',');
				if(AUTHORSET[row[0]]==NULL)
					continue;
				if(row[1]=="DISCONNECTED")
				{
					E.insert(make_pair(row[0],999));
					discon++;
				}
				else
					E.insert(make_pair(row[0],atof(row[1].c_str())));
			}
			holderPathFile.close();
			read_start = clock()-read_start;
			file_time += read_start;
			if(discon<capacity.size()/2)
				break;
			int ss = rand()&AUTHOR.size();
			user_v = AUTHOR[ss];
			discon=0;
		}while(true);
		
		vector<string> team;
		team.push_back(user_v);

		long long MAXITEMS = 0L;

		bool taskFail = false;

		do
		{
			Dinic din_before(REQUIRED_SKILL_NUM+team.size()+2);
			map<string,int> MAP_INDEX;
			unsigned int dex = 0;
			MAP_INDEX.insert(make_pair("_S",dex));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				MAP_INDEX.insert(make_pair(it_task->first,++dex));
			for(int pos = 0; pos<team.size();pos++)
				MAP_INDEX.insert(make_pair(team[pos],++dex));
			MAP_INDEX.insert(make_pair("_T",++dex));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				din_before.AddEdge(MAP_INDEX["_S"],MAP_INDEX[it_task->first],1);
			for(int pos = 0; pos<team.size();pos++)
				din_before.AddEdge(MAP_INDEX[team[pos]],MAP_INDEX["_T"],ceil(capacity[team[pos]]));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
			{
				for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
				{
					if(MAP_INDEX.find(*it_holder)!=MAP_INDEX.end())
						din_before.AddEdge(MAP_INDEX[it_task->first],MAP_INDEX[*it_holder],1);
				}
			}
			MAXITEMS = din_before.GetMaxFlow(MAP_INDEX["_S"],MAP_INDEX["_T"]);
			if(MAXITEMS==TASK.size())
				break;
			
			double ratio =0;
			long long MAXITEMS_X = 0L;
			string x_bar;
			for(map<string,double>::iterator it = E.begin();it!=E.end();++it)
			{
				if(find(team.begin(),team.end(),it->first)!=team.end())
					continue;
				if(it->second>900)
					continue;
				MAP_INDEX.erase("_T");
				dex = MAP_INDEX.size();
				
				MAP_INDEX.insert(make_pair(it->first,dex++));
				MAP_INDEX.insert(make_pair("_T",dex++));
				Dinic din_after(REQUIRED_SKILL_NUM+team.size()+3);
				for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
					din_after.AddEdge(MAP_INDEX["_S"],MAP_INDEX[it_task->first],1);
				for(int pos = 0; pos<team.size();pos++)
					din_after.AddEdge(MAP_INDEX[team[pos]],MAP_INDEX["_T"],ceil(capacity[team[pos]]));
				din_after.AddEdge(MAP_INDEX[it->first],MAP_INDEX["_T"],ceil(capacity[it->first]));
				for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				{
					for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
					{
						if(MAP_INDEX.find(*it_holder)!=MAP_INDEX.end())
							din_after.AddEdge(MAP_INDEX[it_task->first],MAP_INDEX[*it_holder],1);
					}
				}
				for(vector<skill*>::iterator it_skill = (AUTHORSET[it->first]->skill_set.begin());it_skill!=(AUTHORSET[it->first]->skill_set.end());++it_skill)
				{
					if(TASK.find((*it_skill)->getName())==TASK.end())
						continue;
					din_after.AddEdge(MAP_INDEX[(*it_skill)->getName()],MAP_INDEX[it->first],1);
				}

				MAXITEMS_X = din_after.GetMaxFlow(MAP_INDEX["_S"],MAP_INDEX["_T"]);
				if((MAXITEMS_X-MAXITEMS)*1.0/it->second>ratio)
				{
					ratio = (MAXITEMS_X-MAXITEMS)*1.0/it->second;
					x_bar = it->first;
				}
				MAP_INDEX.erase(it->first);
			}
			if(!x_bar.empty())
				team.push_back(x_bar);
			else
			{
				taskFail = true;
				cout<<"fail"<<endl;
				break;
			}
		}while(true);

		if(taskFail)
			continue;

		vector<string> X_PREM;
		int randIndex = rand()%team.size();
		X_PREM.push_back(team[randIndex]);
		team.erase(team.begin()+randIndex);
		ifstream holderPathFile;
		map<string,string> holderMap;

		string i_star;
		clock_t read_start = clock();
		bool fail=false;
		double cost_MST=0;

		while(team.size()!=0)
		{
			/*
			*
			* Find the nearest vertex to X_PREM among team
			*
			*/
			double R_i=999999;
			for(vector<string>::iterator it_name = team.begin();it_name!=team.end();++it_name)
			{
				vector<string> row;
				string line;
				map<string,double> shortestDist;
			
				string p = PATH+*it_name+SUFFIX;
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
					if(row[1]=="DISCONNECTED")
						shortestDist.insert(make_pair(row[0],999));
					else
					{
						shortestDist.insert(make_pair(row[0],atof(row[1].c_str())));
					}
				}
				holderPathFile.close();
				read_start = clock()-read_start;
				file_time += read_start;
				
				double R_ia = 99999999;
				for(vector<string>::iterator it_X_PREM =X_PREM.begin();it_X_PREM!=X_PREM.end();++it_X_PREM)
				{
					//for loop to find d(u,X'), line 3 in algorithm
					if( shortestDist[it_X_PREM->c_str()]<R_ia)
					{
						R_ia = shortestDist[it_X_PREM->c_str()];
					}
				}
				if(R_ia<R_i)
				{
					R_i = R_ia;
					i_star = *it_name;
				}
			}
			cout<<"i_star:"<<i_star<<endl;
			/*
				all the following lines are working for line 9 in algorithm, now the skill holder is i_star
			*/
			
	
			string p = PATH+i_star+SUFFIX;
			holderPathFile.open(p.c_str(),ios::in|ios::binary);
			if(holderPathFile.fail())
			{
				p = PATH + ConvertToString(index[i_star])+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
			}
			if(holderPathFile.fail())
			{
				cout<<i_star<<" "<<"holderPathFile FAIL"<<endl;
				return 0;
			}	
		
			string line;
			vector<string> row;
			map<string,vector<string> > solution;
			map<string,double> shortestDist_i_star;
			read_start = clock();
			while(getline(holderPathFile,line),holderPathFile.good())
			{
				csvRead::csvline_populate(row,line,',');
				if(row[1]=="DISCONNECTED")
				{
					shortestDist_i_star.insert(make_pair(row[0],99999));
					//solution[row[0].c_str()]+=row[0];
				}	
				else
				{
					shortestDist_i_star.insert(make_pair(row[0],atof(row[1].c_str())));
					solution[row[0].c_str()].push_back(row[2]);
					for(int i = 3;i<(row.size()-1)&&row[i]!="";i++)
						solution[row[0].c_str()].push_back(row[i]);
				}
			}	
			holderPathFile.close();
			file_time += clock()-read_start;
			double R_ia = 99999999;
			string destination;
			for(vector<string>::iterator it_owner = X_PREM.begin();it_owner!=X_PREM.end();++it_owner)
			{
				if( shortestDist_i_star[it_owner->c_str()]<R_ia)
				{
					R_ia = shortestDist_i_star[it_owner->c_str()];
					destination = *it_owner;
				}
			}
			X_PREM.push_back(i_star);
			if(!solution[destination].empty())
			{
				for(int dex=0;dex<solution[destination].size();++dex)
					X_PREM.push_back(solution[destination].at(dex));
			}
			else
			{
				fail = true;
				break;
			}
			for(vector<string>::iterator it = team.begin();it!=team.end();++it)
			{
				if(*it==i_star)
				{
					team.erase(it);
					break;
				}
			}
			cost_MST+=shortestDist_i_star[destination];
		}
		if(fail)
		{
			continue;
		}




		total_time = clock()-begin_time-file_time;
		if(X_PREM.size()==0)
			continue;
		string p_a_t_h="";
		string s_k_i_l_l="";
		for(vector<string>::iterator i_s = X_PREM.begin();i_s!=X_PREM.end();++i_s)
		{
			p_a_t_h+=*i_s+",";
		}

		for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<<"SKILL:"<<it_skill->first<<endl;
				s_k_i_l_l+=it_skill->first+",";
			}
		cout<<"Computational time: "<<total_time<<endl;
		cout<<cost_MST<<endl;
		output<<user_v<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost_MST<<endl;
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

void getCapacity(map<string,double>& capacity)
{
	vector<string> row;
	ifstream in("E:\\ProgramData\\Java\\DBLP\\20130806\\capacity.csv",ios::binary);
	if (in.fail())  { cout << "File not found" <<endl; return ; }
	double capa=0;
	string line;
    while(getline(in, line)  && in.good() )
    {
        csvRead::csvline_populate(row, line, ',');

		capacity.insert(make_pair(row[0],atof(row[1].c_str())));
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