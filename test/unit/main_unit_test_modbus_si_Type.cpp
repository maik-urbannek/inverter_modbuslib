// catta
#include <catta/modbus/si/Type.hpp>

// random
#include <catta/random/modbus/si/Type.hpp>

// fromtostring
#include <catta/fromstring/modbus/si/Type.hpp>
#include <catta/tostring/modbus/si/Type.hpp>

// json
#include <catta/fromjson/modbus/si/Type.hpp>
#include <catta/tojson/modbus/si/Type.hpp>

// test
#include <catta/test/ConsoleOutput.hpp>
#include <catta/test/CopyMove.hpp>
#include <catta/test/Test.hpp>
#include <catta/test/ToFromJson.hpp>
#include <catta/test/ToFromString.hpp>

static constexpr bool debug = false;

int main()
{
    using Type = catta::modbus::si::Type;
    using Output = catta::test::ConsoleOutput;
    Output output(catta::tostring::GetName<Type>::name.data());
    const auto test_tofromstring = catta::test::checkToFromString<Output, Type, debug>;
    const auto test_copymove = catta::test::checkCopyMove<Output, Type, catta::test::NO_CHECK>;
    const auto test_tofromjson = catta::test::checkToFromJson<Output, Type, debug>;
    const auto test_tofromjsonempty = catta::test::checkToFromJsonEmpty<Output, Type, debug>;
    int error = 0;
    if (!catta::test::execute<Output>(test_tofromstring, "tofromstring", output, 10000)) error++;
    if (!catta::test::execute<Output>(test_copymove, "copymove", output, 10000)) error++;
    if (!catta::test::execute<Output>(test_tofromjson, "tofromjson", output, 10000)) error++;
    if (!catta::test::execute<Output>(test_tofromjsonempty, "tofromjsonempty", output)) error++;
    return error;
}
