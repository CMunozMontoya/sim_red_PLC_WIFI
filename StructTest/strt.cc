#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/string.h"
#include "ns3/log.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include "ns3/mobility-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/mobility-model.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/ssid.h"
#include <vector>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("prueba");


class Leaf
{
  public:
    std::vector<Leaf*> next;
    std::vector<double> distance;
    uint32_t id;

    Leaf (void) {}

    Leaf (uint32_t id)
    {
        this->id = id;
    }

    void AddVecino (Leaf* l)
    {
      next.push_back (l);
      distance.push_back (0);
    }

    void AddVecino (Leaf* l, double d)
    {
        if (d < 0)

        {
            std::cout << "Distancia no Valida!! " << std::endl;
            return;
        }
        next.push_back (l);
        distance.push_back (d);
    }

    void ModDistance (uint i, double d)
    {
        if (d < 0)
        {
            std::cout << "Distancia no Valida!! " << std::endl;
            return;
        }
        for (uint n = 0 ; n < next.size () ; n++)
        {
            if (next[n]->id == i)
            {
                distance[n] = d;
                return;
            }
        }
    }

    uint32_t Get_id ()
    {
      return id;
    }

    void Print_id (void)
    {
        std::cout << id << std::endl;
    }
};

class Grafo
{
  public:
    Leaf root;

    Grafo (void) {}

    Grafo (Leaf r)
    {
      this->root = r;
    }

    void PrintLeafs (Leaf *r)
    {
        for(uint i = 0 ; i < r->next.size () ; i++)
        {
            r->next[i]->Print_id ();
        }
    }

    void PrintG (Leaf *r)
    {
        for (uint i = 0 ; i < r->next.size () ; i++)
        {
            r->next[i]->Print_id ();
            PrintG(r->next[i]);
        }
    }

   double GetDistanceToNext (Leaf *r, uint i)
   {
       for (uint n = 0 ; n < r->next.size () ; n++ )
       {
            if (r->next[n]->id == i)
                return r->distance[n];
       }
       return 0;
   }

    double GetDistanceToNext (Leaf *r, Leaf *l)
    {
       for (uint n = 0 ; n < r->next.size () ; n++ )
       {
            if (r->next[n] == l)
                return r->distance[n];
       }
       return 0;
    }

    double GetDistance (Leaf *r, uint id_a, uint id_b)
    {
        if (r->id == id_a)
        {
            return GetDistanceToNext (r, id_b);
        }
        for (uint n = 0 ; n < r->next.size () ; n++ )
        {
            return GetDistance (r->next[n], id_a, id_b);
        }
        return 0;
    }

    void GoToLeafId (Leaf *r, uint i)
    {
        if (r->id == i)
        {
            std::cout << r->id << std::endl;
            return;
        }
        for (uint n = 0 ; n < r->next.size () ; n++ )
        {
            GoToLeafId (r->next[n], i);
        }
        return;
    }
};

double GetDistanceToNextId (Leaf *r, uint i)
{
    for (uint n = 0 ; n < r->next.size () ; n++ )
    {
        if (r->next[n]->id == i)
            return r->distance[n];
    }
    return 0;
}

double GetDistanceToNext (Leaf *r, Leaf *n)
{
    for (uint i = 0 ; i < r->next.size () ; i++ )
    {
        if (r->next[i] == n)
            return r->distance[i];
    }
    return 0;
}

double GetDistance (std::vector<Leaf*>* path)
{  
    double dis;

    if (path->size () < 2)
    {
        std::cout << "error, vetor no apto" << std::endl;
        return -1;
    }
    for (uint i = 1 ; i < path->size () ; i++ )
    {
        //Leaf* r = &path->at(i-1);
        dis += GetDistanceToNext (path->at(i-1), path->at(i));
    }
    return dis;
}

void PrintLeafs (Leaf *r)
{
    for(uint i = 0 ; i < r->next.size () ; i++)
    {
        //r->next[i]->Print_id ();
        std::cout << "id: " << r->next[i]->id;
        std::cout << " d = " << r->distance[i] << std::endl;
    }
}

void CreateL (std::vector<Leaf> *container, uint n)
{
    uint s = container->size();
    for (uint i = 0 ; i < n ; i++)
    {
        Leaf l (i+s);
        container->push_back(l);
    }
}

int main (int argc, char *argv[])
{  
    std::vector<Leaf> Ench;
    CreateL(&Ench,13);
    Ench[0].AddVecino(&Ench[1],3.0);
        Ench[1].AddVecino(&Ench[2],4.0);
    Ench[0].AddVecino(&Ench[3],3.5);
        Ench[3].AddVecino(&Ench[4],2.2);
        Ench[3].AddVecino(&Ench[5],3.2);
            Ench[5].AddVecino(&Ench[6],2.0);
    Ench[0].AddVecino(&Ench[10],3.0);
    Ench[0].AddVecino(&Ench[11],2.0);
        Ench[11].AddVecino(&Ench[12],5.2);
    Ench[0].AddVecino(&Ench[7],5.5);
        Ench[7].AddVecino(&Ench[8],2.0);
            Ench[8].AddVecino(&Ench[9],3.5);


    std::vector<Leaf> Inter;
    CreateL(&Inter,15);
    Inter[0].AddVecino(&Inter[1],0.5);
        Inter[1].AddVecino(&Inter[2],2.5);
        Inter[1].AddVecino(&Inter[3],4.0);
    Inter[0].AddVecino(&Inter[11],4.0);
        Inter[11].AddVecino(&Inter[12],5.5);
    Inter[0].AddVecino(&Inter[4],3.5);
        Inter[4].AddVecino(&Inter[5],3.5);
    Inter[0].AddVecino(&Inter[13],3.5);
        Inter[13].AddVecino(&Inter[14],3.0);
    Inter[0].AddVecino(&Inter[6],6.0);
        Inter[6].AddVecino(&Inter[7],3.0);
        Inter[6].AddVecino(&Inter[8],4.0);
            Inter[8].AddVecino(&Inter[10],1.0);
            Inter[8].AddVecino(&Inter[9],1.0);

    std::vector<Leaf*> path = {&Ench[0], &Ench[3], &Ench[5], &Ench[6]};

    std::cout << GetDistance (&path) << std::endl;

    return 0;
}