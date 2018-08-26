#include "rlog.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// -------------------------------------------------------------------

namespace {

// -------------------------------------------------------------------

using namespace testing;

struct MockSink {
    MOCK_METHOD1(message, void(const std::string&));
};

// -------------------------------------------------------------------

TEST(FormatterTest, basic) {
    StrictMock<MockSink> m_mocksink;

    R::reset(R::Level::Info);

    R::addSink(R::makeFormattedSink(
        R_SINK_W_CAPTURE(m, s, &) { m_mocksink.message(s); },
        R::makeSmartFormatter()));

    R::addSink(R::makeSmartFormattedCoutSink());

    EXPECT_CALL(m_mocksink, message(_))
        .WillOnce(Invoke([](const std::string& message) {
            auto has = [](const std::string& s1, const std::string& s2) {
                return s1.find(s2) != std::string::npos;
            };
            EXPECT_TRUE(has(message, "[R]"));
            EXPECT_TRUE(has(message,
                            "[Warning] #FormatterTest (test_formatter.cpp:" +
                                std::to_string(__LINE__ + 3) + ") XYZ"));
        }));

    R_WARNING("FormatterTest") << "XYZ";
}

// -------------------------------------------------------------------

}  // namespace

// -------------------------------------------------------------------
