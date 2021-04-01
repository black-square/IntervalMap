#include "IntervalMap.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <boost/icl/interval_map.hpp>
#include <random>
#include <fstream>

//////////////////////////////////////////////////////////////////////////

#define CHECK(cond) (void)( (!!(cond)) || (std::terminate(), 0) )

template<class CharT, class TraitsT, class K, class V>
std::basic_ostream<CharT, TraitsT> &operator<<( std::basic_ostream<CharT, TraitsT> &os, const interval_map<K, V> &map )
{
  map.print( os );
  return os;  
}
//////////////////////////////////////////////////////////////////////////
template<class K, class V>
void assign_check( interval_map<K, V> &map, 
  const typename interval_map<K, V>::key_type &keyFirst, 
  const typename interval_map<K, V>::key_type &keyLast, 
  const typename interval_map<K, V>::mapped_type &val, 
  const char *szContent
)
{
  std::cout << '[' << keyFirst << ", " << keyLast << ") = " << val << std::endl;
  map.set( keyFirst, keyLast, val );
  map.print_dbg( std::cout );
  std::cout << "     " << map << std::endl << std::endl;

  std::stringstream content;
  content << map;
  
  CHECK( content.str() == szContent );
}
//////////////////////////////////////////////////////////////////////////
 
void BasicOperations()
{
  interval_map<unsigned short, std::string> map("Z");

  assign_check(map, 0, 0, "Y","[0,65535)=Z");
  assign_check(map, 3, 7, "B","[0,3)=Z [3,7)=B [7,65535)=Z"); 
  assign_check(map, 5, 9, "C","[0,3)=Z [3,5)=B [5,9)=C [9,65535)=Z");
  assign_check(map, 6, 7, "D","[0,3)=Z [3,5)=B [5,6)=C [6,7)=D [7,9)=C [9,65535)=Z");
  assign_check(map, 3, 5, "F","[0,3)=Z [3,5)=F [5,6)=C [6,7)=D [7,9)=C [9,65535)=Z");
  assign_check(map, 0, 5, "E","[0,5)=E [5,6)=C [6,7)=D [7,9)=C [9,65535)=Z");
  assign_check(map, 3, 3, "G","[0,5)=E [5,6)=C [6,7)=D [7,9)=C [9,65535)=Z"); 
  assign_check(map, 0, 10, "A","[0,10)=A [10,65535)=Z");
}
//////////////////////////////////////////////////////////////////////////

void ElementAccess()
{
  interval_map<unsigned short, char> map('Z');
  assign_check(map, 3, 7, 'B',"[0,3)=Z [3,7)=B [7,65535)=Z"); 
  assign_check(map, 5, 9, 'C',"[0,3)=Z [3,5)=B [5,9)=C [9,65535)=Z");
  
  const char rgTst[] = "ZZZBBCCCCZZZZZZZZZZZZZ";
  
  for( unsigned short i = 0; i < std::size(rgTst) - 1; ++i )
    CHECK( rgTst[i] == map[i] );   
}
//////////////////////////////////////////////////////////////////////////

void IntervalMerge()
{
  interval_map<unsigned short, std::string> map("Z");

  assign_check(map, 0, 0, "Y", "[0,65535)=Z");
  assign_check(map, 3, 7, "Z", "[0,65535)=Z");
  assign_check(map, 3, 7, "B", "[0,3)=Z [3,7)=B [7,65535)=Z"); 
  assign_check(map, 5, 9, "C", "[0,3)=Z [3,5)=B [5,9)=C [9,65535)=Z");
  assign_check(map, 3, 4, "Z", "[0,4)=Z [4,5)=B [5,9)=C [9,65535)=Z");
  assign_check(map, 0, 5, "Z", "[0,5)=Z [5,9)=C [9,65535)=Z");
  assign_check(map, 10, 20, "Z", "[0,5)=Z [5,9)=C [9,65535)=Z");
  assign_check(map, 7, 20, "Z", "[0,5)=Z [5,7)=C [7,65535)=Z");

  auto map2{ map }, map3{ map };

  assign_check(map, 2, 5, "Z", "[0,5)=Z [5,7)=C [7,65535)=Z");
  assign_check(map2, 2, 8, "Z", "[0,65535)=Z");
  assign_check(map, 2, 7, "Z", "[0,65535)=Z");
  assign_check(map3, 0, 7, "Z", "[0,65535)=Z");
}
//////////////////////////////////////////////////////////////////////////


template<class CharT, class TraitsT, class K, class V>
void print( std::basic_ostream<CharT, TraitsT> &os, const boost::icl::interval_map<K, V> &map )
{
  for( auto it = map.begin(); it != map.end(); ++it )
  {

    if( it != map.begin() )
      os << " ";   

    os << it->first << "=" << it->second;
  }
}
//////////////////////////////////////////////////////////////////////////
 
template<class T = std::mt19937, std::size_t N = T::state_size * sizeof( typename T::result_type )>
T ProperlySeededRandomEngine()
{
    std::random_device source;
    std::random_device::result_type random_data[(N - 1) / sizeof( source() ) + 1];
    std::generate( std::begin( random_data ), std::end( random_data ), std::ref( source ) );
    std::seed_seq seeds( std::begin( random_data ), std::end( random_data ) );
    return T{ seeds };
}
//////////////////////////////////////////////////////////////////////////
void RandomCompareWithBoostIcl()
{
  namespace icl = boost::icl;
  typedef std::uniform_int_distribution<> TRngGen;
  typedef unsigned short TKey;
  typedef char TVal;

  interval_map<TKey, TVal> map('A');  
  icl::interval_map<TKey, TVal> bstMap;
  bstMap.set( std::make_pair( icl::interval<TKey>::right_open(0, std::numeric_limits<TKey>::max()), 'A') );
  
  auto rng { ProperlySeededRandomEngine() };
  //TRngGen keysRnd(0, std::numeric_limits<TKey>::max() - 1 );
  TRngGen keysRnd(0, 20 );
  TRngGen valsRnd('A', 'A' + 10 );

  std::ofstream flOut("RandomCompareWithBoostIclOutput.txt"); 

  for( int i = 0; i < 100000; ++i )
  {
    TKey left( keysRnd(rng) ), right( keysRnd(rng) );
    TVal val( valsRnd(rng) ); 

    if( left > right )
      std::swap(left, right);

    map.set( left, right, val );
    bstMap.set( std::make_pair( icl::interval<TKey>::right_open(left, right), val) );

    std::stringstream myContent;
    myContent << map;

    std::stringstream bstContent;
    print( bstContent, bstMap );

    flOut << '[' << left << ", " << right << ") = " << val << " | " << myContent.str() << std::endl;

    if( bstMap.iterative_size() == 1 )
      flOut << "<<EMPTY>>" << std::endl;  

    CHECK( myContent.str() == bstContent.str() );
  }
}
//////////////////////////////////////////////////////////////////////////

int main()
{
  BasicOperations();
  ElementAccess();
  IntervalMerge();
  RandomCompareWithBoostIcl();

  std::cout << "Done" << std::endl;

  std::cin.get();
	return 0;
}

