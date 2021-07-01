#pragma once

#include <tuple>
#include <tmp/type_list.hpp>

template<class MainComponent_t, class... ExtraComponents_t>
struct SystemBase_t
{
    virtual ~SystemBase_t() = default;
    using SystemSignature_t = TMP::TypeList_t<MainComponent_t, ExtraComponents_t...>;
};
