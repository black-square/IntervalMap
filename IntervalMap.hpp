#ifndef IntervalMap_h__
#define IntervalMap_h__

#include <map>
#include <cassert>
#include <iomanip>
#include <iterator>

template<class KeyT, class ValueT>
class interval_map 
{
  friend void IntervalMapTest();

public: 
  using key_type = KeyT;
  using mapped_type = ValueT;

public:
  explicit interval_map( const mapped_type &val)
  {
    m_map.emplace_hint(m_map.begin(), std::numeric_limits<key_type>::lowest(), val);
  };

  void set( const key_type &keyFirst, const key_type &keyLast, const mapped_type &val ) 
  {
    assert( keyLast < std::numeric_limits<key_type>::max() );
    assert( !m_map.empty() );

    if( !(keyFirst < keyLast) )
      return;

    const auto beginIt = m_map.lower_bound( keyFirst );
    const auto endIt = m_map.upper_bound( keyLast );

    assert( endIt != m_map.begin() );  
    mapped_type rightKey( std::prev(endIt)->second );

    const bool leftKeyNeedInsert = beginIt == m_map.begin() || std::prev(beginIt)->second != val;
    const bool rightKeyNeedInsert = rightKey != val;

    m_map.erase( beginIt, endIt );

    if( leftKeyNeedInsert )
    {
      const auto it = m_map.emplace_hint( endIt, keyFirst, val );
      assert( it != endIt );
    }
    
    if( rightKeyNeedInsert )
    { 
      const auto it = m_map.emplace_hint( endIt, keyLast, std::move(rightKey) );
      assert( it != endIt );
    }  
  }

  const mapped_type & operator[]( const key_type &key ) const
  {
    //Key set mustn't include the MAX value because there is no way to change it correctly
    assert( key < std::numeric_limits<key_type>::max() );
    return std::prev(m_map.upper_bound(key))->second;
  }

  template<class CharT, class TraitsT>
  void print_dbg( std::basic_ostream<CharT, TraitsT> &os ) const;

  template<class CharT, class TraitsT>
  void print( std::basic_ostream<CharT, TraitsT> &os ) const;
  
private:
  using TMap = std::map<key_type, mapped_type>;
  using value_type = typename TMap::value_type;

private:	
  TMap m_map;
};
//////////////////////////////////////////////////////////////////////////

template<class key_type, class mapped_type>
template<class CharT, class TraitsT>
void interval_map<key_type, mapped_type>::print_dbg( std::basic_ostream<CharT, TraitsT> &os ) const
{
  for( auto &v : m_map )
    os << std::setw(4) << v.first << ' ';

  os << std::endl;

  for( auto &v : m_map )
    os << std::setw(4) << v.second << ' ';

  os << std::endl;
}
//////////////////////////////////////////////////////////////////////////

template<class key_type, class mapped_type>
template<class CharT, class TraitsT>
void interval_map<key_type, mapped_type>::print( std::basic_ostream<CharT, TraitsT> &os ) const
{
  for( auto it = m_map.begin(); it != m_map.end(); ++it )
  {
    const auto itNext = std::next(it);
    const key_type nextKey = itNext != m_map.end() ? itNext->first : std::numeric_limits<key_type>::max();

    if( it != m_map.begin() )
      os << " ";   

    os << '[' << it->first << ',' << nextKey << ")=" << it->second;
  }
}



#endif // IntervalMap_h__

