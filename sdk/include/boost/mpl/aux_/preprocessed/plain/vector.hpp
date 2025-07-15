// preprocessed version of 'boost/mpl/vector.hpp' header
// see the original for copyright information

namespace boost {
namespace mpl {

template<
      typename T0 = void, typename T1 = void, typename T2 = void
    , typename T3 = void, typename T4 = void, typename T5 = void
    , typename T6 = void, typename T7 = void, typename T8 = void
    , typename T9 = void
    >
struct vector;

template<
      
    >
struct vector<
          void, void, void, void, void, void, void, void, void
        , void
        >
    : vector0<  >
{
    typedef vector0<  > type;
};

template<
      typename T0
    >
struct vector<
          T0, void, void, void, void, void, void, void, void, void
        >
    : vector1<T0>
{
    typedef vector1<T0> type;
};

template<
      typename T0, typename T1
    >
struct vector<
          T0, T1, void, void, void, void, void, void, void, void
        >
    : vector2< T0,T1 >
{
    typedef vector2< T0,T1 > type;
};

template<
      typename T0, typename T1, typename T2
    >
struct vector< T0,T1,T2,void,void,void,void,void,void,void >
    : vector3< T0,T1,T2 >
{
    typedef vector3< T0,T1,T2 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3
    >
struct vector< T0,T1,T2,T3,void,void,void,void,void,void >
    : vector4< T0,T1,T2,T3 >
{
    typedef vector4< T0,T1,T2,T3 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    >
struct vector< T0,T1,T2,T3,T4,void,void,void,void,void >
    : vector5< T0,T1,T2,T3,T4 >
{
    typedef vector5< T0,T1,T2,T3,T4 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    , typename T5
    >
struct vector< T0,T1,T2,T3,T4,T5,void,void,void,void >
    : vector6< T0,T1,T2,T3,T4,T5 >
{
    typedef vector6< T0,T1,T2,T3,T4,T5 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    , typename T5, typename T6
    >
struct vector< T0,T1,T2,T3,T4,T5,T6,void,void,void >
    : vector7< T0,T1,T2,T3,T4,T5,T6 >
{
    typedef vector7< T0,T1,T2,T3,T4,T5,T6 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    , typename T5, typename T6, typename T7
    >
struct vector< T0,T1,T2,T3,T4,T5,T6,T7,void,void >
    : vector8< T0,T1,T2,T3,T4,T5,T6,T7 >
{
    typedef vector8< T0,T1,T2,T3,T4,T5,T6,T7 > type;
};

template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    , typename T5, typename T6, typename T7, typename T8
    >
struct vector< T0,T1,T2,T3,T4,T5,T6,T7,T8,void >
    : vector9< T0,T1,T2,T3,T4,T5,T6,T7,T8 >
{
    typedef vector9< T0,T1,T2,T3,T4,T5,T6,T7,T8 > type;
};

// primary template (not a specialization!)
template<
      typename T0, typename T1, typename T2, typename T3, typename T4
    , typename T5, typename T6, typename T7, typename T8, typename T9
    >
struct vector
    : vector10< T0,T1,T2,T3,T4,T5,T6,T7,T8,T9 >
{
    typedef vector10< T0,T1,T2,T3,T4,T5,T6,T7,T8,T9 > type;
};

} // namespace mpl
} // namespace boost

