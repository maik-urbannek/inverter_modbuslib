#pragma once

// model
#include <catta/modbus/si/WriteRegister.hpp>

// tomodbus
#include <catta/tomodbus/toModbus.hpp>

namespace catta
{
namespace tomodbus
{
template <>
class Serializer<catta::modbus::si::WriteRegister>
{
  public:
    using Error = catta::state::DefaultError;
    using Input = catta::modbus::si::WriteRegister;
    using Output = catta::modbus::Token;
    [[nodiscard]] constexpr std::tuple<Error, catta::parser::InputHandled> read(const Input& input) noexcept
    {
        using Tuple = std::tuple<Error, catta::parser::InputHandled>;
        const auto error = [this]()
        {
            _state = ERROR_STATE;
            return Tuple{Error::error(), catta::parser::InputHandled::yes()};
        };
        const auto done = [this]()
        {
            _data = Output::end();
            _state = DONE + 0;
            return Tuple{Error(), catta::parser::InputHandled::yes()};
        };
        const auto stay = [this](const Output token)
        {
            _data = token;
            return Tuple{Error(), catta::parser::InputHandled::no()};
        };
        const auto next = [stay, this](const Output token)
        {
            _state++;
            return stay(token);
        };
        const auto high = [next](const auto v) { return next(Output::data(static_cast<std::uint8_t>(v >> 8))); };
        const auto low = [next](const auto v) { return next(Output::data(static_cast<std::uint8_t>(v >> 0))); };
        switch (_state)
        {
            case START + 0:
                return next(Output::start());
            case ADDRESS + 0:
                return next(Output::function(0x10));
            case ADDRESS + 1:
                return high(input.registerAddress().raw());
            case ADDRESS + 2:
                return low(input.registerAddress().raw());
            case DATA + 0:
                return next(Output::data(0x00));
            case DATA + 1:
                return next(Output::data(0x01));
            case DATA + 2:
                return next(Output::data(0x02));
            case DATA + 3:
                return high(input.raw());
            case DATA + 4:
                return low(input.raw());
            case TAIL + 0:
                return done();
            default:
                return error();
        }
    }
    [[nodiscard]] constexpr Serializer() noexcept : _state(START), _data(Output::empty()) {}
    [[nodiscard]] constexpr Output data() const noexcept { return _data; }
    [[nodiscard]] constexpr catta::parser::State state() const noexcept
    {
        if (_state == START) return catta::parser::State::start();
        if (_state < DONE) return catta::parser::State::running();
        if (_state == DONE) return catta::parser::State::done();
        return catta::parser::State::failed();
    }

  private:
    std::uint8_t _state;
    Output _data;
    static constexpr std::uint8_t START = 0;
    static constexpr std::uint8_t ADDRESS = START + 1;
    static constexpr std::uint8_t DATA = ADDRESS + 3;
    static constexpr std::uint8_t TAIL = DATA + 5;
    static constexpr std::uint8_t DONE = TAIL + 1;
    static constexpr std::uint8_t ERROR_STATE = DONE + 1;
};
}  // namespace tomodbus
}  // namespace catta
