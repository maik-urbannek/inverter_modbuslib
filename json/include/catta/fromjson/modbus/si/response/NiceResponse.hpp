#pragma once

// request
#include <catta/modbus/si/response/NiceResponse.hpp>

// fromjson
#include <catta/fromjson/fromJson.hpp>
#include <catta/fromjson/modbus/si/RegisterAddress.hpp>
#include <catta/fromjson/modbus/si/RegisterValue.hpp>
#include <catta/fromjson/modbus/si/response/Exception.hpp>
#include <catta/fromjson/modbus/si/response/FactoryValues.hpp>
#include <catta/fromjson/modbus/si/response/NiceType.hpp>
#include <catta/fromjson/modbus/si/response/ReadError.hpp>
#include <catta/fromjson/modbus/si/response/ReadOperatingData33.hpp>
#include <catta/fromjson/modbus/si/response/ReadOperatingData3e.hpp>

namespace catta
{
namespace fromjson
{
template <>
class Parser<catta::modbus::si::response::NiceResponse>
{
  public:
    using Error = catta::state::DefaultError;
    using Input = catta::json::Token;
    using Output = catta::modbus::si::response::NiceResponse;
    [[nodiscard]] constexpr std::tuple<Error, catta::parser::InputHandled> read(const Input& input) noexcept
    {
        using Tuple = std::tuple<Error, catta::parser::InputHandled>;
        const auto error = [this]()
        {
            _state = ERROR_STATE;
            return Tuple{Error::error(), catta::parser::InputHandled::yes()};
        };
        const auto stay = []() { return Tuple{Error(), catta::parser::InputHandled::yes()}; };
        const auto jump = [stay, this](const std::uint8_t state)
        {
            _state = state;
            return stay();
        };
        const auto jumpAndSetBool = [jump, this](const bool value, const std::uint8_t state)
        {
            _valueSuccess = value;
            return jump(state);
        };
        const auto next = [stay, this]()
        {
            _state++;
            return stay();
        };
        const auto isInEndState = [](auto parser)
        {
            [[maybe_unused]] auto ignore = parser.read(catta::json::Token::end());
            return parser.state().isDone();
        };
        const auto handle = [stay, jump, error, input, isInEndState](auto& parser)
        {
            const auto close = catta::json::Token::closeCurlyBracket();
            const auto again = catta::json::Token::comma();
            const auto token = (input == close || input == again) && isInEndState(parser) ? catta::json::Token::end() : input;
            [[maybe_unused]] auto ignore = parser.read(token);
            if (parser.state().isFailed()) return error();
            if (parser.state().isDone()) return jump(input == close ? TAIL + 0 : HUB + 0);
            return stay();
        };
        const auto valueSet = [this, input, jump, error]()
        {
            _valueSet = true;
            switch (input.type())
            {
                case catta::json::TokenType::closeCurlyBracket():
                    return jump(TAIL + 0);
                case catta::json::TokenType::comma():
                    return jump(HUB + 0);
                default:
                    return error();
            }
        };
        const auto handleSplitt = [this, isInEndState, input, valueSet, jump, error]()
        {
            bool possible = false;
            bool done = false;
            const auto h = [&possible, &done, isInEndState, input](auto& parser)
            {
                if (parser.state().isFailed()) return;
                const auto close = catta::json::Token::closeCurlyBracket();
                const auto again = catta::json::Token::comma();
                const auto token = (input == close || input == again) && isInEndState(parser) ? catta::json::Token::end() : input;
                [[maybe_unused]] auto ignore = parser.read(token);
                if (!parser.state().isFailed()) possible = true;
                if (parser.state().isDone()) done = true;
            };
            h(_writeRegisterParser);
            h(_exceptionParser);
            h(_factoryValuesParser);
            h(_readErrorParser);
            h(_readOperatingData33Parser);
            h(_readOperatingData3eParser);
            h(_registerValueParser);
            if (done) return valueSet();
            return possible ? jump(HUB + 15) : error();
        };
        switch (_state)
        {
            case START + 0:
                return input == catta::json::Token::openCurlyBracket() ? next() : error();
            case HUB + 0:
                return input == catta::json::Token::openString() ? next() : error();
            case HUB + 1:
                return input == catta::json::Token::character('t')   ? jump(HUB + 2)
                       : input == catta::json::Token::character('v') ? jump(HUB + 8)
                                                                     : error();
            case HUB + 2:
                return input == catta::json::Token::character('y') ? jump(HUB + 3) : error();
            case HUB + 3:
                return input == catta::json::Token::character('p') ? jump(HUB + 4) : error();
            case HUB + 4:
                return input == catta::json::Token::character('e') ? jump(HUB + 5) : error();
            case HUB + 5:
                return input == catta::json::Token::closeString() ? jump(HUB + 6) : error();
            case HUB + 6:
                return !_typeParser.state().isStart() ? error() : input == catta::json::Token::colon() ? next() : error();
            case HUB + 7:
                return handle(_typeParser);
            case HUB + 8:
                return input == catta::json::Token::character('a') ? jump(HUB + 9) : error();
            case HUB + 9:
                return input == catta::json::Token::character('l') ? jump(HUB + 10) : error();
            case HUB + 10:
                return input == catta::json::Token::character('u') ? jump(HUB + 11) : error();
            case HUB + 11:
                return input == catta::json::Token::character('e') ? jump(HUB + 12) : error();
            case HUB + 12:
                return input == catta::json::Token::closeString() ? jump(HUB + 13) : error();
            case HUB + 13:
                return _valueSet ? error() : input == catta::json::Token::colon() ? next() : error();
            case HUB + 14:
                return input == catta::json::Token::boolTrue()    ? jumpAndSetBool(true, HUB + 16)
                       : input == catta::json::Token::boolFalse() ? jumpAndSetBool(false, HUB + 16)
                                                                  : handleSplitt();
            case HUB + 15:
                return handleSplitt();
            case HUB + 16:
                return valueSet();
            case TAIL + 0:
                return input == catta::json::Token::end() ? jump(DONE + 0) : error();
            default:
                return error();
        }
    }
    [[nodiscard]] constexpr Parser() noexcept : _state(START), _valueSet(false), _valueSuccess(false) {}
    [[nodiscard]] constexpr Output data() const noexcept
    {
        if (_state != DONE || !_valueSet) return Output::empty();
        using Type = catta::modbus::si::response::NiceType;
        switch (_typeParser.data())
        {
            case Type::exception():
                return catta::modbus::si::response::NiceResponse::exception(_exceptionParser.data());
            case Type::factoryValues():
                return catta::modbus::si::response::NiceResponse::factoryValues(_factoryValuesParser.data());
            case Type::readError():
                return catta::modbus::si::response::NiceResponse::readError(_readErrorParser.data());
            case Type::readOperatingData33():
                return catta::modbus::si::response::NiceResponse::readOperatingData33(_readOperatingData33Parser.data());
            case Type::readOperatingData3e():
                return catta::modbus::si::response::NiceResponse::readOperatingData3e(_readOperatingData3eParser.data());
            case Type::switchOffGridRelay():
                return catta::modbus::si::response::NiceResponse::switchOffGridRelay(_valueSuccess);
            case Type::switchOnGridRelay():
                return catta::modbus::si::response::NiceResponse::switchOnGridRelay(_valueSuccess);
            case Type::forceIdle():
                return catta::modbus::si::response::NiceResponse::forceIdle(_valueSuccess);
            case Type::deactivateIdle():
                return catta::modbus::si::response::NiceResponse::deactivateIdle(_valueSuccess);
            case Type::startConstantVoltage():
                return catta::modbus::si::response::NiceResponse::startConstantVoltage(_valueSuccess);
            case Type::endConstantVoltage():
                return catta::modbus::si::response::NiceResponse::endConstantVoltage(_valueSuccess);
            case Type::setPowerFactor():
                return catta::modbus::si::response::NiceResponse::setPowerFactor(_valueSuccess);
            case Type::controlBatteryInvert():
                return catta::modbus::si::response::NiceResponse::controlBatteryInvert(_valueSuccess);
            case Type::limitBatteryInvert():
                return catta::modbus::si::response::NiceResponse::limitBatteryInvert(_valueSuccess);
            case Type::writeRegister():
                return catta::modbus::si::response::NiceResponse::writeRegister(_writeRegisterParser.data());
            case Type::readRegister():
                return catta::modbus::si::response::NiceResponse::readRegister(_registerValueParser.data());
            default:
                return catta::modbus::si::response::NiceResponse::empty();
        }
    }
    [[nodiscard]] constexpr catta::parser::State state() const noexcept
    {
        if (_state == START) return catta::parser::State::start();
        if (_state < DONE) return catta::parser::State::running();
        if (_state == DONE) return catta::parser::State::done();
        return catta::parser::State::failed();
    }

  private:
    std::uint8_t _state;
    bool _valueSet;
    bool _valueSuccess;
    Parser<catta::modbus::si::response::NiceType> _typeParser;
    Parser<catta::modbus::si::response::Exception> _exceptionParser;
    Parser<catta::modbus::si::response::FactoryValues> _factoryValuesParser;
    Parser<catta::modbus::si::response::ReadError> _readErrorParser;
    Parser<catta::modbus::si::response::ReadOperatingData33> _readOperatingData33Parser;
    Parser<catta::modbus::si::response::ReadOperatingData3e> _readOperatingData3eParser;
    Parser<catta::modbus::si::RegisterAddress> _writeRegisterParser;
    Parser<catta::modbus::si::RegisterValue> _registerValueParser;
    static constexpr std::uint8_t START = 0;
    static constexpr std::uint8_t HUB = START + 1;
    static constexpr std::uint8_t TAIL = HUB + 17;
    static constexpr std::uint8_t DONE = TAIL + 1;
    static constexpr std::uint8_t ERROR_STATE = DONE + 1;
};
}  // namespace fromjson
}  // namespace catta
