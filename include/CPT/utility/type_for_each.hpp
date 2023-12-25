#pragma once
template<
      class TypeList
    , template<class T> class Func
>
struct TypeForEach
{};

template<
      template<class... T> class List
    , template<class T> class Func
    , class First
    , class... Tail
>
struct TypeForEach < List<First, Tail...>, Func >
{
    static void run ()
    {
        Func<First>::run();
        TypeForEach<List<Tail...>, Func>::run();
    }
};
template<
      template<class... T> class List
    , template<class T> class Func
>
struct TypeForEach < List<>, Func >
{
    static void run ()
    {}
};
