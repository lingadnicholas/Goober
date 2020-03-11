
#include "provided.h"
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;
void test4()
{
        StreetMap sm;

        sm.load("C:\\Users\\linga\\OneDrive - UCLA IT Services\\Project4\\mapdata.txt");
        PointToPointRouter p(&sm);

        GeoCoord start("34.0625329", "-118.4470263");
        //GeoCoord end("34.0636533", "-118.4470480");
        GeoCoord end("34.0636344", "-118.4482275");
        //GeoCoord end("34.0593696","-118.4455875");

        list <StreetSegment> seg;
        vector <StreetSegment> segs;
        double dist;


        //testing deliveries
        GeoCoord depot = start;
        vector<DeliveryRequest> deliveries;
        deliveries.push_back(DeliveryRequest("popcorn", GeoCoord("34.0712323", "-118.4505969")));
        deliveries.push_back(DeliveryRequest("cake", GeoCoord("34.0687443", "-118.4449195")));
        deliveries.push_back(DeliveryRequest("coffee", GeoCoord("34.0685657", "-118.4489289")));
        deliveries.push_back(DeliveryRequest("pizza", GeoCoord("34.0718238", "-118.4525699")));

        deliveries.push_back(DeliveryRequest("coffee", GeoCoord("34.0666168", "-118.4395786")));


        DeliveryPlanner dp(&sm);
        vector<DeliveryCommand> dcs;
        double totalMiles;

        dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);

        cout << totalMiles;
}
void test3()
{
        StreetMap sm;

        sm.load("C:\\Users\\linga\\OneDrive - UCLA IT Services\\Project4\\mapdata.txt");
        PointToPointRouter p(&sm);

        GeoCoord start("34.0625329", "-118.4470263");
        GeoCoord end("34.0636533", "-118.4470480");
        //GeoCoord end("34.0636344", "-118.4482275");
        list <StreetSegment> seg;
        vector <StreetSegment> segs;
        double dist;

        sm.getSegmentsThatStartWith(start, segs);
        p.generatePointToPointRoute(start, end, seg, dist);

        //testing deliveries
        GeoCoord depot = end;
        vector<DeliveryRequest> deliveries;
        deliveries.push_back(DeliveryRequest("popcorn", GeoCoord("34.0636344", "-118.4482275")));
        deliveries.push_back(DeliveryRequest("cake", GeoCoord("34.0608001", "-118.4457307")));
        deliveries.push_back(DeliveryRequest("coffee", GeoCoord("34.0438832", "-118.4950204")));
        deliveries.push_back(DeliveryRequest("soda", GeoCoord("34.0666168", "-118.4395786")));

        DeliveryPlanner dp(&sm);
        vector<DeliveryCommand> dcs;
        double totalMiles;

        dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
        cout << totalMiles;
  

}
void test2()
{
    StreetMap sm;

    sm.load("C:\\Users\\linga\\OneDrive - UCLA IT Services\\Project4\\mapdata.txt");
    PointToPointRouter p(&sm);

    GeoCoord start("34.0625329", "-118.4470263");
    GeoCoord end("34.0636533", "-118.4470480");
    list <StreetSegment> seg;
    vector <StreetSegment> segs;
    double dist;

    sm.getSegmentsThatStartWith(start, segs);
    p.generatePointToPointRoute(start, end, seg, dist);
    cout << "Done" << endl;
}

void test1()
{
    GeoCoord first;
    GeoCoord second("34.0555267", "118.4796954");
    GeoCoord third("34.0555267", "118.4796954");
    GeoCoord fourth("4.0555267", "18.4796954");
    GeoCoord fifth("324.0555267", "11.479694");
    GeoCoord sixth("134.0555267", "8.479654");
    GeoCoord seventh("374.0555267", "18.47954");
    GeoCoord eighth("384.0555267", "1.4796954");
    GeoCoord ninth("394.0555267", "8.479654");
    GeoCoord tenth("3224.0555267", "18.479654");
    GeoCoord eleventh("314.0555267", "1128.479954");

    ExpandableHashMap<GeoCoord, double> hashing;
    hashing.associate(first, 30); 
    hashing.associate(second, 50); 
    assert(hashing.size() == 2); 
    hashing.associate(second, 60);
    assert(hashing.size() == 2); 
    hashing.reset(); 
    cout << hashing.size() << endl; 
    assert(hashing.size() == 0); 
    hashing.associate(second, 70); 
    assert(hashing.size() == 1);

    //Looks like there is an error in expanding the table.
    hashing.associate(third, 10);
    hashing.associate(fourth, 10);
    hashing.associate(fifth, 10);
    hashing.associate(sixth, 10);
    hashing.associate(seventh, 10); 
    hashing.associate(eighth, 10);
    hashing.associate(ninth, 10); //Throws an exception when trying to copy this one
    hashing.associate(tenth, 10);
    hashing.associate(eleventh, 10);
    cout << "All tests passed" << endl; 
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

int main()
{
    StreetMap sm;
    //"C:\\Users\\linga\\OneDrive - UCLA IT Services\\Project4\\mapdata.txt"
    //"E:\\OneDrive - UCLA IT Services\\Project4\\mapdata.txt"
    if (!sm.load("mapdata.txt"))
    {
        std::cout << "Unable to load map data file " << endl;
        return 1;
    }
    
    GeoCoord depot;
    vector<DeliveryRequest> deliveries;
    //"C:\\Users\\linga\\OneDrive - UCLA IT Services\\Project4\\deliveries.txt"
    //"E:\\OneDrive - UCLA IT Services\\Project4\\deliveries.txt"
    if (!loadDeliveryRequests("deliveries.txt", depot, deliveries))
    {
        std::cout << "Unable to load delivery request file " << endl;
        return 1;
    }

    std::cout << "Generating route...\n\n";

    DeliveryPlanner dp(&sm);
    vector<DeliveryCommand> dcs;
    double totalMiles;
    DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
    if (result == BAD_COORD)
    {
        std::cout << "One or more depot or delivery coordinates are invalid." << endl;
        return 1;
    }
    if (result == NO_ROUTE)
    {
        std::cout << "No route can be found to deliver all items." << endl;
        return 1;
    }
    std::cout << "Starting at the depot...\n";
    for (const auto& dc : dcs)
        std::cout << dc.description() << endl;
    std::cout << "You are back at the depot and your deliveries are done!\n";
    std::cout.setf(ios::fixed);
    std::cout.precision(2);
    std::cout << totalMiles << " miles travelled for all deliveries." << endl;
}

bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v)
{
    ifstream inf(deliveriesFile);
    if (!inf)
        return false;
    string lat;
    string lon;
    inf >> lat >> lon;
    inf.ignore(10000, '\n');
    depot = GeoCoord(lat, lon);
    string line;
    while (getline(inf, line))
    {
        string item;
        if (parseDelivery(line, lat, lon, item))
            v.push_back(DeliveryRequest(item, GeoCoord(lat, lon)));
    }
    return true;
}

bool parseDelivery(string line, string& lat, string& lon, string& item)
{
    const size_t colon = line.find(':');
    if (colon == string::npos)
    {
        cout << "Missing colon in deliveries file line: " << line << endl;
        return false;
    }
    istringstream iss(line.substr(0, colon));
    if (!(iss >> lat >> lon))
    {
        cout << "Bad format in deliveries file line: " << line << endl;
        return false;
    }
    item = line.substr(colon + 1);
    if (item.empty())
    {
        cout << "Missing item in deliveries file line: " << line << endl;
        return false;
    }
    return true;

    
}



