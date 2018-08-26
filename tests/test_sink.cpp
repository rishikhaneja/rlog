#include "rlog.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

// -------------------------------------------------------------------

namespace {

// -------------------------------------------------------------------

using namespace std;
using namespace testing;

// -------------------------------------------------------------------

struct MockSink {
    MOCK_METHOD1(message, void(const std::string&));
    MOCK_METHOD1(level, void(R::Level));
    MOCK_METHOD1(filename, void(const std::string&));
    MOCK_METHOD1(line, void(long));
    MOCK_METHOD1(timestamp, void(const std::string&));
    MOCK_METHOD1(tag, void(const std::string&));
};

// -------------------------------------------------------------------

struct SinkTest : Test {
    SinkTest() {
        R::reset(R::Level::Info);
        R::addSink(R_SINK_W_CAPTURE(m, s, this) {
            m_mocksink.level(m.level);
            m_mocksink.filename(m.filename);
            m_mocksink.line(m.line);
            m_mocksink.timestamp(m.timestamp);
            m_mocksink.tag(m.tag);
            m_mocksink.message(s);
        });
        R::addSink(R::makeSmartFormattedCoutSink());
    }
    virtual ~SinkTest() override {}
    StrictMock<MockSink> m_mocksink;
};

// -------------------------------------------------------------------

TEST_F(SinkTest, basic) {
    EXPECT_CALL(m_mocksink, level(R::Level::Warning));
    EXPECT_CALL(m_mocksink, filename("test_sink.cpp"));
    EXPECT_CALL(m_mocksink, line(__LINE__ + 4));
    EXPECT_CALL(m_mocksink, timestamp(_));
    EXPECT_CALL(m_mocksink, tag("SinkTest"));
    EXPECT_CALL(m_mocksink, message("XYZ\n"));
    R_WARNING("SinkTest") << "XYZ" << endl;
    EXPECT_CALL(m_mocksink, level(R::Level::Info));
    EXPECT_CALL(m_mocksink, filename("test_sink.cpp"));
    EXPECT_CALL(m_mocksink, line(__LINE__ + 4));
    EXPECT_CALL(m_mocksink, timestamp(_));
    EXPECT_CALL(m_mocksink, tag("SinkTest"));
    EXPECT_CALL(m_mocksink, message("ABC"));
    R_INFO("SinkTest") << "ABC";
}

// -------------------------------------------------------------------

struct Date {
    Date(int day, int month, int year) : day(day), month(month), year(year) {}
    int day = 0;
    int month = 0;
    int year = 0;
    R_USERTYPE_DECL(Date);
};

R_USERTYPE_DEF(Date, dt, dt.day << '/' << dt.month << '/' << dt.year);

TEST_F(SinkTest, usertype) {
    Date d{17, 9, 88};
    EXPECT_CALL(m_mocksink, level(R::Level::Error));
    EXPECT_CALL(m_mocksink, filename("test_sink.cpp"));
    EXPECT_CALL(m_mocksink, line(__LINE__ + 4));
    EXPECT_CALL(m_mocksink, timestamp(_));
    EXPECT_CALL(m_mocksink, tag("Whatever"));
    EXPECT_CALL(m_mocksink, message("17/9/88\n is the date\n"));
    R_ERROR("Whatever") << d << endl << " is the date" << endl;
}

// -------------------------------------------------------------------

TEST_F(SinkTest, multisink) {
    R::addSink(R_SINK_W_CAPTURE(m, s, this) { m_mocksink.message(s); });
    EXPECT_CALL(m_mocksink, level(R::Level::Info));
    EXPECT_CALL(m_mocksink, filename("test_sink.cpp"));
    EXPECT_CALL(m_mocksink, line(__LINE__ + 4));
    EXPECT_CALL(m_mocksink, timestamp(_));
    EXPECT_CALL(m_mocksink, tag("SinkTest"));
    EXPECT_CALL(m_mocksink, message("X14.5")).Times(2);
    R_INFO("SinkTest") << "X" << 1 << 4.5;
}

// -------------------------------------------------------------------

}  // namespace

// -------------------------------------------------------------------
