#include <gtest/gtest.h>

// Sample function to test
int add(int a, int b) {
    return a + b;
}

class HelloWorldTest : public::testing::Test {

};

TEST_F(HelloWorldTest, TestAdd) {
    EXPECT_EQ(add(2, 2), 4);
    EXPECT_EQ(add(-1, 1), 0);
}

TEST_F(HelloWorldTest, AnotherTest) {
    EXPECT_TRUE(true);
}
