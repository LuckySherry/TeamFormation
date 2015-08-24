//
//  main.cpp
//  BiObjectiveReplace
//
//  Created by WangXinyu on 15/3/17.
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

//#define NUM_OF_NODES 1712433
#define NUM_OF_NODES 9948
#define LAMBDA 0.5

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
boost::property<boost::vertex_distance_t, double>, boost::property<boost::edge_weight_t, double> > graph_t;
typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;

struct Skill_Holder_Cost
{
    unsigned long name;
    double cost;
};

class greaterCost
{
public:
    bool operator() (const Skill_Holder_Cost&, const Skill_Holder_Cost&) const;
};

bool greaterCost::operator()(const Skill_Holder_Cost& c1, const Skill_Holder_Cost& c2) const
{
    return c1.cost<c2.cost;
}


class skill
{
public:
    skill(){};
    std::vector<Skill_Holder_Cost> holders;
    void push_back(unsigned long s, double cost){Skill_Holder_Cost shc;shc.name=s;shc.cost=cost; holders.push_back(shc);}
    void setFreq(unsigned d){frequency = d;}
    unsigned getFreq(){return frequency;}
    std::string getName(){return name;}
    void setName(std::string n){name = n;}
    void sort(){std::sort(holders.begin(), holders.end(), greaterCost());}
    unsigned long i_prem;	//d(i*,s(a)) = d(i*,i')
    //vector<string>::size_type index;
    unsigned indexInGraph;
    bool isCover;
    unsigned holderIndex;
    skill(std::string name, unsigned freq):name(name),frequency(freq){holders.clear();isCover=false;holderIndex=0;};
private:
    std::string name;
    
    unsigned frequency;
};


int main(int argc, const char * argv[]) {
    //comment after testing:
    std::ifstream in(getenv("INPUT"));
    std::cin.rdbuf(in.rdbuf());
    
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
                //if (personCost[expert.numberLong()]<=1) {
                  //  continue;
                //}
//                tmpSkill.push_back(expert.numberLong(),personCost[expert.numberLong()]);
                tmpSkill.push_back(expert.numberLong(),personCost[expert.numberLong()]);
            }
            tmpSkill.sort();
            if (tmpSkill.holders.size()==0) {
                for(auto expert:experts)
//                    tmpSkill.push_back(expert.numberLong(),personCost[expert.numberLong()]);
                    tmpSkill.push_back(expert.numberLong(),personCost[expert.numberLong()]);
            }
            tmpSkill.i_prem = tmpSkill.holders[0].name;
            skills.push_back(tmpSkill);
            
        }
        std::string rarestSkill = columns[numOfSkills+1];
        
        /*
         Begin Replace Algorithm
         */
        double ComCost = 99999;
        
        std::time_t start = clock();
        std::time_t duration = 0;
        
        bool reachEnd = false;
        while (!reachEnd) {
            for (auto &reqSkill:skills)
            {
                d.clear();
                p.clear();
                candidate_d.clear();
                candidate_p.clear();
                
                if (reqSkill.holderIndex>=reqSkill.holders.size()-1)
                    continue;
                unsigned long candidate = reqSkill.i_prem;
                unsigned long replace = reqSkill.holders[++reqSkill.holderIndex].name;
                
                double distFromCandidate = 0, distFromReplace = 0;
                duration += clock()-start;
                vertex_descriptor s = vertex(candidate,AMinerGraph);
                dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                     boost::predecessor_map(boost::make_iterator_property_map(candidate_p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                     distance_map(boost::make_iterator_property_map(candidate_d.begin(), get(boost::vertex_index, AMinerGraph))));
                
                vertex_descriptor r = vertex(replace,AMinerGraph);
                dijkstra_shortest_paths_no_color_map(AMinerGraph, r,
                                                     boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                     distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
                
                start = clock();
                
                for (auto &iterateSkill:skills)
                {
                    if (iterateSkill.getName()==reqSkill.getName())
                        continue;
                    unsigned long neighbour = iterateSkill.i_prem;
                    double tempDis1 = (1-LAMBDA)*(personCost[candidate]+personCost[neighbour])+LAMBDA*(candidate_d[neighbour]);
                    distFromCandidate += tempDis1;
                    
                    double tempDis2 = (1-LAMBDA)*(personCost[replace]+personCost[neighbour])+2*LAMBDA*(d[neighbour]);
                    distFromReplace += tempDis2;
                }
                if (distFromReplace < distFromCandidate) {
                    reqSkill.i_prem = replace;
                }
            }
            reachEnd = true;
            for (auto &reqSkill:skills)
            {
                if (reqSkill.holderIndex < reqSkill.holders.size()-1) {
                    reachEnd = false;
                    break;
                }
            }
        }
        std::time_t realEnd = clock();
        // Algorithm finished
        bool fail = false;
        for (auto &reqSkill : skills)
        {
            candidate_d.clear();
            candidate_p.clear();
            double sumCC = 0;
            unsigned long candidate = reqSkill.i_prem;
            if (candidate > NUM_OF_NODES) {
                break;
            }
            
            vertex_descriptor s = vertex(candidate,AMinerGraph);
            dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                 boost::predecessor_map(boost::make_iterator_property_map(candidate_p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                 distance_map(boost::make_iterator_property_map(candidate_d.begin(), get(boost::vertex_index, AMinerGraph))));
            
            for (auto &iterateSkill : skills)
            {
                if (iterateSkill.i_prem > NUM_OF_NODES||candidate_d[iterateSkill.i_prem]>9999)
                {
                    fail = true;
                    break;
                }
                if (iterateSkill.i_prem==reqSkill.i_prem)
                    continue;
                double tempDis = (1-LAMBDA)*(personCost[reqSkill.i_prem]+personCost[iterateSkill.i_prem])+2*LAMBDA*(candidate_d[iterateSkill.i_prem]);
                sumCC += tempDis;
            }
            if (fail) {
                break;
            }
            if (sumCC < ComCost) {
                ComCost = sumCC;
            }
        }
        
        
        
        std::cout << taskIndex << '\t' << ComCost << '\t' << duration*1000.0/CLOCKS_PER_SEC <<'\t'<< (realEnd-realStart)*1000.0/CLOCKS_PER_SEC;
        if (ComCost > 9999) {
            std::cout << std::endl;
            continue;
        }
        std::set<unsigned long> experts;
        unsigned long candidateIndex = skills[skills.size()-1].i_prem;
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
