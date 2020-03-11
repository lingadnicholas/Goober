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
    string angleToDir(double angle, bool& proceed) const;

    void findRoute(const GeoCoord& start,
        const GeoCoord& end,
        list<StreetSegment>& route,
        vector<DeliveryCommand>& commands,
        double& totalDistanceTravelled) const;
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
    vector<DeliveryRequest> newDeliveries = deliveries;
    m_optimize.optimizeDeliveryOrder(depot, newDeliveries, oldDist, newDist); 
    list<StreetSegment> route;
    auto it = newDeliveries.begin(); 
    
    commands.clear();
    int deliveryIndex = 0; 
    GeoCoord start = depot; 

    DeliveryCommand food;
    for (; it != newDeliveries.end(); it++)
    {
      findRoute(start, it->location, route, commands, totalDistanceTravelled); 
      //This is a delivery
      food.initAsDeliverCommand(newDeliveries[deliveryIndex].item);
      commands.push_back(food);
      start = it->location;
      ++deliveryIndex;
    }
    findRoute(start, depot, route, commands, totalDistanceTravelled); 
    return DELIVERY_SUCCESS;  
    
}

void DeliveryPlannerImpl::findRoute(const GeoCoord& start,
    const GeoCoord& end,
    list<StreetSegment>& route,
    vector<DeliveryCommand>& commands,
    double& totalDistanceTravelled) const
{
    route.clear();
    DeliveryResult tempRes = m_router.generatePointToPointRoute(start, end, route, totalDistanceTravelled);
    auto routeIt = route.begin();
    DeliveryCommand cur;
    while (routeIt != route.end())
    {
        string streetName;
        double procDist = 0; //proceed distance
        procDist = 0;
        StreetSegment s;
        s.start = routeIt->start; s.end = routeIt->end;
        streetName = routeIt->name;

        bool proceed;

        if (!commands.empty() && (commands[commands.size() - 1].streetName() != "" && commands[commands.size() - 1].streetName() != streetName))
            //It's a turn (or just proceeding because you can't turn)
        {
            proceed = false;
            double angle = angleOfLine(s);
            string dir = angleToDir(angle, proceed);
            if (!proceed) //You are turning! 
            {
                cur.initAsTurnCommand(dir, streetName);
                commands.push_back(cur);
                //Next command will be a proceed
                proceed = true;
                ++routeIt;
                if (routeIt == route.end())
                    break;
                s.start = routeIt->start; s.end = routeIt->end;
                angle = angleOfLine(s);
                procDist = distanceEarthMiles(s.start, s.end);
                string dir = angleToDir(angle, proceed);
                cur.initAsProceedCommand(dir, streetName, procDist);
                commands.push_back(cur);
                continue; //Don't ++ again!
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
        else if (!commands.empty() && (streetName == commands[commands.size() - 1].streetName())) //You are on the same street, so proceed. 
        {
            //Angle of direction should be based on the first street segment of command
            proceed = true;
            double segmentDist = distanceEarthMiles(s.start, s.end);
            commands[commands.size() - 1].increaseDistance(segmentDist);
        }
        else //This is the first or the last was a delivery
        {
            //Calculate the distance it will travel this segment and push a new command onto the commands vector
            proceed = true;
            procDist = distanceEarthMiles(s.start, s.end);
            double angle = angleOfLine(s);
            string dir = angleToDir(angle, proceed);
            cur.initAsProceedCommand(dir, streetName, procDist);
            commands.push_back(cur);
        }



        ++routeIt;

    }
}

//Proceed is true if command is proceed, false if command is turn
string DeliveryPlannerImpl::angleToDir(double angle, bool& proceed) const
{
    if (!proceed)
    {
        if (angle < 1 || angle > 359)
        {
            proceed = true;
        }
        if (angle >= 1 && angle < 180)
        {
            return "left";
        }
        else
            return "right";
    }
    if (proceed)
    {
        if (angle < 0)
            angle += 360;
        if (angle >= 0 && angle < 22.5)
        {
            return "east";
        }
        else if (angle >= 22.5 && angle < 67.5)
        {
            return "northeast";
        }
        else if (angle >= 67.5 && angle < 112.5)
        {
            return "north";
        }
        else if (angle >= 112.5 && angle < 157.5)
        {
            return "northwest";
        }
        else if (angle >= 157.5 && angle < 202.5)
        {
            return "west";
        }
        else if (angle >= 202.5 && angle < 247.5)
        {
            return "southwest";
        }
        else if (angle >= 247.5 && angle < 292.5)
        {
            return "south";
        }
        else if (angle >= 292.5 && angle < 337.5)
        {
            return "southeast";
        }
        else
            return "east";
    }
    return "ERROR";
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

