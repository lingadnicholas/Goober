#include "provided.h"
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cassert>
using namespace std;
int main()
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

    //Error destructing 
}
/*
bool loadDeliveryRequests(string deliveriesFile, GeoCoord& depot, vector<DeliveryRequest>& v);
bool parseDelivery(string line, string& lat, string& lon, string& item);

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Usage: " << argv[0] << " mapdata.txt deliveries.txt" << endl;
        return 1;
    }

    StreetMap sm;
        
    if (!sm.load(argv[1]))
    {
        cout << "Unable to load map data file " << argv[1] << endl;
        return 1;
    }

    GeoCoord depot;
    vector<DeliveryRequest> deliveries;
    if (!loadDeliveryRequests(argv[2], depot, deliveries))
    {
        cout << "Unable to load delivery request file " << argv[2] << endl;
        return 1;
    }

    cout << "Generating route...\n\n";

    DeliveryPlanner dp(&sm);
    vector<DeliveryCommand> dcs;
    double totalMiles;
    DeliveryResult result = dp.generateDeliveryPlan(depot, deliveries, dcs, totalMiles);
    if (result == BAD_COORD)
    {
        cout << "One or more depot or delivery coordinates are invalid." << endl;
        return 1;
    }
    if (result == NO_ROUTE)
    {
        cout << "No route can be found to deliver all items." << endl;
        return 1;
    }
    cout << "Starting at the depot...\n";
    for (const auto& dc : dcs)
        cout << dc.description() << endl;
    cout << "You are back at the depot and your deliveries are done!\n";
    cout.setf(ios::fixed);
    cout.precision(2);
    cout << totalMiles << " miles travelled for all deliveries." << endl;
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
*/