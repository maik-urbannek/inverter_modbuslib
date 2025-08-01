#pragma once

// gui
#include <catta/gui/Curve.hpp>
#include <catta/gui/WriteSingle.hpp>

// fltk
#include <FL/Fl_Box.H>
#include <FL/Fl_Group.H>

// std
#include <array>
#include <string>

namespace catta
{
namespace gui
{
/**
 * @brief Wind shows wind config.
 * @author CattaTech - Maik Urbannek
 */
class Wind : public Fl_Group
{
  public:
    /**
     * @param[in] X The x coordinate of the widget.
     * @param[in] Y The y coordinate of the widget.
     * @param[in] W The width of the widget.
     * @param[in] H The height of the widget.
     * Constructor.
     */
    Wind(const int X, const int Y, const int W, const int H) : Fl_Group(X, Y, W, H, nullptr)
    {
        static constexpr std::array<const char*, ROBIN_SIZE> LABEL = {"U_DC_START",    "U_DC_HIGH",       "Z_PT1WINDUSOL", "P_MAX",
                                                                      "X 00…07 Power", "Y 00…07 Voltage", "X 08…15 Power", "Y 08…15 Voltage"};
        static constexpr int GAP = 5;
        static constexpr int H_LINE = 65;
        static const int PER_LINE = 4;
        const int W_WRITE = (W - (PER_LINE + 1) * GAP) / PER_LINE;
        static const int W_SEND = 30;
        using Address = catta::modbus::si::RegisterAddress;
        static constexpr auto dcStartReadAddress = Address::siControlVoltageDcStartRead();
        static constexpr auto dcStartWriteAddress = Address::siControlVoltageDcStartWrite();
        static constexpr auto dcHighReadAddress = Address::siControlDcHighRead();
        static constexpr auto dcHighWriteAddress = Address::siControlDcHighWrite();
        static constexpr auto zPt1WindUsolAddress = Address::siControlFilterWindCurve();
        static constexpr auto pMaxAddress = Address::basicSettingsMaxPower();

        static constexpr std::array<std::array<Address, 8>, CURVES> curveAddressRead = {
            std::array<Address, 8>{Address::siControlWindCurveX00(), Address::siControlWindCurveX01(), Address::siControlWindCurveX02(),
                                   Address::siControlWindCurveX03(), Address::siControlWindCurveX04(), Address::siControlWindCurveX05(),
                                   Address::siControlWindCurveX06(), Address::siControlWindCurveX07()},
            std::array<Address, 8>{Address::siControlWindCurveY00Read(), Address::siControlWindCurveY01Read(), Address::siControlWindCurveY02Read(),
                                   Address::siControlWindCurveY03Read(), Address::siControlWindCurveY04Read(), Address::siControlWindCurveY05Read(),
                                   Address::siControlWindCurveY06Read(), Address::siControlWindCurveY07Read()},
            std::array<Address, 8>{Address::siControlWindCurveX08(), Address::siControlWindCurveX09(), Address::siControlWindCurveX10(),
                                   Address::siControlWindCurveX11(), Address::siControlWindCurveX12(), Address::siControlWindCurveX13(),
                                   Address::siControlWindCurveX14(), Address::siControlWindCurveX15()},
            std::array<Address, 8>{Address::siControlWindCurveY08Read(), Address::siControlWindCurveY09Read(), Address::siControlWindCurveY10Read(),
                                   Address::siControlWindCurveY11Read(), Address::siControlWindCurveY12Read(), Address::siControlWindCurveY13Read(),
                                   Address::siControlWindCurveY14Read(), Address::siControlWindCurveY15Read()}};

        static constexpr std::array<std::array<Address, 8>, CURVES> curveAddressWrite = {
            std::array<Address, 8>{Address::siControlWindCurveX00(), Address::siControlWindCurveX01(), Address::siControlWindCurveX02(),
                                   Address::siControlWindCurveX03(), Address::siControlWindCurveX04(), Address::siControlWindCurveX05(),
                                   Address::siControlWindCurveX06(), Address::siControlWindCurveX07()},
            std::array<Address, 8>{Address::siControlWindCurveY00Write(), Address::siControlWindCurveY01Write(),
                                   Address::siControlWindCurveY02Write(), Address::siControlWindCurveY03Write(),
                                   Address::siControlWindCurveY04Write(), Address::siControlWindCurveY05Write(),
                                   Address::siControlWindCurveY06Write(), Address::siControlWindCurveY07Write()},
            std::array<Address, 8>{Address::siControlWindCurveX08(), Address::siControlWindCurveX09(), Address::siControlWindCurveX10(),
                                   Address::siControlWindCurveX11(), Address::siControlWindCurveX12(), Address::siControlWindCurveX13(),
                                   Address::siControlWindCurveX14(), Address::siControlWindCurveX15()},
            std::array<Address, 8>{Address::siControlWindCurveY08Write(), Address::siControlWindCurveY09Write(),
                                   Address::siControlWindCurveY10Write(), Address::siControlWindCurveY11Write(),
                                   Address::siControlWindCurveY12Write(), Address::siControlWindCurveY13Write(),
                                   Address::siControlWindCurveY14Write(), Address::siControlWindCurveY15Write()}};

        static constexpr std::array<const char*, CURVES> curveUnit = {"V", "W", "V", "W"};

        static std::array<std::tuple<std::size_t, Wind*>, ROBIN_SIZE> cbArray;
        for (std::size_t i = 0; i < SIZE; i++)
        {
            cbArray[i] = std::tuple<std::size_t, Wind*>{i, this};
        }

        _write[U_DC_START] = new WriteSingle(X + GAP + (int(U_DC_START) % PER_LINE) * (GAP + W_WRITE), Y + H_LINE * (int(U_DC_START) / PER_LINE),
                                             W_WRITE, H_LINE, FL_FLOAT_INPUT, 0.01, 655.00, 0.01, dcStartReadAddress, dcStartWriteAddress,
                                             _toRegisterCenti, _fromRegisterCenti, W_SEND, "V", LABEL[U_DC_START]);

        _write[U_DC_HIGH] = new WriteSingle(X + GAP + (int(U_DC_HIGH) % PER_LINE) * (GAP + W_WRITE), Y + H_LINE * (int(U_DC_HIGH) / PER_LINE),
                                            W_WRITE, H_LINE, FL_FLOAT_INPUT, 0.01, 655.00, 0.01, dcHighReadAddress, dcHighWriteAddress,
                                            _toRegisterCenti, _fromRegisterCenti, W_SEND, "V", LABEL[U_DC_HIGH]);

        _write[Z_PT1WINDUSOL] =
            new WriteSingle(X + GAP + (int(Z_PT1WINDUSOL) % PER_LINE) * (GAP + W_WRITE), Y + H_LINE * (int(Z_PT1WINDUSOL) / PER_LINE), W_WRITE,
                            H_LINE, FL_FLOAT_INPUT, 0.1, 25.5, 0.1, zPt1WindUsolAddress, zPt1WindUsolAddress, _toRegisterDeci, _fromRegisterDeci,
                            W_SEND, "s", LABEL[Z_PT1WINDUSOL]);

        _write[P_MAX] = new WriteSingle(X + GAP + (int(P_MAX) % PER_LINE) * (GAP + W_WRITE), Y + H_LINE * (int(P_MAX) / PER_LINE), W_WRITE, H_LINE,
                                        FL_FLOAT_INPUT, 0.1, 6550.0, 0.1, pMaxAddress, pMaxAddress, _toRegisterDeci, _fromRegisterDeci, W_SEND, "W",
                                        LABEL[P_MAX]);

        _windLabel = new Fl_Box(X, Y + H_LINE * int(SIZE / PER_LINE + 1) + 20, W, 45, "WIND_CURVE");
        for (std::size_t i = 0; i < CURVES; i++)
        {
            cbArray[i + SIZE] = std::tuple<std::size_t, Wind*>{i + SIZE, this};
            const int y = Y + H_LINE * (int(i + SIZE / PER_LINE) + 2);
            const int W_CURVE = W - 2 * GAP;
            _curve[i] = new Curve<8>(X + GAP, y, W_CURVE, H_LINE, FL_INT_INPUT, 0.0, 256.0, 1.0, curveAddressRead[i], curveAddressWrite[i],
                                     _toRegister256, _fromRegister256, 30, curveUnit[i], LABEL[SIZE + i]);
        }
        this->end();
    }
    /**
     * Destructor.
     */
    ~Wind() {}
    /**
     * @param canTakeRequest Wether there is space to send request.
     * @param response The received response.
     * @param request The corresponding request to received response.
     * @param[in] now The current time.
     * @return Returns the request to send.
     */
    catta::modbus::si::request::Request work(const bool canTakeRequest, const catta::modbus::si::response::Response& response,
                                             const catta::modbus::si::request::Request& request, const std::chrono::microseconds now)
    {
        catta::modbus::si::request::Request send;
        std::size_t newRoundRobin = _roundRobin;
        for (std::size_t i = 0; i < ROBIN_SIZE; i++)
        {
            std::size_t j = (_roundRobin + i) % ROBIN_SIZE;
            const catta::modbus::si::request::Request r = j < SIZE ? _write[j]->work(canTakeRequest && send.isEmpty(), response, request, now)
                                                                   : _curve[j - SIZE]->work(canTakeRequest && send.isEmpty(), response, request, now);
            if (!r.isEmpty())
            {
                send = r;
                newRoundRobin = (j + 1) % ROBIN_SIZE;
            }
        }
        _roundRobin = newRoundRobin;
        return send;
    }
    /**
     * Better hide.
     */
    void stop()
    {
        for (std::size_t i = 0; i < _write.size(); i++) _write[i]->stop();
        for (std::size_t i = 0; i < _curve.size(); i++) _curve[i]->stop();
    }

  private:
    static constexpr std::size_t U_DC_START = 0;
    static constexpr std::size_t U_DC_HIGH = U_DC_START + 1;
    static constexpr std::size_t Z_PT1WINDUSOL = U_DC_HIGH + 1;
    static constexpr std::size_t P_MAX = Z_PT1WINDUSOL + 1;
    static constexpr std::size_t SIZE = P_MAX + 1;

    static constexpr std::size_t X_PAC_0 = 0;
    static constexpr std::size_t Y_U_SOL_0 = X_PAC_0 + 1;
    static constexpr std::size_t X_PAC_1 = Y_U_SOL_0 + 1;
    static constexpr std::size_t Y_U_SOL_1 = X_PAC_1 + 1;
    static constexpr std::size_t CURVES = Y_U_SOL_1 + 1;

    static constexpr std::size_t ROBIN_SIZE = SIZE + CURVES;

    std::array<WriteSingle*, SIZE> _write;
    Fl_Box* _windLabel;
    std::array<Curve<8>*, CURVES> _curve;
    std::size_t _roundRobin;
    class ToRegisterCenti
    {
      public:
        std::uint16_t operator()(const double input) { return static_cast<std::uint16_t>(input * 100.0); }

      private:
    } _toRegisterCenti;
    class FromRegisterCenti
    {
      public:
        double operator()(const std::uint16_t input) { return static_cast<double>(input) / 100.0; }

      private:
    } _fromRegisterCenti;

    class ToRegisterDeci
    {
      public:
        std::uint16_t operator()(const double input) { return static_cast<std::uint16_t>(input * 10.0); }

      private:
    } _toRegisterDeci;
    class FromRegisterDeci
    {
      public:
        double operator()(const std::uint16_t input) { return static_cast<double>(input) / 10.0; }

      private:
    } _fromRegisterDeci;

    class ToRegister256
    {
      public:
        std::uint16_t operator()(const double input) { return static_cast<std::uint16_t>(input); }
    } _toRegister256;
    class FromRegister256
    {
      public:
        double operator()(const std::uint16_t input) { return static_cast<double>(input); }
    } _fromRegister256;
};
}  // namespace gui
}  // namespace catta
