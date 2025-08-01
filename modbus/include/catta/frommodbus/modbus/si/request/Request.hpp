#pragma once

// request
#include <catta/modbus/si/request/Request.hpp>

// frommodbus
#include <catta/frommodbus/fromModbus.hpp>
#include <catta/frommodbus/modbus/si/ReadRegister.hpp>
#include <catta/frommodbus/modbus/si/WriteRegister.hpp>
#include <catta/frommodbus/modbus/si/request/ConstantVoltage.hpp>
#include <catta/frommodbus/modbus/si/request/ControlBatteryInvert.hpp>
#include <catta/frommodbus/modbus/si/request/LimitBatteryInvert.hpp>
#include <catta/frommodbus/modbus/si/request/PowerFactor.hpp>

namespace catta
{
namespace frommodbus
{
template <>
class Parser<catta::modbus::si::request::Request>
{
  public:
    using Error = catta::state::DefaultError;
    using Input = catta::modbus::Token;
    using Output = catta::modbus::si::request::Request;
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
        const auto next = [stay, this]()
        {
            _state++;
            return stay();
        };

        const auto function = [input, error, this, jump]()
        {
            if (!input.type().isFunction()) return error();
            const auto startParserAndJump = [input, jump](auto& parser, const std::uint8_t state)
            {
                [[maybe_unused]] auto ignore0 = parser.read(Input::start());
                [[maybe_unused]] auto ignore1 = parser.read(input);
                return jump(state);
            };
            _function = input.value();
            switch (_function)
            {
                case 0x03:
                    return startParserAndJump(_readRegisterParser, READ_REGISTER + 0);
                case 0x10:
                    return startParserAndJump(_writeRegisterParser, WRITE_REGISTER + 0);
                case 0x38:
                    return startParserAndJump(_constantVoltageParser, START_CONSTANT_VOLTAGE + 0);
                case 0x3b:
                    return startParserAndJump(_powerFactorParser, SET_POWER_FACTOR + 0);
                case 0x3f:
                    return startParserAndJump(_controlBatteryInvertParser, CONTROL_BATTERY_INVERT + 0);
                case 0x44:
                    return startParserAndJump(_limitBatteryInvertParser, LIMIT_BATTERY_INVERT + 0);
                case 0x31:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x39:
                case 0x3e:
                case 0x40:
                    return jump(SMALL + 0);
                default:
                    return error();
            }
        };

        const auto handle = [stay, jump, error, input](auto& parser)
        {
            [[maybe_unused]] auto ignore = parser.read(input);
            if (parser.state().isFailed()) return error();
            if (parser.state().isDone()) return jump(DONE + 0);
            return stay();
        };
        switch (_state)
        {
            case START + 0:
                return input.type().isStart() ? next() : error();
            case START + 1:
                return function();
            case READ_REGISTER + 0:
                return handle(_readRegisterParser);
            case WRITE_REGISTER + 0:
                return handle(_writeRegisterParser);
            case START_CONSTANT_VOLTAGE + 0:
                return handle(_constantVoltageParser);
            case SET_POWER_FACTOR + 0:
                return handle(_powerFactorParser);
            case CONTROL_BATTERY_INVERT + 0:
                return handle(_controlBatteryInvertParser);
            case LIMIT_BATTERY_INVERT + 0:
                return handle(_limitBatteryInvertParser);
            case SMALL + 0:
                return input == Input::data(0x01) ? next() : error();
            case SMALL + 1:
                return input == Input::data(0x01) ? next() : error();
            case SMALL + 2:
                return input == catta::modbus::Token::end() ? jump(DONE + 0) : error();
            default:
                return error();
        }
    }
    [[nodiscard]] constexpr Parser() noexcept : _state(START), _function(0) {}
    [[nodiscard]] constexpr Output data() const noexcept
    {
        using Type = catta::modbus::si::request::Type;
        if (_state != DONE) return Output::empty();
        switch (_function)
        {
            case Type::readRegister().toModbus():
                return Output::readRegister(_readRegisterParser.data());
            case Type::writeRegister().toModbus():
                return Output::writeRegister(_writeRegisterParser.data());
            case Type::factoryValues().toModbus():
                return Output::factoryValues();
            case Type::readOperatingData33().toModbus():
                return Output::readOperatingData33();
            case Type::switchOffGridRelay().toModbus():
                return Output::switchOffGridRelay();
            case Type::switchOnGridRelay().toModbus():
                return Output::switchOnGridRelay();
            case Type::forceIdle().toModbus():
                return Output::forceIdle();
            case Type::deactivateIdle().toModbus():
                return Output::deactivateIdle();
            case Type::startConstantVoltage().toModbus():
                return Output::startConstantVoltage(_constantVoltageParser.data());
            case Type::endConstantVoltage().toModbus():
                return Output::endConstantVoltage();
            case Type::setPowerFactor().toModbus():
                return Output::setPowerFactor(_powerFactorParser.data());
            case Type::readOperatingData3e().toModbus():
                return Output::readOperatingData3e();
            case Type::controlBatteryInvert().toModbus():
                return Output::controlBatteryInvert(_controlBatteryInvertParser.data());
            case Type::readError().toModbus():
                return Output::readError();
            case Type::limitBatteryInvert().toModbus():
                return Output::limitBatteryInvert(_limitBatteryInvertParser.data());
            default:
                return Output::empty();
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
    std::uint8_t _function;
    Parser<catta::modbus::si::request::PowerFactor> _powerFactorParser;
    Parser<catta::modbus::si::request::LimitBatteryInvert> _limitBatteryInvertParser;
    Parser<catta::modbus::si::request::ConstantVoltage> _constantVoltageParser;
    Parser<catta::modbus::si::request::ControlBatteryInvert> _controlBatteryInvertParser;
    Parser<catta::modbus::si::WriteRegister> _writeRegisterParser;
    Parser<catta::modbus::si::ReadRegister> _readRegisterParser;
    static constexpr std::uint8_t START = 0;
    static constexpr std::uint8_t READ_REGISTER = START + 2;
    static constexpr std::uint8_t WRITE_REGISTER = READ_REGISTER + 1;
    static constexpr std::uint8_t START_CONSTANT_VOLTAGE = WRITE_REGISTER + 1;
    static constexpr std::uint8_t SET_POWER_FACTOR = START_CONSTANT_VOLTAGE + 1;
    static constexpr std::uint8_t CONTROL_BATTERY_INVERT = SET_POWER_FACTOR + 1;
    static constexpr std::uint8_t LIMIT_BATTERY_INVERT = CONTROL_BATTERY_INVERT + 1;
    static constexpr std::uint8_t SMALL = LIMIT_BATTERY_INVERT + 1;
    static constexpr std::uint8_t DONE = SMALL + 3;
    static constexpr std::uint8_t ERROR_STATE = DONE + 1;
};
}  // namespace frommodbus
}  // namespace catta
