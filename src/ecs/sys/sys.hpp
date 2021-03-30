#include <tuple>

template<class... Args_t>
struct TypeList
{
    template<std::size_t N>
    using type = typename std::tuple_element<N, std::tuple<Args_t...>>::type;
    static constexpr std::size_t size { sizeof...(Args_t) };
};

struct System_t
{
    virtual ~System_t() = default;
};

template<typename GameCTX_t, typename Return_t, typename MainCMP_t, typename ...ExtraCMPs_t>
struct SystemBase_t : System_t, public TypeList<MainCMP_t, ExtraCMPs_t...>
{
    explicit SystemBase_t(GameCTX_t& ent_man)
        : m_EntMan { ent_man }
    {  }

    virtual Return_t Update() = 0;
    virtual void UpdateOneEntity(MainCMP_t&, ExtraCMPs_t&...) = 0;

    GameCTX_t& m_EntMan;
};

struct CollisionSystem_t : SystemBase_t<int, float, char&>
{
    CollisionSystem_t(int& ref_int)
        : SystemBase_t(ref_int)
    {
        
    }

    void UpdateOneEntity(char&) override
    {
        
    }

    float Update() override
    {
        return 3.0f;
    }

};

struct Data_t
{
protected:
    int x {  };
};

template<class C>
struct MyStructure_t : private Data_t
{
    friend C;
    MyStructure_t&& Create(C& c)
    {
        (void)c;
        return MyStructure_t{};
    }
};

struct Class_t
{
    void func(MyStructure_t<Class_t>& mys)
    {
        struct Other : Data_t {} o;
        (void)o;
        ++mys.x;
    }
};

struct Other : Data_t {};
