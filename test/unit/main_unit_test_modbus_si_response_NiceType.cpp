// catta
#include <catta/modbus/si/response/NiceType.hpp>

// random
#include <catta/random/modbus/si/response/NiceType.hpp>

// fromtostring
#include <catta/fromstring/modbus/si/response/NiceType.hpp>
#include <catta/tostring/modbus/si/response/NiceType.hpp>

// json
#include <catta/fromjson/modbus/si/response/NiceResponse.hpp>
#include <catta/tojson/modbus/si/response/NiceResponse.hpp>

// test
#include <catta/test/ConsoleOutput.hpp>
#include <catta/test/CopyMove.hpp>
#include <catta/test/Test.hpp>
#include <catta/test/ToFromJson.hpp>
#include <catta/test/ToFromString.hpp>

static constexpr bool debug = false;

int main()
{
    using Type = catta::modbus::si::response::NiceType;
    using Output = catta::test::ConsoleOutput;
    const auto test_tofromstring = catta::test::checkToFromString<Output, Type, debug>;
    const auto test_copymove = catta::test::checkCopyMove<Output, Type, catta::test::NO_CHECK>;
    const auto test_tofromjson = catta::test::checkToFromJson<Output, Type, debug>;
    Output output(catta::tostring::GetName<Type>::name.data());
    int error = 0;
    if (!catta::test::execute<Output>(test_tofromstring, "tofromstring", output, 10000)) error++;
    if (!catta::test::execute<Output>(test_copymove, "copymove", output, 10000)) error++;
    if (!catta::test::execute<Output>(test_tofromjson, "tofromjson", output, 10000)) error++;
    return error;
}
