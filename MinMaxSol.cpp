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
	const unsigned short REQUIRED_SKILL_NUM = 14;
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	map<string,double> capacity;
	getCapacity(capacity);
	vector<Radius*> E;
	vector<skill*> SKILLSET;
	map<string,author_skill*> AUTHORSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	output.open(PATH+"resultMMS\\"+"result_"+ConvertToString( REQUIRED_SKILL_NUM)+SUFFIX,ios::out|ios::binary|ios::trunc);
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
			E.push_back(new Radius(user_v,0));
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
					E.push_back(new Radius(row[0],999));
					discon++;
				}
				else
					E.push_back(new Radius(row[0],atof(row[1].c_str())));
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

		sort(E.begin(),E.end(),compare);//the radius have been sorted increasingly for the BS follows;
		cout<<E.size()<<endl;;
		long long MAXITEMS = 0L;
		vector<int> solution;
		bool fail = false;
		vector<Radius*>::size_type endPos=0;
		for(endPos=0;endPos<E.size();++endPos)
		{
			Dinic din(REQUIRED_SKILL_NUM+endPos+3);
			map<string,int> MAP_INDEX;
			unsigned int dex = 0;
			MAP_INDEX.insert(make_pair("_S",dex));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				MAP_INDEX.insert(make_pair(it_task->first,++dex));
			for(int pos = 0; pos<=endPos;pos++)
				MAP_INDEX.insert(make_pair(E[pos]->name,++dex));
			MAP_INDEX.insert(make_pair("_T",++dex));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
				din.AddEdge(MAP_INDEX["_S"],MAP_INDEX[it_task->first],1);
			for(int pos = 0; pos<=endPos;pos++)
				din.AddEdge(MAP_INDEX[E[pos]->name],MAP_INDEX["_T"],ceil(capacity[E[pos]->name]));
			for(map<string,skill*>::iterator it_task = TASK.begin();it_task!=TASK.end();++it_task)
			{
				for(vector<string>::iterator it_holder = it_task->second->holders.begin();it_holder!=it_task->second->holders.end();it_holder++)
				{
					if(MAP_INDEX.find(*it_holder)!=MAP_INDEX.end())
						din.AddEdge(MAP_INDEX[it_task->first],MAP_INDEX[*it_holder],1);
				}
			}
			MAXITEMS = din.GetMaxFlow(MAP_INDEX["_S"],MAP_INDEX["_T"]);
			if(MAXITEMS == REQUIRED_SKILL_NUM)
			{
				for(int b=1;b<=REQUIRED_SKILL_NUM;b++)
				{
					for(vector<Edge>::const_iterator it = din.G[b].begin();it!=din.G[b].end();++it)
					{
						if(it->flow==1)
						{
							solution.push_back(it->to);
							break;
						}
					}
				}
				break;
			}
			if(E[endPos]->dist==999)
			{
				fail = true;
				break;
			}
		}
		total_time = clock()-begin_time-file_time;
		if(fail||solution.size()==0)
			continue;
		string p_a_t_h="";
		string s_k_i_l_l="";
		double sumOfDis = 0;
		for(vector<int>::iterator i_s = solution.begin();i_s!=solution.end();++i_s)
		{
			p_a_t_h+=E[*i_s-REQUIRED_SKILL_NUM]->name+",";
			sumOfDis+=E[*i_s-REQUIRED_SKILL_NUM]->dist;
		}

		for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
		{
			cout<<"SKILL:"<<it_skill->first<<endl;
			s_k_i_l_l+=it_skill->first+",";	
		}
		cout<<"Computational time: "<<total_time<<endl;
		cout<<E[endPos]->dist<<endl;
		output<<user_v<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<E[endPos]->dist<<","<<sumOfDis<<endl;
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