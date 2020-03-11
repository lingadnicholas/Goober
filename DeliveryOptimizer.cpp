#include "provided.h"
#include <vector>
#include <random>
//#include <utility> 
using namespace std;

class DeliveryOptimizerImpl
{
public:
    DeliveryOptimizerImpl(const StreetMap* sm);
    ~DeliveryOptimizerImpl();
    void optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const;
private: 
    const StreetMap* m_streetMap;

    double computeCrowDistance(const GeoCoord& depot, const vector<DeliveryRequest>& deliveries) const
    {
        double crowDistance = 0;
        GeoCoord start = depot; 
        for (int i = 0; i < deliveries.size(); i++)
        {
            GeoCoord dest = deliveries[i].location;
            crowDistance += distanceEarthMiles(start, dest);
            start = dest;
        }
        
        crowDistance += distanceEarthMiles(start, depot); 

        return crowDistance; 
    }

    // Return a uniformly distributed random int from min to max, inclusive

    
        int randInt(int min, int max) const
    {
        if (max < min)
            std::swap(max, min);
        static std::random_device rd;
        static std::default_random_engine generator(rd());
        std::uniform_int_distribution<> distro(min, max);
        return distro(generator);
    }

};

DeliveryOptimizerImpl::DeliveryOptimizerImpl(const StreetMap* sm)
: m_streetMap(sm)
{
}

DeliveryOptimizerImpl::~DeliveryOptimizerImpl()
{
}

void DeliveryOptimizerImpl::optimizeDeliveryOrder(
    const GeoCoord& depot,
    vector<DeliveryRequest>& deliveries,
    double& oldCrowDistance,
    double& newCrowDistance) const
{
    //First, compute old crow distance. 
    oldCrowDistance = computeCrowDistance(depot, deliveries); 
    double tempCrowDistance = oldCrowDistance;
    GeoCoord start = depot; 
    vector<DeliveryRequest> actualBestSol = deliveries; //Keeps track of best
    vector<DeliveryRequest> tempSol = deliveries;
    //Attempt at simulated annealing: 
    //i is how many chances you get to improve, j is a hard cap
    
    for (int i = 0, j = 0,  probability = 90; j < 500; i++, j++, probability -= 4) //Cap number of attempts = to 5000
    {
        if (i >= 50) //No improvements after 50 tries.
            break; 
        bool prob = false; 
        if (probability < 0)
            probability = 0; 
        //Compute probability that IF this iteration doesn't work, to use it anyway
        int rng = randInt(1, 100); 
        if (rng <= probability)
            prob = true; 
        //Generate two random numbers to swap
        int swap1 = randInt(0, deliveries.size() - 1); 
        int swap2; 
        do {
            swap2 = randInt(0, deliveries.size() - 1);
        } while (swap1 == swap2); 
        swap(tempSol[swap1], tempSol[swap2]); 
        newCrowDistance = computeCrowDistance(depot, tempSol); 
        if (newCrowDistance < tempCrowDistance)
        {
            tempCrowDistance = newCrowDistance; 
            i = 0; //Found a new good solution, 50 more tries!
            actualBestSol = tempSol; //Keep track of this & keep going with tempSol
        }
        else if (prob) //Probability tells you to go with the current "bad" solution
            continue;//Gives you more chances
        else //Or else, go back to trying with the current best solution that you have.
            tempSol = actualBestSol; 
    }
    //Compute new crow distance
    newCrowDistance = computeCrowDistance(depot, actualBestSol);
    deliveries = actualBestSol; 
}

//******************** DeliveryOptimizer functions ****************************

// These functions simply delegate to DeliveryOptimizerImpl's functions.
// You probably don't want to change any of this code.

DeliveryOptimizer::DeliveryOptimizer(const StreetMap* sm)
{
    m_impl = new DeliveryOptimizerImpl(sm);
}

DeliveryOptimizer::~DeliveryOptimizer()
{
    delete m_impl;
}

void DeliveryOptimizer::optimizeDeliveryOrder(
        const GeoCoord& depot,
        vector<DeliveryRequest>& deliveries,
        double& oldCrowDistance,
        double& newCrowDistance) const
{
    return m_impl->optimizeDeliveryOrder(depot, deliveries, oldCrowDistance, newCrowDistance);
}
