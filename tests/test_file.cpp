#include "rlog.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// -------------------------------------------------------------------

namespace {

// -------------------------------------------------------------------

using namespace std;
using namespace testing;

// -------------------------------------------------------------------

TEST(FileTest, basic) {
    const char* filename = "outputs/file_output.txt";
    ofstream of;
    of.open(filename);

    R::reset(R::Level::Warning);

    R::addSink(R::FileSink(of));

    R::addSink(R::makeSmartFormattedCoutSink());

    R_WARNING("filetest") << "XYZ";  // will get printed
    R_INFO("filetest") << "ABC";     // will get filtered

    ifstream ifs(filename);
    string content((istreambuf_iterator<char>(ifs)),
                   (istreambuf_iterator<char>()));

    EXPECT_EQ(content, "XYZ\n");
}

// -------------------------------------------------------------------

}  // namespace

// -------------------------------------------------------------------
