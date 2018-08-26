#include "rlog.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <mutex>
#include <string>
#include <thread>

// -------------------------------------------------------------------

namespace {

// -------------------------------------------------------------------

using namespace std;
using namespace testing;

// -------------------------------------------------------------------

void number_guesser() {
    R_INFO("main") << "Booting up";

    srand(time(NULL));

    const auto secret = rand() % 10 + 1;

    volatile auto stop = false;

    std::mutex mut;
    volatile auto finished = false;

    auto Solver = [&](const string& name) {
        return [&, name] {
            R_INFO(name) << "Solver " << name << " starting";

            do {
                R_INFO(name) << "Solver " << name << " running";

                this_thread::sleep_for(chrono::milliseconds(10));

                if (stop) {
                    R_ERROR(name) << "Solver " << name << " failed";
                    break;
                }

                auto guess = rand() % 10 + 1;

                if (guess == secret) {
                    R_WARNING(name) << "Solver " << name << " won";
                    lock_guard<mutex> lock(mut);
                    finished = true;
                    break;
                }

            } while (true);
        };
    };

    std::thread foo(Solver("Foo"));
    std::thread bar(Solver("Bar"));

    do {
        if (finished) {
            stop = true;
            break;
        }
    } while (true);

    foo.join();
    bar.join();

    R_INFO("main") << "Shutting down";
}

const auto run_sample = [](R::Level level) {
    ofstream fs;
    fs.open(std::string("outputs/sampleusage_") + R::to_string(level) +
            ".json");

    R::reset(level);

    R::addSink(R::makeSmartFormattedCoutSink());

    R::JsonSink json(fs);
    R::addSink(std::ref(json));

    number_guesser();

    R::reset();
};

// -------------------------------------------------------------------

TEST(SampleUsageTest, Info) { run_sample(R::Level::Info); }

// -------------------------------------------------------------------

TEST(SampleUsageTest, Warning) { run_sample(R::Level::Warning); }

// -------------------------------------------------------------------

TEST(SampleUsageTest, Error) { run_sample(R::Level::Error); }

// -------------------------------------------------------------------

}  // namespace

// -------------------------------------------------------------------
