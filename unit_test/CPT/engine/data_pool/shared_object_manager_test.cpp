#include <CPT/engine/data_pool/shared_object_manager.hpp>
#include <Nucleona/app/cli/gtest.hpp>

template <unsigned n>
struct Q
{
  public:
    Q(void)  { std::cerr << "Q" << n << " at " << this << " created\n"; }
    ~Q(void) { std::cerr << "Q" << n << " at " << this << " deleted\n"; }
};

struct DataPool
     : public cpt::engine::data_pool::SharedObjectManager
{};
DataPool db;

struct Component_1
{
    std::shared_ptr<Q<1>> Q1;
    std::shared_ptr<Q<2>> Q2;

    Component_1(void)
    {
        std::cerr << "Init Component_1\n";
    }
    void config_parameter(void)
    {
        Q1 = db.require_shared<Q<1>>("Q1");
        Q2 = db.require_shared<Q<2>>("Q2");
    }
    void start(void)
    {
        std::cerr << "Component_1 does something with Q1 and Q2\n";
    }
    void finish(void)
    {
        std::cerr << "Component_1 finished\n";
        Q1 = nullptr;
        Q2 = nullptr;
    }
};
struct Component_2
{
    std::shared_ptr<Q<2>> Q2;
    std::shared_ptr<Q<3>> Q3;

    Component_2(void)
    {
        std::cerr << "Init Component_2\n";
    }
    void config_parameter(void)
    {
        Q2 = db.require_shared<Q<2>>("Q2");
        Q3 = db.require_shared<Q<3>>("Q3");
    }
    void start(void)
    {
        std::cerr << "Component_2 does something with Q2 and Q3\n";
    }
    void finish(void)
    {
        Q2 = nullptr;
        Q3 = nullptr;
    }
};

TEST( SharedObjectManager, test )
{
    std::cerr << "a simulated pipeline started\n";
    {
        Component_1 c1; c1.config_parameter();
        Component_2 c2; c2.config_parameter();
        db.start_shared_object_management();
        c1.start();
        c1.finish();
        c2.start();
        c2.finish();
    }
    std::cerr << "a simulated pipeline finished\n";
}
