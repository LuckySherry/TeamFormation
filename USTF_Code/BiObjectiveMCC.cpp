//
//  main.cpp
//  BiObjectiveMCC
//
//  Created by WangXinyu on 15/3/16.
//  Copyright (c) 2015年 WangXinyu. All rights reserved.
//

#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include "csv.h"
#include "edgeDB_guru.h"
#include "personCost_guru.h"

#include "mongo/client/dbclient.h"
#include "mongo/client/init.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/property_map/property_map.hpp>

#include "stdlib.h"
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <ctime>

// #define NUM_OF_NODES 1712433
#define NUM_OF_NODES 9948
#define LAMBDA 0.5

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
boost::property<boost::vertex_distance_t, double>, boost::property<boost::edge_weight_t, double> > graph_t;
typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;

class skill
{
public:
    skill(){};
    std::vector<unsigned long> holders;
    void push_back(unsigned long s){holders.push_back(s);}
    void setFreq(unsigned d){frequency = d;}
    unsigned getFreq(){return frequency;}
    std::string getName(){return name;}
    void setName(std::string n){name = n;}
    unsigned long i_prem;	//d(i*,s(a)) = d(i*,i')
    //vector<string>::size_type index;
    unsigned indexInGraph;
    bool isCover;
    skill(std::string name, unsigned freq):name(name),frequency(freq){holders.clear();isCover=false;};
private:
    std::string name;
    
    unsigned frequency;
};

int main(int argc, const char * argv[]) {
    // insert code here...
    //comment after testing:
    std::ifstream in(getenv("INPUT"));
    std::cin.rdbuf(in.rdbuf());
    std::ofstream out(getenv("OUTPUT"));
    std::cout.rdbuf(out.rdbuf());
    
//    const unsigned numOfEdges = 3826930;
    const unsigned numOfEdges = 3753288;
    /*
     Build graph with sources, targets and dists
     */
    graph_t AMinerGraph(1712431);
    for (unsigned edgeIndex = 0; edgeIndex<numOfEdges; ++edgeIndex)
        boost::add_edge(sources[edgeIndex],targets[edgeIndex],1-dists[edgeIndex],AMinerGraph);
    std::vector<vertex_descriptor> p(num_vertices(AMinerGraph));
    std::vector<double> d(num_vertices(AMinerGraph));
    
    std::vector<vertex_descriptor> candidate_p(num_vertices(AMinerGraph));
    std::vector<double> candidate_d(num_vertices(AMinerGraph));
    
    /*
     Create mongodb client
     */
    std::string uri = "mongodb://localhost:27017";
    std::string errmsg;
    mongo::ConnectionString cs = mongo::ConnectionString::parse(uri, errmsg);
    
    if (!cs.isValid()) {
        return EXIT_FAILURE;
    }
    
    boost::scoped_ptr<mongo::DBClientBase> conn(cs.connect(errmsg));
    if ( !conn ) {
        return EXIT_FAILURE;
    }
    
    /*
     Get skills from stdin
     */
    std::string inputLine;
    std::vector<std::string> columns;
    std::vector<skill> skills;
    
    while (getline(std::cin,inputLine)) {
        /*
         clear vectors: p, d, skills, columns
         */
        p.clear();
        d.clear();
        skills.clear();
        columns.clear();
        std::time_t realStart = clock();
        
        csvRead::csvline_populate(columns, inputLine, '\t');
        
        int taskIndex = atoi(columns[0].c_str());
        int numOfSkills = (taskIndex/100+1)*4;
        
        for (int skillIndex = 1; skillIndex<=numOfSkills; ++skillIndex) {
            std::string skillName = columns[skillIndex];
            mongo::BSONObj skillSearchObj = BSON("Skill"<<skillName);
            std::auto_ptr<mongo::DBClientCursor> cursor = conn->query(/*"AMinerDB*/"GuruDB.skillCollection", skillSearchObj,0,0,NULL,mongo::QueryOption_NoCursorTimeout);
            if (!cursor.get()) {
                return EXIT_FAILURE;
            }
            
            mongo::BSONObj skillNode = cursor->next();
            unsigned expertSize = skillNode.getIntField("ExpertSize");
            std::vector<mongo::BSONElement> experts = skillNode["ExpertIndex"].Array();
            
            skill tmpSkill(skillName,expertSize);
            for(auto expert:experts)
            {
                if (personCost[expert.numberLong()]<=1) {
                    continue;
                }
                tmpSkill.push_back(expert.numberLong());
            }
            if (tmpSkill.holders.size()==0) {
                for(auto expert:experts)
                    tmpSkill.push_back(expert.numberLong());
            }
            skills.push_back(tmpSkill);
        }
        
        std::string rarestSkill = columns[numOfSkills+1];
        int candidateIndex = std::atoi(columns[columns.size()-1].c_str());
        if (personCost[candidateIndex]<=1) {
            std::cout << taskIndex << '\t' << -1 << '\t' << 0 <<'\t'<< (clock()-realStart)*1000.0/CLOCKS_PER_SEC<<std::endl;
            
            continue;
        }
        /*
         Begin MCC Algorithm
         */
        double ComCost = 0;
        std::set<unsigned long> team;
        team.insert(candidateIndex);
        
        
        // 计算基于 candidateIndex 的最短距离
        vertex_descriptor s = vertex(candidateIndex,AMinerGraph);
        dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                             boost::predecessor_map(boost::make_iterator_property_map(candidate_p.begin(), get(boost::vertex_index, AMinerGraph))).
                                             distance_map(boost::make_iterator_property_map(candidate_d.begin(), get(boost::vertex_index, AMinerGraph))));
        
        std::time_t start = clock();
        std::time_t duration = 0;
        bool fail = false;
        
        for (auto &reqSkill:skills)
        {
            if (reqSkill.getName() == rarestSkill) {
                reqSkill.i_prem = candidateIndex;
                continue;
            }
            double leaseMCC = 999999;
            for (auto expert : reqSkill.holders)
            {
                double distFromTeam = 0;
                
                d.clear();
                p.clear();
                duration += clock() - start;
                vertex_descriptor s = vertex(expert,AMinerGraph);
                dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                     boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                     distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
                start = clock();
                
                for (auto teamMate:team)
                    distFromTeam += d[teamMate];
//                double MCC_e_T = (1-LAMBDA)*personCost[expert] + LAMBDA*distFromTeam/team.size();
                double MCC_e_T = (1-LAMBDA)*(1-1.0/personCost[expert]) + LAMBDA*distFromTeam/team.size();
                if (MCC_e_T < leaseMCC) {
                    leaseMCC = MCC_e_T;
                    reqSkill.i_prem = expert;
                }
                
            }
            if (reqSkill.i_prem>=NUM_OF_NODES)
            {
                fail = true;
                break;
            }
            team.insert(reqSkill.i_prem);
            ComCost+=(1-LAMBDA)*((1-1.0/personCost[candidateIndex])+(1-1.0/personCost[reqSkill.i_prem]))+LAMBDA*candidate_d[reqSkill.i_prem];
        }
        duration += clock() - start;
        if (fail) {
            continue;
        }
        // Algorithm finished
        
        std::cout << taskIndex << '\t' << ComCost << '\t' << duration*1000.0/CLOCKS_PER_SEC <<'\t'<< (clock()-realStart)*1000.0/CLOCKS_PER_SEC;
        if (ComCost > 9999) {
            std::cout << std::endl;
            continue;
        }
        std::set<unsigned long> experts;
        experts.insert(candidateIndex);
        for (auto reqSkill : skills)
        {
            unsigned long des = candidate_p[reqSkill.i_prem];
            do
            {
                experts.insert(des);
                des = candidate_p[des];
            }while (des != candidateIndex);
        }
        for (auto reqSkill : skills)
        {
            experts.insert(reqSkill.i_prem);
        }
        for (auto expert :experts)
        {
            std::cout<< '\t' << expert;
        }
        std::cout << std::endl;
        

        
    
    }
        std::cout << "Hello, World!\n";
    return 0;
}
