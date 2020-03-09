#include "provided.h"
#include <vector>
using namespace std;

class DeliveryPlannerImpl
{
public:
    DeliveryPlannerImpl(const StreetMap* sm);
    ~DeliveryPlannerImpl();
    DeliveryResult generateDeliveryPlan(
        const GeoCoord& depot,
        const vector<DeliveryRequest>& deliveries,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
private: 
    DeliveryOptimizer m_optimize; 
    PointToPointRouter m_router; 
    const StreetMap* m_streetMap;

    //Proceed is true if command is proceed, false if command is turn
    string angleToDir(const double& angle, bool &proceed) const
    {
        if (!proceed)
        {
            if (angle < 1 || angle > 359)
            {
                proceed = true; //TODO: but what angle should it return? For a negative angle
            }
            if (angle >= 1 && angle < 180)
            {
                return "Turn left";
            }
            else
                return "Turn right"; 
        }
        if (proceed)
        {
            if (angle >= 0 && angle < 22.5)
            {
                return "Proceed east";
            }
            else if (angle >= 22.5 && angle < 67.5)
            {
                return "Proceed northeast";
            }
            else if (angle >= 67.5 && angle < 112.5)
            {
                return "Proceed north";
            }
            else if (angle >= 112.5 && angle < 157.5)
            {
                return "Proceed northwest";
            }
            else if (angle >= 157.5 && angle < 202.5)
            {
                return "Proceed west";
            }
            else if (angle >= 202.5 && angle < 247.5)
            {
                return "Proceed southwest";
            }
            else if (angle >= 247.5 && angle < 292.5)
            {
                return "Proceed south";
            }
            else if (angle >= 292.5 && angle < 337.5)
            {
                return "Proceed southeast";
            }
            else
                return "Proceed east";
        }

    }
};

DeliveryPlannerImpl::DeliveryPlannerImpl(const StreetMap* sm)
    : m_streetMap(sm), m_router(sm), m_optimize(sm)
{
    
}

DeliveryPlannerImpl::~DeliveryPlannerImpl()
{
}

DeliveryResult DeliveryPlannerImpl::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    
    double oldDist;
    double newDist; 
    //m_optimize.optimizeDeliveryOrder(depot, deliveries, oldDist, newDist); 
    list<StreetSegment> route;
    auto it = deliveries.begin(); 
    
    int deliveryIndex = 0; 
    GeoCoord start = depot; 
    for (; it != deliveries.end(); ++it) //Generate routes into route list
    {
        double thisDist; 
        DeliveryResult tempRes = m_router.generatePointToPointRoute(start, it->location, route, totalDistanceTravelled);
        start = it->location; 
    }

    //Generate a route back into route list 
    m_router.generatePointToPointRoute(depot, start, route, totalDistanceTravelled); 

    commands.clear(); 
    auto routeIt = route.begin(); 

    string streetName;

    double procDist = 0; //proceed distance
    bool lastWasDelivery = false; 
    for (; routeIt != route.end(); ++routeIt) //TODO: Increment vecit whenever it's a new cmd
    {
        procDist = 0; 
        DeliveryCommand cur;
        StreetSegment s;
        s.start = routeIt->start; s.end = routeIt->end;
        streetName = routeIt->name; 
       
        bool proceed; 
        
        if (lastWasDelivery || commands.empty() ) 
            //This is the first, or the last command was deliver
        {
            lastWasDelivery = false; 
            //Calculate the distance it will travel this segment and push a new command onto the commands vector
            proceed = true; 
            procDist = distanceEarthMiles(s.start, s.end);
            double angle = angleOfLine(s); 
            string dir = angleToDir(angle, proceed); 
            cur.initAsProceedCommand(dir, streetName, procDist); 
            commands.push_back(cur); 
        }
        else if (!commands.empty() && streetName == commands[commands.size()-1].streetName()) //You are on the same street, so proceed. 
            {
                //Angle of direction should be based on the first street segment of command
            lastWasDelivery = false; 
                proceed = true; 
                double segmentDist = distanceEarthMiles(s.start, s.end);
                commands[commands.size() - 1].increaseDistance(segmentDist); 
            }
        else if (deliveries[deliveryIndex].location == s.start) //This is a delivery
        {
            lastWasDelivery = true; 
            cur.initAsDeliverCommand(deliveries[deliveryIndex].item); 
        }
        else //It's a turn (or just proceeding because you can't turn)
        {
            lastWasDelivery = false; 
            proceed = false; 
            double angle = angleOfLine(s); 
            string dir = angleToDir(angle, proceed); 
            if (!proceed) //You are turning! 
            {
                cur.initAsTurnCommand(dir, streetName); 
                commands.push_back(cur); 
                //Next command should be a proceed.
             
            }
            else if (proceed) //Push a proceed command instead because you're not ~really~ turning 😍🤤🌚💦💩💩
            {
                procDist = distanceEarthMiles(s.start, s.end);
                double angle = angleOfLine(s);
                string dir = angleToDir(angle, proceed);
                cur.initAsProceedCommand(dir, streetName, procDist);
                commands.push_back(cur); 
            }
        }
    }
    
    return DELIVERY_SUCCESS;  
    
}

//******************** DeliveryPlanner functions ******************************

// These functions simply delegate to DeliveryPlannerImpl's functions.
// You probably don't want to change any of this code.

DeliveryPlanner::DeliveryPlanner(const StreetMap* sm)
{
    m_impl = new DeliveryPlannerImpl(sm);
}

DeliveryPlanner::~DeliveryPlanner()
{
    delete m_impl;
}

DeliveryResult DeliveryPlanner::generateDeliveryPlan(
    const GeoCoord& depot,
    const vector<DeliveryRequest>& deliveries,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    return m_impl->generateDeliveryPlan(depot, deliveries, commands, totalDistanceTravelled);
}
