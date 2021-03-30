#pragma once

namespace ECS
{

template<typename T, typename ID_t>
struct IdentificableBase_t
{
    constexpr IdentificableBase_t<T, ID_t>()  = default;
    virtual  ~IdentificableBase_t<T, ID_t>()  = default;
public:
    static inline ID_t NextIdentificator {  };
};

template<typename T, typename ID_t, typename INC_CB_t>
struct IdentificableForEachInstance_t 
    : public IdentificableBase_t<
                        IdentificableForEachInstance_t<T, ID_t, INC_CB_t>,
                        ID_t>
{
    virtual ~IdentificableForEachInstance_t<T, ID_t, INC_CB_t>() = default;

    constexpr IdentificableForEachInstance_t() = default;
    constexpr IdentificableForEachInstance_t(ID_t id) : m_InstID { id } {  }

    const ID_t m_InstID
    {
        INC_CB_t()(IdentificableBase_t<
                        IdentificableForEachInstance_t<T, ID_t, INC_CB_t>,
                        ID_t>::NextIdentificator)
    };
};

} // namespace ECS
