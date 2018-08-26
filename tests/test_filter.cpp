#include "rlog.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// -------------------------------------------------------------------

namespace {

// -------------------------------------------------------------------

using namespace testing;

struct MockSink {
    MOCK_METHOD1(message, void(const std::string &));
};

// -------------------------------------------------------------------

TEST(FilterTest, basic) {
    StrictMock<MockSink> m_mocksink;

    R::reset(R::Level::Info);
    R::addSink(R::makeFilteredSink(
        R_SINK_W_CAPTURE(m, s, &) { m_mocksink.message(s); },
        R_FILTER(m, s) { return m.tag == "A" || m.tag == "C"; }));
    R::addSink(R::makeSmartFormattedCoutSink());

    EXPECT_CALL(m_mocksink, message(StrEq("X"))).RetiresOnSaturation();
    EXPECT_CALL(m_mocksink, message(StrEq("Z"))).RetiresOnSaturation();

    R_INFO("A") << "X";
    R_INFO("B") << "Y";
    R_INFO("C") << "Z";
}

// -------------------------------------------------------------------

}  // namespace

// -------------------------------------------------------------------
