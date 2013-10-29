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
	double getFreq(){return frequency;}
	string getName(){return name;}
	void setName(string n){name = n;}
	string i_prem;	//d(i*,s(a)) = d(i*,i')
	//vector<string>::size_type index;
	skill(string name, double freq):name(name),frequency(freq){holders.clear();};
private:
	string name;
	
	double frequency;
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

int main000()
{
	const float FREQ_THRESHOLD_HIGH = 0.05;
	const float FREQ_THRESHOLD_LOW = 0.001;
	const unsigned int NUM_OF_TASK = 100;
	//{2,4,6,8,10,12,14,16,18,20};
	const unsigned short REQUIRED_SKILL_NUM = 20;
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;

	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);

	ofstream output; //FOR DEBUGGING
	output.open(PATH+"result_RF\\"+"result_"+ConvertToString( REQUIRED_SKILL_NUM)+SUFFIX,ios::out|ios::binary|ios::trunc);
	for(unsigned int times=0;times!=NUM_OF_TASK;++times)
	{
		clock_t begin_time = clock();
		clock_t total_time = 0L;
		clock_t file_time = 0L;
		cout<<times+1<<endl;
		map<string,skill*> TASK;
		srand(time(0)+times);
		double currentMinFreq = 1;
		skill* rarest_skill;
		set<string> TASK_HOLDER_SET;
		while(TASK.size()!=REQUIRED_SKILL_NUM)
		{
			int required_skill_index = rand()%SKILLSET.size();
			cout<<required_skill_index<<endl;
			if(SKILLSET[required_skill_index]->getFreq()<currentMinFreq)
			{
				currentMinFreq = SKILLSET[required_skill_index]->getFreq();
				rarest_skill = SKILLSET[required_skill_index];
			}
			TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
		}
		vector<skill*> tttt;

		ifstream holderPathFile;
		map<string,string> holderMap;
		double minDist=99999999;
		string i_star;
		clock_t read_start = clock();
		for(vector<string>::iterator it_name = rarest_skill->holders.begin();it_name!=rarest_skill->holders.end();it_name++)
		{
			//for loop to find the min(R_i), line 8 in the algorithm;
			string p = PATH+*it_name+SUFFIX;
			//cout<<p;
			vector<string> row;
			string line;
			map<string,double> shortestDist;
			
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
					shortestDist.insert(make_pair(row[0],99999));
				else
				{
					shortestDist.insert(make_pair(row[0],atof(row[1].c_str())));
				}
			}
			holderPathFile.close();
			read_start = clock()-read_start;
			file_time += read_start;
			double R_i=0;
			for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				//for loop to find max_a(R_ia), line 7 in algorithm
				double R_ia = 99999999;
				if(it_skill->second==rarest_skill)
					continue;
				for(vector<string>::iterator it_owner = it_skill->second->holders.begin();it_owner!=it_skill->second->holders.end();++it_owner)
				{
					//for loop to find d(i,s(a), line 6 in algorithm
					TASK_HOLDER_SET.insert(*it_owner);
					if(*it_owner==*it_name)
					{
						R_ia = 0;
						break;
					}

					if( shortestDist[it_owner->c_str()]<R_ia)
					{
						R_ia = shortestDist[it_owner->c_str()];
					}
				}
				if(R_ia>R_i)
				{
					R_i = R_ia;
				}
			}
			if(R_i<minDist)
			{
				minDist = R_i;
				i_star = *it_name;
			}
		}
		if(i_star.empty())
		{
			cout<< "i_star FAIL"<<endl;
			for(map<string,skill*>::iterator it_skill = TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<< it_skill->first<<endl;
			}
			return 0;
		}
		cout<<"i_star:"<<i_star<<endl;
		/*
			all the following lines are working for line 9 in algorithm, now the skill holder is i_star
		*/
		read_start = clock();

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
		map<string,string> solution;
		map<string,double> shortestDist_i_star;
		solution[i_star] = i_star;
		while(getline(holderPathFile,line),holderPathFile.good())
		{
			csvRead::csvline_populate(row,line,',');
			if(row[1]=="DISCONNECTED")
			{
				shortestDist_i_star.insert(make_pair(row[0],99999));
				solution[row[0].c_str()]+=row[0];
			}
			else
			{
				shortestDist_i_star.insert(make_pair(row[0],atof(row[1].c_str())));
				solution[row[0].c_str()]+=row[2];
				for(int i = 3;i<row.size()&&row[i]!="";i++)
					solution[row[0].c_str()]+=","+row[i];
			}
		}
		holderPathFile.close();
		file_time += clock()-read_start;

		string s_k_i_l_l="";
		string p_a_t_h="";
		string coordinator="";

		total_time = clock()-begin_time-file_time;

		//output.open(PATH+"testing.txt",ios::out|ios::binary|ios::trunc);
		for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
			{
				cout<<"SKILL:"<<it_skill->first<<endl;
				s_k_i_l_l+=it_skill->first+",";
				double R_ia = 99999999;
				if(it_skill->second==rarest_skill)
					continue;
				for(vector<string>::iterator it_owner = it_skill->second->holders.begin();it_owner!=it_skill->second->holders.end();++it_owner)
				{
					if( shortestDist_i_star[it_owner->c_str()]<R_ia)
					{
						R_ia = shortestDist_i_star[it_owner->c_str()];
						it_skill->second->i_prem = *it_owner;
					}
				}
				cout<<solution[it_skill->second->i_prem]<<endl;
				if(TASK_HOLDER_SET.find(it_skill->second->i_prem)!=TASK_HOLDER_SET.end())
				{
					if(solution[it_skill->second->i_prem]=="")
					{
						cout<<it_skill->first<<":"<<it_skill->second->i_prem<<endl;
						system("pause");
					}					
					p_a_t_h+=solution[it_skill->second->i_prem]+",";
					
				}
				else
				{
					coordinator+="COORDINATOR "+solution[it_skill->second->i_prem]+",";
				}
			}
		cout<<"Computational time: "<<total_time<<endl;
		cout <<minDist<<endl;
		output<<i_star<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<coordinator<<","<<minDist<<endl;
		//cout<<(500000000%1052)<<endl;
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