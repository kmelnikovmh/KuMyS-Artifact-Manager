#include <gtest/gtest.h>
#include <iostream>
#include <sstream>
#include "ServerApp.h"

using namespace std;

class MainTest : public ::testing::Test {
protected:
    streambuf* orig_cout;
    streambuf* orig_cerr;
    streambuf* orig_cin;
    ostringstream output;
    ostringstream err;

    void SetUp() override {
        orig_cout = cout.rdbuf(output.rdbuf());
        orig_cerr = cerr.rdbuf(err.rdbuf());
        orig_cin = cin.rdbuf();
    }

    void TearDown() override {
        cout.rdbuf(orig_cout);
        cerr.rdbuf(orig_cerr);
        cin.rdbuf(orig_cin);
        
        output.str("");
        err.str("");
    }
};

TEST_F(MainTest, MainReturnsZero) {
    istringstream input("\n");
    cin.rdbuf(input.rdbuf());

    int ret = runServerApp();
    EXPECT_EQ(ret, 0);
}

TEST_F(MainTest, PrintsStartupMessage) {
    istringstream input("\n");
    cin.rdbuf(input.rdbuf());

    runServerApp();
    EXPECT_NE(output.str().find("Start server-setup"), string::npos);
}
