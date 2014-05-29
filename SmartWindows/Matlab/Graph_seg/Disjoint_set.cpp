#include "StdAfx.h"
#include "Disjoint_set.h"

Disjoint_set::Disjoint_set(int num_elts)
{
	m_elts = new disjoint_elt[num_elts];

    int i;
    for(i=0; i<num_elts; i++)
    {
        m_elts[i].m_depth = 0;
        m_elts[i].m_p = i;
    }
}


Disjoint_set::~Disjoint_set()
{
	delete [] m_elts;

}

int Disjoint_set::find(int x)
{
	int RegionId = x;
    int next;
	//upstream, find RegionId
    while( RegionId!= m_elts[RegionId].m_p )
    {
        RegionId = m_elts[RegionId].m_p;
    }
    //Downstream, mark the whole region as 'RegionId'
    while( m_elts[x].m_p!=RegionId)
    {
        next = m_elts[x].m_p;
        m_elts[x].m_p=RegionId;
        x=next;
    }
    
    return RegionId;
}

int Disjoint_set::joint(int x_elt, int y_elt)
{

	int x = find(x_elt);
	int y = find(y_elt);
	if(x == y)
	{
		//cout<<endl;
		return -1;
	}


	if(m_elts[x].m_depth > m_elts[y].m_depth)
	{
		m_elts[y].m_p = x;
		return x;
	}
	else if(m_elts[y].m_depth > m_elts[x].m_depth)
	{
		m_elts[x].m_p = y;
		return y;
	}
	else
	{
		m_elts[y].m_p = x;
		m_elts[x].m_depth++;
		return x;
	}

}