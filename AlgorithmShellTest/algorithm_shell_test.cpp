#include <gtest/gtest.h>
#include "../AlgorithmShell.h"

void algorithm_test_1()
{
    ALGORITHM_DEPENDS(data_1, char);
    ALGORITHM_DEPENDS(data_2, int);
    ALGORITHM_DEPENDS(data_3, float);
    ALGORITHM_DEPENDS(data_4, std::string);

    ALGORITHM_UPDATES(data_a, char);
    ALGORITHM_UPDATES(data_b, int);
    ALGORITHM_UPDATES(data_c, float);
    ALGORITHM_UPDATES(data_d, std::string);

    data_a = data_1;
    data_b = data_2;

    data_c = data_3;
    data_d = data_4;
}

ALGORITHM_LINKAGE(data_1, algorithm_test_1);
ALGORITHM_LINKAGE(data_2, algorithm_test_1);
ALGORITHM_LINKAGE(data_3, algorithm_test_1);
ALGORITHM_LINKAGE(data_4, algorithm_test_1);


TEST(algorithm_shell_test, BasicTest)
{
    Dataset& ds = ALGSHL.getDataset();

    ds.set("data_1", dw::any('a'));
    ds.set("data_2", dw::any(20));
    ds.set("data_3", dw::any(60.0f));
    ds.set("data_4", dw::any("Algorithm shell test."));

    algorithm_test_1();

    ASSERT_TRUE(ds.check("data_a", 'a'));
    ASSERT_TRUE(ds.check("data_b", 20));
    ASSERT_TRUE(ds.check("data_c", 60.0f));
    ASSERT_TRUE(ds.check("data_d", "Algorithm shell test."));
}


TEST(algorithm_shell_test, LinkageTest)
{
    Dataset& ds = ALGSHL.getDataset();

    ds.set("data_1", dw::any('b'));
    ds.set("data_2", dw::any(240));
    ds.set("data_3", dw::any(80.0f));
    ds.set("data_4", dw::any("Algorithm linkage test."));

    ds.set("data_a", dw::any('x'));
    ds.set("data_b", dw::any(2));
    ds.set("data_c", dw::any(10.0f));
    ds.set("data_d", dw::any("xxxxxxxxxxxxxxxxxxxxx"));

    ALGSHL.enableLinkage(true);

    ds.set("data_1", dw::any('d'));
    ASSERT_TRUE(ds.check("data_a", 'd'));
    ASSERT_TRUE(ds.check("data_b", 240));
    ASSERT_TRUE(ds.check("data_c", 80.0f));
    ASSERT_TRUE(ds.check("data_d", "Algorithm linkage test."));

    ds.set("data_2", dw::any(500));
    ASSERT_TRUE(ds.check("data_a", 'd'));
    ASSERT_TRUE(ds.check("data_b", 500));
    ASSERT_TRUE(ds.check("data_c", 80.0f));
    ASSERT_TRUE(ds.check("data_d", "Algorithm linkage test."));

    ds.set("data_3", dw::any(666.66f));
    ASSERT_TRUE(ds.check("data_a", 'd'));
    ASSERT_TRUE(ds.check("data_b", 500));
    ASSERT_TRUE(ds.check("data_c", 666.66f));
    ASSERT_TRUE(ds.check("data_d", "Algorithm linkage test."));

    ds.set("data_4", dw::any("Successful"));
    ASSERT_TRUE(ds.check("data_a", 'd'));
    ASSERT_TRUE(ds.check("data_b", 500));
    ASSERT_TRUE(ds.check("data_c", 666.66f));
    ASSERT_TRUE(ds.check("data_d", "Successful"));
}
