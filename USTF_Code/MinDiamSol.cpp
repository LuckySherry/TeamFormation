//
//  main.cpp
//  MinDiamSol
//
//  Created by WangXinyu on 15/3/18.
//  Copyright (c) 2015年 WangXinyu. All rights reserved.
//
#if defined(_WIN32)
#include <winsock2.h>
#include <windows.h>
#endif

#include "csv.h"
#include "edgeDB_guru.h"
#include "personCapacity.h"

#include "mongo/client/dbclient.h"
#include "mongo/client/init.h"
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>

#include "stdlib.h"
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <ctime>

#define NUM_OF_NODES 1712433

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
boost::property<boost::vertex_distance_t, double>, boost::property<boost::edge_weight_t, double> > graph_t;
typedef boost::graph_traits < graph_t >::vertex_descriptor vertex_descriptor;


typedef boost::adjacency_list_traits<boost::vecS, boost::vecS, boost::directedS> Traits;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
boost::property<boost::vertex_name_t, std::string>,
boost::property<boost::edge_capacity_t, long,
boost::property<boost::edge_residual_capacity_t, long,
boost::property<boost::edge_reverse_t, Traits::edge_descriptor> > >
> MFGraph;
typedef boost::graph_traits<MFGraph>::vertex_descriptor MF_vertex_descriptor;
typedef boost::graph_traits<MFGraph>::edge_descriptor MF_edge_descriptor;

struct Skill_Holder_Capa
{
    unsigned long name;
    int capacity;
    double distance;
    unsigned skillIndex;
};

class greaterCost
{
public:
    bool operator() (const Skill_Holder_Capa&, const Skill_Holder_Capa&) const;
};

bool greaterCost::operator()(const Skill_Holder_Capa& c1, const Skill_Holder_Capa& c2) const
{
    return c1.distance <c2.distance;
}


class skill
{
public:
    skill(){};
    std::vector<Skill_Holder_Capa> holders;
    void push_back(unsigned long s, int capacity){Skill_Holder_Capa shc;shc.name=s;shc.capacity=capacity; holders.push_back(shc);}
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
    graph_t AMinerGraph(NUM_OF_NODES);
    for (unsigned edgeIndex = 0; edgeIndex<numOfEdges; ++edgeIndex)
        boost::add_edge(sources[edgeIndex],targets[edgeIndex],1-dists[edgeIndex],AMinerGraph);
    std::vector<vertex_descriptor> p(num_vertices(AMinerGraph));
    std::vector<double> d(num_vertices(AMinerGraph));
    
    
    
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

    std::srand(time(0));
    
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
        
        std::vector<Skill_Holder_Capa> skillHolders;
        
        for (int skillIndex = 1; skillIndex<=numOfSkills; ++skillIndex) {
            std::string skillName = columns[skillIndex];
            mongo::BSONObj skillSearchObj = BSON("Skill"<<skillName);
            std::auto_ptr<mongo::DBClientCursor> cursor = conn->query("GuruDB.skillCollection"/*"AMinerDB.skillCollection"*/, skillSearchObj,0,0,NULL,mongo::QueryOption_NoCursorTimeout);
            if (!cursor.get()) {
                return EXIT_FAILURE;
            }
            
            mongo::BSONObj skillNode = cursor->next();
            unsigned expertSize = skillNode.getIntField("ExpertSize");
            std::vector<mongo::BSONElement> experts = skillNode["ExpertIndex"].Array();
            
            skill tmpSkill(skillName,expertSize);
            for(auto expert:experts)
            {
                tmpSkill.push_back(expert.numberLong(),std::ceil(capacity[expert.numberLong()]));
                Skill_Holder_Capa shc;
                shc.name = expert.numberLong();
                shc.capacity = std::ceil(capacity[shc.name]);
                shc.skillIndex = skillIndex;
                skillHolders.push_back(shc);
            }
            skills.push_back(tmpSkill);
        }
        
        std::string rarestSkill = columns[numOfSkills+1];
        unsigned long candidateIndex = 0, count;
        do
        {
            count = 0;
            int randIndex = std::rand()%skillHolders.size();
            candidateIndex = skillHolders[randIndex].name;
            vertex_descriptor s = vertex(candidateIndex,AMinerGraph);
            dijkstra_shortest_paths_no_color_map(AMinerGraph, s,
                                                 boost::predecessor_map(boost::make_iterator_property_map(p.begin(), get(boost::vertex_index, AMinerGraph))).
                                                 distance_map(boost::make_iterator_property_map(d.begin(), get(boost::vertex_index, AMinerGraph))));
            
            for (auto &holder :skillHolders)
            {
                holder.distance = d[holder.name];
                count += (d[holder.name]>9999)? 1:0;
            }
            
        }while (count>skillHolders.size()/2);
        
        
        /*
         Begin MDS Algorithm
         */
        
        std::time_t start = clock();
        
        std::sort(skillHolders.begin(), skillHolders.end(), greaterCost());
        
        unsigned r_begin = 0, r_end = skillHolders.size() - count - 1;
        unsigned minDiaIndex = r_end + 1;
        std::set<unsigned long>solution;
        
        while (true) {
            MFGraph maxFlowGraph;
            boost::property_map<MFGraph, boost::edge_capacity_t>::type
            edgeCapacity = get(boost::edge_capacity, maxFlowGraph);
            boost::property_map<MFGraph, boost::edge_reverse_t>::type
            rev = get(boost::edge_reverse, maxFlowGraph);
            boost::property_map<MFGraph, boost::edge_residual_capacity_t>::type
            residual_capacity = get(boost::edge_residual_capacity, maxFlowGraph);
            
            Traits::vertex_descriptor source, end;
            
            unsigned r_mid = (r_begin+r_end)/2, length = r_mid + skills.size() +2;
            std::vector<MF_vertex_descriptor> verts;
            /*
             build up max flow graph
             */
            for (unsigned long vi = 0; vi < length; ++vi) {
                verts.push_back(boost::add_vertex(maxFlowGraph));
            }
            source = verts[0];
            end = verts[length-1];
            // add edge out from source and into target
            for (unsigned long index = 0; index < verts.size()-1; ++index) {
                MF_edge_descriptor e1, e2;
                bool in1, in2;
                long tail, head;
                long cap;
                if (index < skills.size())
                {
                    tail = 0;
                    head = index + 1;
                    cap  = 1;
                }
                else
                {
                    tail = index + 1;
                    head = length -1;
                    cap  = skillHolders[index-skills.size()].capacity;
                    
                    long skillTail = skillHolders[index-skills.size()].skillIndex;
                    long skillHead = tail;
                    
                    boost::tie(e1, in1) = boost::add_edge(verts[skillTail], verts[skillHead], maxFlowGraph);
                    boost::tie(e2, in2) = boost::add_edge(verts[skillHead], verts[skillTail], maxFlowGraph);
                    
                    if (!in1 || !in2) {
                        std::cerr << "unable to add edge (" << head << "," << tail << ")"
                        << std::endl;
                        return -1;
                    }
                    edgeCapacity[e1] = 1;
                    edgeCapacity[e2] = 0;
                    rev[e1] = e2;
                    rev[e2] = e1;
                    
                    
                }
                boost::tie(e1, in1) = boost::add_edge(verts[tail], verts[head], maxFlowGraph);
                boost::tie(e2, in2) = boost::add_edge(verts[head], verts[tail], maxFlowGraph);
                
                if (!in1 || !in2) {
                    std::cerr << "unable to add edge (" << head << "," << tail << ")"
                    << std::endl;
                    return -1;
                }
                edgeCapacity[e1] = cap;
                edgeCapacity[e2] = 0;
                rev[e1] = e2;
                rev[e2] = e1;
            }
            long flow;
            #if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
                        // Use non-named parameter version
                boost::property_map<MFGraph, boost::vertex_index_t>::type
                indexmap = get(boost::vertex_index, maxFlowGraph);
                flow = push_relabel_max_flow(maxFlowGraph, source, end, edgeCapacity, residual_capacity, rev, indexmap);
            #else
                flow = push_relabel_max_flow(maxFlowGraph, source, end);
            #endif
            if (flow == skills.size()) {
                //
                solution.clear();
                boost::graph_traits<MFGraph>::vertex_iterator u_iter, u_end;
                boost::graph_traits<MFGraph>::out_edge_iterator ei, e_end;
                for (boost::tie(u_iter, u_end) = vertices(maxFlowGraph); u_iter != u_end; ++u_iter)
                {
                    if (*u_iter==0||*u_iter>skills.size()) {
                        continue;
                    }
                    for (boost::tie(ei, e_end) = out_edges(*u_iter, maxFlowGraph); ei != e_end; ++ei)
                    {
                        if (edgeCapacity[*ei] > 0 && edgeCapacity[*ei] - residual_capacity[*ei] > 0)
                            solution.insert(skillHolders[target(*ei, maxFlowGraph) -skills.size()-1].name);
                            //std::cout << "f " << *u_iter << " " << target(*ei, g) << " "
                            //<< (capacity[*ei] - residual_capacity[*ei]) << std::endl;
                    }
                }
                r_end = r_mid;
                if (r_mid < minDiaIndex) {
                    minDiaIndex = r_mid;
                }
                if (r_end<=r_begin) {
                    break;
                }
                
            }
            else if (flow < skills.size())
            {
                r_begin = r_mid + 1;
                if (r_begin >= r_end) {
                    break;
                }
            }
            else
            {
                std::cout << "ERROR" << std::endl;
                break;
            }
        }
        
        // Algorithm finished
        std::time_t realEnd = clock();
        std::cout << taskIndex << '\t' << skillHolders[minDiaIndex].distance << '\t' <<
        (clock() - start)*1000.0/CLOCKS_PER_SEC<<"\t"<<(realEnd - realStart)*1000.0/CLOCKS_PER_SEC <<
        '\t'<< candidateIndex;
        if (skillHolders[r_end].distance>9999) {
            std::cout<<std::endl;
            continue;
        }
        for (auto reqSkill:skills)
        {
            std::cout<<"\t"<<reqSkill.getName();
        }
        if (solution.empty())
            continue;
        std::set<unsigned long>experts;
        for (auto expert : solution)
        {
            experts.insert(expert);
            unsigned long des = p[expert];
            do
            {
                experts.insert(des);
                des = p[des];
            }while(des != candidateIndex);
        }
        for (auto expert : experts)
        {
            std::cout<<"\t"<<expert;
        }
        std::cout<<std::endl;
        
        
    }
    std::cout<< "Hello, World!\n";
    return 0;

}
