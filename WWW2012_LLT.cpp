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
double getWorkLoad(int& max_workload,int* workload);

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
	const double BS_UpperBound[] = {3.934449955,7.833595526,11.52751515,14.52217418,17.73947517,19.76494182,21.69230768,23.69832,25.92347945,28.3909197};
	
	const string PATH = "E:\\ProgramData\\Java\\DBLP\\20130726\\";
	const string SKILL_PATH = "E:\\ProgramData\\Java\\DBLP\\20130730\\_";
	const string SUFFIX = ".csv";
	map<string,vector<skill*>::size_type> index;
	getIndex(index); //get the name-index map;
	vector<skill*> SKILLSET;
	getskills(FREQ_THRESHOLD_HIGH,FREQ_THRESHOLD_LOW,SKILLSET);
	ofstream output; //FOR DEBUGGING
	int max_workload = 1;
	const double lambda[] = {0.0001,0.0004,0.0016,0.0064,0.0256,0.1024,0.4096,1.6384,6.4};
	for(int lambda_index = 0;lambda_index<9;lambda_index++)
	{
		int workload [7159]={0};
		output.open(PATH+"resultWWW12_2\\"+"result_"+ConvertToString(lambda_index)+SUFFIX,ios::out|ios::binary|ios::trunc);
		/*
		* read skill index
		*/
		string pline;
		vector<string> prow;
		ifstream input (PATH+"resultWWW12_1\\"+"skill_2_"+ConvertToString(lambda_index)+SUFFIX,ios::binary);
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
			short pos = rand()%10;
			for(vector<int>::iterator i_Task = task_list[times].begin();i_Task!=task_list[times].end();i_Task++)
			{
				int required_skill_index = *i_Task;
				SKILLSET[required_skill_index]->isCover = false;
				TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
			}
			/*
			while(TASK.size()!=num[pos])
			{
				int required_skill_index = rand()%SKILLSET.size();
				//cout<<required_skill_index<<endl;
				SKILLSET[required_skill_index]->isCover = false;
				TASK.insert(make_pair(SKILLSET[required_skill_index]->getName(),SKILLSET[required_skill_index]));
			}*/
			map<string,author_skill*> AUTHORSET;
			map<string,map<string,double> > HENCECOVER;
			map<string,map<string,double> >SKILL_WL;
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
				map<string,double> path_workload;
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
						double work=0;
						for(int i = 2; i<row.size()-1;i++)
						{
							int* data = new int [3];
							data[0] = workload[index[row[i]]];
							data[1] = TASK.size();
							data[2] = index.size();
							work+=getWorkLoad(max_workload, data);
							delete data;
						}
						path_workload.insert(make_pair(row[0],lambda[lambda_index]*work));
						shortestDist.insert(make_pair(row[0],atof(row[1].c_str())));
					}
				}
				skillPathFile.close();
				file_time+=clock()-read_start;
				SKILL_WL.insert(make_pair("_"+it_task->first,path_workload));
				HENCECOVER.insert(make_pair("_"+it_task->first,shortestDist));
				GREEDYCOVER.push_back("_"+it_task->first);
			}
			cout<<HENCECOVER.size()<<endl;

			double lambda_A = 0, lambda_1 = lambda[lambda_index], lambda_2 = UPPERBOUND;

			vector<string> X_PREM;
			int randIndex = rand()%HENCECOVER.size();
		
			X_PREM.push_back(GREEDYCOVER[randIndex]);
			GREEDYCOVER.erase(GREEDYCOVER.begin()+randIndex);
			ifstream holderPathFile;
			map<string,string> holderMap;

			string i_star;
		
			bool fail=false;
			double cost_MST=0;
			double cost_Work=0;
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
				double R_work=0;

				for(vector<string>::iterator it_name = GREEDYCOVER.begin();it_name!=GREEDYCOVER.end();++it_name)
				{
				
					double R_ia = 99999999;
					double work = 0;
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
										work = 0;
										if(find(X_PREM.begin(),X_PREM.end(),Hdex->first)==X_PREM.end())
										{
											int*data = new int [3];
											data[0] = workload[index[Hdex->first]];
											data[1] = TASK.size();
											data[2] = index.size();
											work = getWorkLoad(max_workload,data);
											delete data;
										}
										if(9999.0*2+work<R_ia)
										{
											R_ia = 9999.0*2+work;
											if(R_ia<R_i)
											{
												R_i = R_ia;
												mediator = Hdex->first;
												destination = *it_X_PREM;
												i_star = *it_name;
												R_work = work;
												break;
											}
										}							
									}
									else
									{
										work = SKILL_WL[*it_name][Hdex->first]+SKILL_WL[*it_X_PREM][Hdex->first];
										if(find(X_PREM.begin(),X_PREM.end(),Hdex->first)==X_PREM.end())
										{
											int*data = new int [3];
											data[0] = workload[index[Hdex->first]];
											data[1] = TASK.size();
											data[2] = index.size();
											work += getWorkLoad(max_workload,data);
											delete data;
										}
										if(HENCECOVER[*it_name][Hdex->first]+HENCECOVER[*it_X_PREM][Hdex->first]+work<R_ia)
										{
											R_ia = HENCECOVER[*it_name][Hdex->first]+HENCECOVER[*it_X_PREM][Hdex->first]+work;
											if(R_ia<R_i)
											{
												R_i = R_ia;
												mediator = Hdex->first;
												destination = *it_X_PREM;
												i_star = *it_name;
												R_work = work;
											}										
										}
									}
								}
							}
						}
						else //it_X_PREM is an author
						{
							if(HENCECOVER[*it_name][*it_X_PREM]<30000.0)
							{
								if(HENCECOVER[*it_name][*it_X_PREM]==9999.0)
								{
									work = SKILL_WL[*it_name][*it_X_PREM];
									if(R_ia > 9999.0+work)
									{
										R_ia = 9999.0+work;
										if(R_ia<R_i)
										{
											R_i = R_ia;
											i_star = *it_name;
											mediator = *it_X_PREM;
											R_work = work;
										}
										break;
									}
								}
								else
								{
									work = SKILL_WL[*it_name][*it_X_PREM];
								
									if(HENCECOVER[*it_name][*it_X_PREM]+work <R_ia)
									{
										R_ia = HENCECOVER[*it_name][*it_X_PREM]+work;
									}
								}
							}
						}
						if(R_ia<R_i)
						{
							R_i = R_ia;
							i_star = *it_name;
							R_work = work;
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
				double f_WL = 0;
			

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
					if(find(X_PREM.begin(),X_PREM.end(),i_star)==X_PREM.end())
						X_PREM.push_back(i_star);
					if(!solution.empty())
					{
						for(set<string>::iterator dex=solution.begin();dex!=solution.end();++dex)
						{
							if(find(X_PREM.begin(),X_PREM.end(),*dex)==X_PREM.end())
							{
								X_PREM.push_back(*dex);
								//++workload[index[*dex]];
							}
						}
					}
					else
					{
						fail = true;
						break;
					}
					dist = R_i-9999.0-R_work;
					f_WL = R_work;

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
							{
								X_PREM.push_back(*dex);
								//++workload[index[*dex]];
							}
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
							{
								X_PREM.push_back(*dex);
								//++workload[index[*dex]];
							}
						}
					}
					else
					{
						fail = true;
						break;
					}
					dist = R_i-9999.0*2-R_work;
					f_WL = R_work;
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
				cost_Work += f_WL;

			}

			

			
			if(fail)
			{
				continue;
			}
			set<string> final_solution;
			for(vector<string>::size_type i = 0;i!=printSolution.size();i++)
			{
				if(printSolution[i].at(0)!='_')
				{
					final_solution.insert(printSolution[i]);
				}
			}
			double max_personal_workload = 0;
			/*for(vector<string>::iterator it_NE = Q_apx.begin();it_NE!=Q_apx.end();it_NE++)
				++workload[index[*it_NE]];
			*/
			for(unsigned int dex =0;dex<index.size();dex++)
			{
				if(workload[dex]>max_personal_workload&&workload[dex]<=100)
					max_personal_workload=workload[dex];
				if(max_personal_workload>100)
					cout<<"FFFFFFFFFFFF"<<endl;
			}
			cout<<"max_personal_workload: "<<max_personal_workload<<endl;
			int* data = new int[3];
			data[0] = max_personal_workload;
			data[1] = TASK.size();
			data[2] = index.size();
			getWorkLoad(max_workload,data);
			delete data;
			total_time = clock()-begin_time-file_time;

			string p_a_t_h="";
			string s_k_i_l_l="";
			for(set<string>::iterator i_X_P = final_solution.begin();i_X_P!=final_solution.end();++i_X_P)
			{
				p_a_t_h+=*i_X_P+",";
				++workload[index[*i_X_P]];
			}

			for(map<string,skill*>::iterator it_skill =TASK.begin();it_skill!=TASK.end();++it_skill)
				{
					cout<<"SKILL:"<<it_skill->first<<endl;
					s_k_i_l_l+=it_skill->first+",";
				
				}
			cout<<"Computational time: "<<total_time<<endl;
			cout <<cost_MST<<endl;
			cout<<cost_Work<<endl;
			if(cost_MST>=0)
				output<<i_star<<","<<total_time<<","<<s_k_i_l_l<<","<<p_a_t_h<<","<<cost_MST<<","<<cost_Work<<endl;
			AUTHORSET.clear();
			HENCECOVER.clear();
			SKILL_WL.clear();
			GREEDYCOVER.clear();
			X_PREM.clear();

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

double getWorkLoad(int& max_workload,int*data)
{
	//data[0] workload
	//data[1] numofskills
	//data[2] numberofperson
	double A = 2*log(data[1]*1.0)*max_workload*log(2*data[2]*1.0);
	while (A<data[0])
	{
		max_workload *=2;
		A = 2*log(data[1]*1.0)*max_workload*log(2*data[2]*1.0);
	}
	double EXPLoad;
	EXPLoad = pow(2*data[2]*1.0,(data[0]/A));
	return EXPLoad;
}