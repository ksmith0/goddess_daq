#include <iostream>
#include <string>
#include <vector>
#include "GoddessStruct.h"

template<typename T> T* GetMemberFromName ( std::string name , SiDataBase* siDat )
{
//     if ( name == "dE_eSum_p" ) return ( T* ) &siDat->dE_eSum_p;
//     else if ( name == "dE_eSum_n" ) return ( T* ) &siDat->dE_eSum_n;
//     else if ( name == "E1_eSum_p" ) return ( T* ) &siDat->E1_eSum_p;
//     else if ( name == "E1_eSum_n" ) return ( T* ) &siDat->E1_eSum_n;
//     else if ( name == "E2_eSum_p" ) return ( T* ) &siDat->E2_eSum_p;
//     else if ( name == "E2_eSum_n" ) return ( T* ) &siDat->E2_eSum_n;
//     else if ( name == "dE_stripMax_p" ) return ( T* ) &siDat->dE_stripMax_p;
//     else if ( name == "dE_stripMax_n" ) return ( T* ) &siDat->dE_stripMax_n;
//     else if ( name == "E1_stripMax_p" ) return ( T* ) &siDat->E1_stripMax_p;
//     else if ( name == "E1_stripMax_n" ) return ( T* ) &siDat->E1_stripMax_n;
//     else if ( name == "E2_stripMax_p" ) return ( T* ) &siDat->E2_stripMax_p;
//     else if ( name == "E2_stripMax_n" ) return ( T* ) &siDat->E2_stripMax_n;
//     else return 0;

    return 0;
}

template<typename T> T* GetMemberFromName ( std::string name , SiDataDetailed* siDat )
{
//     if ( name == "dE_e_p" ) return ( T* ) &siDat->dE_e_p;
//     else if ( name == "dE_e_n" ) return ( T* ) &siDat->dE_e_n;
//     else if ( name == "E1_e_p" ) return ( T* ) &siDat->E1_e_p;
//     else if ( name == "E1_e_n" ) return ( T* ) &siDat->E1_e_n;
//     else if ( name == "E2_e_p" ) return ( T* ) &siDat->E2_e_p;
//     else if ( name == "E2_e_n" ) return ( T* ) &siDat->E2_e_n;
//     else if ( name == "dE_strip_p" ) return ( T* ) &siDat->dE_strip_p;
//     else if ( name == "dE_strip_n" ) return ( T* ) &siDat->dE_strip_n;
//     else if ( name == "E1_strip_p" ) return ( T* ) &siDat->E1_strip_p;
//     else if ( name == "E1_strip_n" ) return ( T* ) &siDat->E1_strip_n;
//     else if ( name == "E2_strip_p" ) return ( T* ) &siDat->E2_strip_p;
//     else if ( name == "E2_strip_n" ) return ( T* ) &siDat->E2_strip_n;
//     else return GetMemberFromName<T> ( name , dynamic_cast<SiDataBase*> ( siDat ) );

    if ( name == "dE_e_p" ) return ( T* ) &siDat->dE.en.p;
    else if ( name == "dE_e_n" ) return ( T* ) &siDat->dE.en.n;
    else if ( name == "E1_e_p" ) return ( T* ) &siDat->E1.en.p;
    else if ( name == "E1_e_n" ) return ( T* ) &siDat->E1.en.n;
    else if ( name == "E2_e_p" ) return ( T* ) &siDat->E2.en.p;
    else if ( name == "E2_e_n" ) return ( T* ) &siDat->E2.en.n;
    else if ( name == "dE_strip_p" ) return ( T* ) &siDat->dE.strip.p;
    else if ( name == "dE_strip_n" ) return ( T* ) &siDat->dE.strip.n;
    else if ( name == "E1_strip_p" ) return ( T* ) &siDat->E1.strip.p;
    else if ( name == "E1_strip_n" ) return ( T* ) &siDat->E1.strip.n;
    else if ( name == "E2_strip_p" ) return ( T* ) &siDat->E2.strip.p;
    else if ( name == "E2_strip_n" ) return ( T* ) &siDat->E2.strip.n;
    else return GetMemberFromName<T> ( name , dynamic_cast<SiDataBase*> ( siDat ) );

//     return GetMemberFromName<T> ( name , dynamic_cast<SiDataBase*> ( siDat ) );
}

SiDataBase::SiDataBase() {}

SiDataBase::~SiDataBase() {}

void SiDataBase::Clear()
{
//     dE_eSum_p = 0.0;
//     dE_eSum_n = 0.0;
//     E1_eSum_p = 0.0;
//     E1_stripMax_n = 0.0;
//     E2_eSum_p = 0.0;
//     E2_eSum_n = 0.0;
//
//     dE_stripMax_p = -1;
//     dE_stripMax_n = -1;
//     E1_stripMax_p = -1;
//     E1_stripMax_n = -1;
//     E2_stripMax_p = -1;
//     E2_stripMax_n = -1;

    eSum.clear();
    stripMax.clear();
}

float* SiDataBase::SetMemberAddress ( std::string member, float** address )
{
    *address = GetMemberFromName<float> ( member, this );
    return *address;
}

int* SiDataBase::SetMemberAddress ( std::string member, int** address )
{
    *address = GetMemberFromName<int> ( member, this );
    return *address;
}

std::vector<float>* SiDataBase::SetMemberAddress ( std::string member, std::vector<float>** address )
{
    *address = GetMemberFromName<std::vector<float>> ( member, this );
    return *address;
}

std::vector<int>* SiDataBase::SetMemberAddress ( std::string member, std::vector<int>** address )
{
    *address = GetMemberFromName<std::vector<int>> ( member, this );
    return *address;
}

float SiDataBase::eSumLayer ( short unsigned int layer, bool isNType ) const
{
    if ( eSum.size() > 0 )
        for ( unsigned short i = 0; i < eSum.size(); i++ )
            if ( stripMax[i] >= ( layer*100 + 300*isNType )  && stripMax[i] < ( layer*100 + 300*isNType ) + 100 )
                return eSum[i];

    return 0.0;
}

int SiDataBase::stripMaxLayer ( short unsigned int layer, bool isNType ) const
{
    if ( stripMax.size() > 0 )
        for ( unsigned short i = 0; i < stripMax.size(); i++ )
            if ( stripMax[i] >= ( layer*100 + 300*isNType )  && stripMax[i] < ( layer*100 + 300*isNType ) + 100 )
                return stripMax[i] - ( layer*100 + 300*isNType );

    return -1;
}

ClassImp ( SiDataBase )


SiDataDetailed::SiDataDetailed() {}
SiDataDetailed::~SiDataDetailed() {}

void SiDataDetailed::Clear()
{
    SiDataBase::Clear();

//     dE_e_p.clear();
//     dE_e_n.clear();
//     E1_e_p.clear();
//     E1_e_n.clear();
//     E2_e_p.clear();
//     E2_e_n.clear();
//
//     dE_strip_p.clear();
//     dE_strip_n.clear();
//     E1_strip_p.clear();
//     E1_strip_n.clear();
//     E2_strip_p.clear();
//     E2_strip_n.clear();

    dE.Clear();
    E1.Clear();
    E2.Clear();

    SiDataBase::Clear();
}

float* SiDataDetailed::SetMemberAddress ( std::string member, float** address )
{
    *address = GetMemberFromName<float> ( member, this );
    return *address;
}

int* SiDataDetailed::SetMemberAddress ( std::string member, int** address )
{
    *address = GetMemberFromName<int> ( member, this );
    return *address;
}

std::vector<float>* SiDataDetailed::SetMemberAddress ( std::string member, std::vector<float>** address )
{
    *address = GetMemberFromName<std::vector<float>> ( member, this );
    return *address;
}

std::vector<int>* SiDataDetailed::SetMemberAddress ( std::string member, std::vector<int>** address )
{
    *address = GetMemberFromName<std::vector<int>> ( member, this );
    return *address;
}

ClassImp ( SiDataDetailed )
