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
    std::vector<Leaf*> next; //"ramas" del Leaf [i]
    std::vector<double> distance; //distancia a la rama [i]
    Leaf *parent; //Leaf "origen". en el root original es void
    uint32_t id;

    Leaf (void) {}

    Leaf (uint32_t id)
    {
        this->id = id;
        this->parent = NULL;
    }

    void AddVecino (Leaf* l) //DONE
    {
      next.push_back (l);
      distance.push_back (0);
    }

    void AddVecino (Leaf* l, double d) //DONE
    {
        if (d < 0)

        {
            std::cout << "Distancia no Valida!! " << std::endl;
            return;
        }
        next.push_back (l);
        distance.push_back (d);
    }
    
    //añade un vecino incluyendo quien es su "parent" | uso de pruebas
    void AddVecino (Leaf* l, double d, Leaf* p) //DONE
    {
        if (d < 0)

        {
            std::cout << "Distancia no Valida!! " << std::endl;
            return;
        }
        this->parent = p;
        next.push_back (l);
        distance.push_back (d);
    }

    //modifica el Leaf "padre"
    void ModParent (Leaf *p) //DONE
    {
        this->parent = p;
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

//encuentra la distancia entre un noodo y el siguiente segun el id del nodo
double GetDistanceToNextId (Leaf *r, uint i) //WORK
{
    for (uint n = 0 ; n < r->next.size () ; n++ )
    {
        if (r->next[n]->id == i)
            return r->distance[n];
    }
    return 0;
}

//encuentra la distancia entre un nodo y el siguiente 
double GetDistanceToNext (Leaf *r, Leaf *n) //WORK

{
    for (uint i = 0 ; i < r->next.size () ; i++ )
    {
        if (r->next[i] == n)
            return r->distance[i];
    }
    return 0;
}

//encuentra la distancia dado un camino determinado
double GetDistance (std::vector<Leaf*>* path) //WORKING
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

std::vector<Leaf*> GetPathToRoot (Leaf* r, std::vector<Leaf*> container)
{
    if(r->parent == NULL)
    {
        container.push_back(r);
        return container;
    }

    container.push_back(r);
    return GetPathToRoot (r->parent, container);  
}

void printCont (std::vector<Leaf*> container)
{
    for(uint32_t i = 0 ; i<container.size() ; i++)
    {
        std::cout << container[i]->Get_id() << " <- " ;
    }
    std::cout<<std::endl;
}

int main (int argc, char *argv[])
{  
    /*
    std::vector<Leaf> Ench;
    CreateL(&Ench,13);
    Ench[0].AddVecino(&Ench[1],3.0, NULL);
        Ench[1].AddVecino(&Ench[2],4.0, &Ench[0]);
    Ench[0].AddVecino(&Ench[3],3.5);
        Ench[3].AddVecino(&Ench[4],2.2, &Ench[0]);
        Ench[3].AddVecino(&Ench[5],3.2, &Ench[0]);
            Ench[5].AddVecino(&Ench[6],2.0);
    Ench[0].AddVecino(&Ench[10],3.0);
    Ench[0].AddVecino(&Ench[11],2.0);
        Ench[11].AddVecino(&Ench[12],5.2, &Ench[0]);
    Ench[0].AddVecino(&Ench[7],5.5);
        Ench[7].AddVecino(&Ench[8],2.0, &Ench[0]);
            Ench[8].AddVecino(&Ench[9],3.5, &Ench[7]);


    std::vector<Leaf> Inter;
    CreateL(&Inter,15);
    Inter[0].AddVecino(&Inter[1],0.5, NULL);
        Inter[1].AddVecino(&Inter[2],2.5, &Inter[0]);
        Inter[1].AddVecino(&Inter[3],4.0, &Inter[0]);
    Inter[0].AddVecino(&Inter[11],4.0);
        Inter[11].AddVecino(&Inter[12],5.5, &Inter[0]);
    Inter[0].AddVecino(&Inter[4],3.5);
        Inter[4].AddVecino(&Inter[5],3.5, &Inter[0]);
    Inter[0].AddVecino(&Inter[13],3.5);
        Inter[13].AddVecino(&Inter[14],3.0, &Inter[0]);
    Inter[0].AddVecino(&Inter[6],6.0);
        Inter[6].AddVecino(&Inter[7],3.0, &Inter[0]);
        Inter[6].AddVecino(&Inter[8],4.0, &Inter[0]);
            Inter[8].AddVecino(&Inter[10],1.0, &Inter[6]);
            Inter[8].AddVecino(&Inter[9],1.0, &Inter[6]);
    */

    std::vector<Leaf> Test;
    CreateL(&Test, 5);
    Test[0].AddVecino(&Test[1], 1, NULL);
        Test[1].AddVecino(&Test[2], 2, &Test[0]);
            Test[2].AddVecino(&Test[3], 2, &Test[1]);
                Test[3].AddVecino(&Test[4], 2, &Test[2]);
                Test[4].ModParent(&Test[3]);
    //std::vector<Leaf*> path = {&Ench[0], &Ench[3], &Ench[5], &Ench[6]};
    std::vector<Leaf*> path;
    path = GetPathToRoot(&Test[4],path);

    printCont(path);

    std::reverse(path.begin(),path.end());

    std::cout << GetDistance (&path) << std::endl;

    return 0;
}