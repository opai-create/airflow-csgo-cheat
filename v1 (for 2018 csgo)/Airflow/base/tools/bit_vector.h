#pragma once
#include <cstdint>

namespace utils
{
  // DON'T USE IT
  // u should use bitsXX instead
  template < typename type_t = uint32_t >
  class c_bit_vector
  {
  private:
    type_t m_value = 0u;
    using this_type = c_bit_vector< type_t >;

  public:
    __forceinline constexpr c_bit_vector( )
    {
      m_value = 0u;
    }

    __forceinline constexpr c_bit_vector( type_t value )
    {
      m_value = value;
    }

    class iterator
    {
    private:
      uint8_t m_position = 0;
      c_bit_vector* m_parent = nullptr;

    public:
      __forceinline constexpr iterator( c_bit_vector* parent, uint8_t position ): m_parent( parent ), m_position( position ){ };

      __forceinline constexpr void operator=( uint8_t new_flag )
      {
        m_parent->set( m_position, new_flag );
      }
      __forceinline constexpr operator bool( ) const
      {
        return this->get( );
      }
      __forceinline constexpr bool get( ) const
      {
        return m_parent->at( m_position );
      }
    };

    __forceinline constexpr type_t& get( )
    {
      return m_value;
    }

    __forceinline constexpr type_t get( ) const
    {
      return m_value;
    }

    __forceinline constexpr this_type& set( uint8_t position, uint8_t value )
    {
      if( value )
        m_value |= 1 << position;
      else
        m_value &= ~( 1 << position );

      return *this;
    }

    __forceinline constexpr this_type& inverse( uint8_t position )
    {
      static_assert( position < sizeof( type_t ) * 8u, "c_bit_vector subscript out of range" );
      m_value ^= 1 << position;
      return *this;
    }

    __forceinline constexpr this_type& _xor( uint32_t number )
    {
      for( uint8_t i = 0u; i < sizeof( type_t ) * 8u; ++i )
      {
        if( ( this->m_value & ( 1 << i ) ) == ( number & ( 1 << i ) ) )
          this->set( i, false );
        else
          this->set( i, true );
      }
      return *this;
    }

    __forceinline constexpr bool at( uint8_t i ) const
    {
      static_assert( i < sizeof( type_t ) * 8u, "c_bit_vector subscript out of range" );
      return m_value & i;
    }

    __forceinline constexpr iterator operator[]( uint8_t i )
    {
      return iterator( this, i );
    }

    __forceinline constexpr bool operator[]( uint8_t i ) const
    {
      static_assert( i < sizeof( type_t ) * 8u, "c_bit_vector subscript out of range" );
      return this->at( i );
    }
  };
}

using bits8 = utils::c_bit_vector< uint8_t >;
using bits16 = utils::c_bit_vector< uint16_t >;
using bits32 = utils::c_bit_vector< uint32_t >;
using bits64 = utils::c_bit_vector< uint64_t >;