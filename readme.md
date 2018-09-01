# RLog

* osx clang build [![Build Status](https://travis-ci.org/rishikhaneja/rlog.svg?branch=master)](https://travis-ci.org/rishikhaneja/rlog)

* windows msvc 2017 build [![Build status](https://ci.appveyor.com/api/projects/status/snk32n500mbiknw5/branch/master?svg=true)](https://ci.appveyor.com/project/rishikhaneja/rlog)

## Overview

* A header-only, zero-external-dependency logger
* Highly customisable
* Modular and composable
* Readable and concise interface
* Thread-safe
* Easy to switch from existing "cout logging"
* Severity levels
* Timestamp support
* Built in Smart Formatter
* Built in Json writter

## Integration

* Copy headers `rlog.hpp` and `rlog_config.hpp` into your includes folder
* And include `rlog.hpp` into your source
* You are good to go!

## Building tests

* `build_and_test.sh`

## Design & Usage

### Levels

* Severity of each log can be one of three levels, lowest to highest

```c++
R::Level::Info
R::Level::Warning
R::Level::Error
```

### Initialisation

* Reset sets global level to specified value and clears any previous global state
* Can be used for both init and deinit
* Best called atleast once from a single-threaded init context

```c++
R::reset(); // defaults to Info
R::reset(R::Level::Warning);
```

### Logging

* A log is made using one of three macros per level
* Any custom string `message` can be provided using stream syntax
* Argument to the macro is a `tag` string, which is stored on its `metadata` and can be used for formatting and filtering

```c++
R_INFO("foo") << "starting up " << module;
```

```c++
R_WARNING("bar") << "detected problem in object " << o.name;
```

```c++
R_ERROR("") << "failed to load";
```

### Metadata

* Type `R:Metadata` automatically stores `level`, `filename`, `line`, `timestamp` and `tag` per log
  
```c++
Level level;
std::string filename;
long line;
std::string timestamp;
std::string tag;
```

### Sink

* Type `R::Sink` captures objects or functions that output given their metadata and message

* Being a `std::function`, it can capture any callable with signature `void(const R::Metadata&, const std::string&)`
  
```c++
auto cout_sink = [](const R::Metadata& metadata,
                    const std::string& message)
{
    std::cout << message;
}
```

* In-built macros help make this more readable and concise

```c++
// lambda without capture
auto fooSink = R_SINK(metadata, message) { std::cout << message; }

// lambda with capture
auto barSink = R_SINK_W_CAPTURE(metadata, message, x) { std::cout << message << x; }

// class with () operator
struct FooBarSink {
    R_SINK_OPERATOR(metadata, message) {
        std::cout << message;
    }
};
```

* Sinks must be passed to RLog to activate them, as copyable or as reference via std::ref

```c++
// copyable sink can be passed even as a temporary, since its anyway copied
R::addSink(R_SINK(metadata, message) { std::cout << message; });

// non-copyable sink must be passed as via std::ref
struct XSink {
    XSink() {...};
    XSink(const XSink&) = delete; // non-copyable
    R_SINK_OPERATOR(metadata, message) {...}
} xsink;
R::addSink(std::ref(xsink));
```

### Filter

* Type `R::Filter` captures objects or functions that return a `boolean` given a metadata and a message, and can be used by sinks for making per-log decisions

* Being a `std::function`, it can capture any callable with signature `bool(const R::Metadata&, const std::string&)`

```c++
auto xyzFilter = [](const R::Metadata& metadata,
                    const std::string& message)
{
    // returns true if tag is "XYZ"
    return metadata.tag == "XYZ";
}
```

* In-built macros help make this more readable and concise

```c++
// lambda without capture
auto fooFilter = R_FILTER(metadata, message) { return metadata.tag == "XYZ"; };

// lambda with capture
auto barFilter = R_FILTER_W_CAPTURE(metadata, message, x) { return metadata.tag == "XYZ"; }
```

* Must be composed with a sink to use

```c++
R::addSink(
    R_SINK(metadata, message) {
        if (fooFilter(metadata, message)) {
            fooSink(metadata, message);
        }
    }
);
```

* In-built utility helps ease this composition

```c++
R::addSink(makeFilteredSink(fooSink, fooFilter));
```

### Formatter

* Type `R::Formatter` captures objects or functions that return a `std::string` given a metadata and a message, and can be used by sinks for formatting the log

* Being a `std::function`, it can capture any callable with signature `std::string(const R::Metadata&, const std::string&)`

```c++
auto xyzFormatter = [](const R::Metadata& metadata,
                    const std::string& message)
{
    return R::to_string(metadata.level) 
           << to_upper(message) ;
}
```

* In-built macros help make this more readable and concise

```c++
// lambda without capture
auto fooFormatter = R_FORMATTER(metadata, message) { return to_upper(message); };

// lambda with capture
auto barFormatter = R_FORMATTER_W_CAPTURE(metadata, message, x) { return to_upper(message) << x; }
```

* Must be composed with a sink to use, manually or with in-built utlity

```c++
R::addSink(makeFormattedSink(fooSink, fooFormatter));
```

### Composition

* All these objects can be composed together in new ways, allowing for
    * powerful combined effects
    * re-use
    * unlimited extensibility

### Smart Formatter

* In-built intelligent formatter
* Using a string format, allows puting together metadata values and message in custom fashion
* Default format is `"[R] #timestamp [#level] #tag (#filename:#line) #message"`

```c++
auto fooFormatter = makeSmartFormatter("#filename : #line : #message");
```

### Cout Sink

* In-built basic `std::cout` sink
* Pushes every log to cout, along with a `std::endl`, thus flushing immediately

```c++
R::addSink(R::CoutSink);
// log on
```

### Smart Formatted Cout Sink

* In-built composition of `CoutSink` and `SmartFormatter`

```c++
R::addSink(R::makeSmartFormattedCoutSink(format));
// log on
```

### Json Sink

* In-built formatted `json` file sink
* Pushes every log to a file, in proper json format

```c++
ofstream fs;
fs.open("foo.json");
R::JsonSink json(fs);
R::addSink(std::ref(json));
// log on
```

### Compile-time configurations

* Header `rlog_config.hpp` has two compile time configurations
* `R_ACTIVE` : Allows completely disabling all logging, when set to false
* `R_MIN_LEVEL`: Allows setting filtering all logs globally, such that any log below specified level shall be completely disabled

## Other Strengths

* Runtime level switch-ability support means no need of recompilation
* When a log is filtered even at runtime, all costs except the comparision are removed!

## Limitations / Weaknesses

* Sinks and level are stored on globals
* Dynamic memory used by std::function

## Further development

* Level-based effects like warnings cause debug-break and errors cause assertion
* Conditional logging support
* More in-built output options
* More tests
* Test coverage setup