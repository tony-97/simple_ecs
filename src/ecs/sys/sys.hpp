#include <tuple>

template<class... Args_t>
struct TypeList
{
    template<std::size_t N>
    using type = typename std::tuple_element<N, std::tuple<Args_t...>>::type;
};

template<class MainComponent_t, class... ExtraComponents_t>
struct SystemBase_t
{
    using SystemSignature_t = TypeList<MainComponent_t, ExtraComponents_t...>;
};
