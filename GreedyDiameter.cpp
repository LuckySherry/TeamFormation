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

int exchange(edges *a,edges *b)
{
	edges t;
	t = *a;
	*a = *b;
	*b = t;
	return 0;
}

int partition(edges*edge,int p,int r)
{
  int i = p-1,j = p;
  
  for(;j<r;j++)
  {
	  if(edge[j].getWeight() <= edge[r].getWeight())
   {
    i++;
    exchange(edge+i,edge+j);
   }
  }
  exchange(&edge[i+1],&edge[r]);
  return i+1;
}
int quick_sort(edges edge[],int p,int r)
{
 if(p < r)
 {
  int q = partition(edge,p,r);
  quick_sort(edge,p,q-1);
  quick_sort(edge,q+1,r);
 }
 return 0;
}

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n);

int main()
{
	const float FREQ_THRESHOLD_HIGH = 0.05;
	const float FREQ_THRESHOLD_LOW = 0.001;
	const unsigned int NUM_OF_TASK = 100;
	//{2,4,6,8,10,12,14,16,18,20};
	const unsigned short REQUIRED_SKILL_NUM = 8;
	int num[] = {2,4,6,8,10,12,14,16,18,20};
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;
	map<string,author_skill*> AUTHORSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	for(int pos = 0;pos<10;pos++)
	{
	output.open(PATH+"resultGD\\"+"result_"+ConvertToString( num[pos])+SUFFIX,ios::out|ios::binary|ios::trunc);
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
		while(TASK.size()!=num[pos])
		{
			int required_skill_index = rand()%SKILLSET.size();
			cout<<required_skill_index<<endl;
			SKILLSET[required_skill_index]->isCover = false;
			TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
		}
		map<string,author_skill*> AUTHORSET;
		vector<string> GREEDYCOVER;
		vector<string> GREEDYDIA;
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

		}
		cout<<"AUTHOR SIZE:"<<AUTHORSET.size()<<endl;
		map<string,map<string,double> > UNIVERSE_DIST;
		map<string,map<string,vector<string> > > UNIVERSE_PATH;
		for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
		{
			vector<string> row;
			ifstream holderPathFile;
			string line;
			map<string,double> shortestDist;
			string p = PATH+it_au_s->first+SUFFIX;
			clock_t read_start = clock();
			holderPathFile.open(p.c_str(),ios::in|ios::binary);
			if(holderPathFile.fail())
			{
				p = PATH + ConvertToString(index[it_au_s->first])+SUFFIX;
				holderPathFile.open(p.c_str(),ios::in|ios::binary);
			}	
			if(holderPathFile.fail())
			{
				cout<<it_au_s->first<<"OPEN FAIL"<<endl;
				return 0;
			}	
			while(getline(holderPathFile,line),holderPathFile.good())
			{
				csvRead::csvline_populate(row,line,',');
				if(row[1]=="DISCONNECTED")
				{
					UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],999));
					vector<string> temp;
					UNIVERSE_PATH[it_au_s->first].insert(make_pair(row[0],temp));
				}
				else
				{
					vector<string> solution;
					UNIVERSE_DIST[it_au_s->first].insert(make_pair(row[0],atof(row[1].c_str())));
					for(int i = 2;i<row.size();++i)
					{
						solution.push_back(row[i]);
					}
					UNIVERSE_PATH[it_au_s->first].insert(make_pair(row[0],solution));
				}
			}
			holderPathFile.close();
			read_start = clock()-read_start;
			file_time += read_start;
		}

		bool RET = false;
		/*
		*
		* The following while loop find the greedy cover of the task;
		*
		*/



		int coveredCount =0;
		int coverGrow = 0;
		double cost = 0;
		while(!RET&&AUTHORSET.size()>0)
		{
			if(GREEDYDIA.size()==0)
			{
				int currentMaxCover = 0;
				string maxCoverX;
				for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
				{
					int cover = 0;
					for(vector<skill*>::iterator it_skill=it_au_s->second->skill_set.begin();it_skill!=it_au_s->second->skill_set.end();++it_skill)
					{
						if (!(*it_skill)->isCover)
						{
							cover++;
						}
					}
					if(cover>currentMaxCover)
					{
						currentMaxCover = cover;
						maxCoverX = it_au_s->second->getName();
					}
				}
				for(vector<skill*>::iterator it_skill=AUTHORSET[maxCoverX]->skill_set.begin();it_skill!=AUTHORSET[maxCoverX]->skill_set.end();++it_skill)
				{
					(*it_skill)->isCover = true;
				}
				GREEDYDIA.push_back(maxCoverX);
				coveredCount+=currentMaxCover;
				AUTHORSET.erase(maxCoverX);
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
			else
			{
				double benefit=0;
				vector<string> SKILL;
				string auth;
				string i_star;
				for(map<string,author_skill*>::iterator it_au_s = AUTHORSET.begin();it_au_s!=AUTHORSET.end();++it_au_s)
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
					double cost_MST=0;
					double diameter = 0;
					set<edges> graph;
					
					string des;
					for(vector<string>::iterator it_NE = GREEDYDIA.begin();it_NE!=GREEDYDIA.end();it_NE++)
					{
						if(UNIVERSE_DIST[it_au_s->first][*it_NE]==999)
						{
							fail = true;
							break;
						}
						edges e(it_au_s->first,*it_NE);
						e.setWeight(UNIVERSE_DIST[it_au_s->first][*it_NE]);
						graph.insert(e);
						if(UNIVERSE_DIST[it_au_s->first][*it_NE]>diameter)
						{
							diameter = UNIVERSE_DIST[it_au_s->first][*it_NE];
							des = *it_NE;
						}
					}
					if(fail)
					{
						it_au_s = AUTHORSET.erase(it_au_s);
						continue;
					}
					if(UNIVERSE_PATH[it_au_s->first][des].size()!=0)
					{
						for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first][des].begin();it_sol!=UNIVERSE_PATH[it_au_s->first][des].end();it_sol++)
						{
							if(AUTHORSET.find(*it_sol)!=AUTHORSET.end())
							{
								for(vector<skill*>::iterator it_skill=AUTHORSET[*it_sol]->skill_set.begin();it_skill!=AUTHORSET[*it_sol]->skill_set.end();++it_skill)
								{
									skillToBeCovered.insert((*it_skill)->getName());
								}
								edges e(it_au_s->first,*it_sol);
								e.setWeight(UNIVERSE_DIST[it_au_s->first][*it_sol]);
								graph.insert(e);
							}
						}
					}/*
					*FOR MST
					*/
					/*
					* use for the MST calculating
					*/
					/*edges *edge = new edges[graph.size()];
					map<string,double> MAKE_SET;
					set<edges> MST_edge;
					if(!fail&&GREEDYDIA.size()>1)
					{
						int index=0;
						for(set<edges>::iterator it = graph.begin();it!=graph.end();it++)
						{
							edge[index++] = *it;
						}
						index = 0;
						if(UNIVERSE_PATH[it_au_s->first][des].size()!=0)
						{
							for(vector<string>::iterator it_sol = UNIVERSE_PATH[it_au_s->first][des].begin();it_sol!=UNIVERSE_PATH[it_au_s->first][des].end();it_sol++)
							{
								if(AUTHORSET.find(*it_sol)!=AUTHORSET.end())
								{
									MAKE_SET[*it_sol]=index++;
								}
							}
						}
						MST_Kruskal(edge,MAKE_SET,MST_edge,graph.size());
					}
					for(set<edges>::iterator it = MST_edge.begin();it!=MST_edge.end();++it)
					{
						cost_MST+= it->getWeight();
					}*/

					if(skillToBeCovered.size()/diameter>benefit)
					{
						
						benefit = skillToBeCovered.size()/diameter;
						auth = it_au_s->first;
						i_star = des;
						cost = diameter;
						SKILL.clear();
						for(set<string>::iterator it = skillToBeCovered.begin();it!=skillToBeCovered.end();++it)
						{
							SKILL.push_back(*it);
						}
					}
				}
				GREEDYDIA.push_back(auth);
				for(vector<string>::iterator it_sol = UNIVERSE_PATH[auth][i_star].begin();it_sol!=UNIVERSE_PATH[auth][i_star].end();it_sol++)
				{
					GREEDYDIA.push_back(*it_sol);
					map<string,author_skill*>::iterator it=AUTHORSET.find(*it_sol);
					if(it!=AUTHORSET.end())
						AUTHORSET.erase(it);
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
				cout<<AUTHORSET.size()<<endl;
			}

		}
		
		

		
		int line = 0;
		if(RET)
		{
			total_time = clock()-begin_time-file_time;
			string p_a_t_h="";
			string s_k_i_l_l="";
			for(vector<string>::iterator i_X_P = GREEDYDIA.begin();i_X_P!=GREEDYDIA.end();++i_X_P)
			{
				p_a_t_h+=*i_X_P+",";
			}				
			for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
				{
					cout<<"SKILL:"<<it_skill->first<<endl;
					s_k_i_l_l+=it_skill->first+",";
				}
			cout<<"Computational time: "<<total_time<<endl;
			cout <<cost<<endl;
			output<<line++<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost<<endl;
		}
	}
	
	output.close();
}}

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

void MST_Kruskal(edges edge[],map<string,double>& nodeSet,set<edges>& eSet,int n)
{
	quick_sort(edge,0,n-1);
	for(int i =0;i<n;i++)
	{
		if(nodeSet[edge[i].getAuthor(false)]!=nodeSet[edge[i].getAuthor(true)])
		{
			eSet.insert(edge[i]);
			double s1 = nodeSet[edge[i].getAuthor(true)];
			double s2 = nodeSet[edge[i].getAuthor(false)];
			for(map<string,double>::iterator it = nodeSet.begin();it!=nodeSet.end();it++)
			{
				if(it->second==s1)
					it->second =s2;
			}

		}
	} 
}