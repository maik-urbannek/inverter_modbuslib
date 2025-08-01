#pragma once

// gui
#include <catta/gui/Led.hpp>

// si
#include <catta/modbus/si/request/Request.hpp>
#include <catta/modbus/si/response/Response.hpp>

// fltk
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Spinner.H>

// std
#include <array>
#include <string>

namespace catta
{
namespace gui
{
/**
 * @brief Commands shows commands.
 * @author CattaTech - Maik Urbannek
 */
class Commands : public Fl_Group
{
  public:
    /**
     * @param[in] X The x coordinate of the widget.
     * @param[in] Y The y coordinate of the widget.
     * @param[in] W The width of the widget.
     * @param[in] H The height of the widget.
     * Constructor.
     */
    Commands(const int X, const int Y, const int W, const int H) : Fl_Group(X, Y, W, H, nullptr)
    {
        static constexpr int GAP = 5;
        static constexpr int H_LINE = 45;
        static constexpr int W_LABEL = 130;
        static constexpr int W_FLIP = 100;
        static constexpr int W_SPINNER = 100;
        static constexpr int W_SEND = 30;
        static constexpr int W_LED = 100;
        static const int X0 = X + 20;
        static const int X1 = X0 + W_LABEL + GAP;
        static const int X2 = X1 + W_FLIP + GAP;
        static const int X3 = X2 + W_SPINNER + GAP;
        static const int X4 = X3 + W_SEND + GAP;
        const auto createLabel = [Y](Fl_Box*& label, const int line, const char* l)
        {
            label = new Fl_Box(X0, Y + H_LINE * line, W_LABEL, H_LINE, l);
            label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        };
        createLabel(_labelMode, 1, "CV Mode");
        createLabel(_labelInverterControl, 2, "Inverter Control");
        createLabel(_labelReset, 3, "Reset Inverter");
        createLabel(_labelWriteEeprom, 4, "Write Eeprom (attention!)");

        _flipMode = new FlipButton(X1, Y + H_LINE * 1, W_FLIP, H_LINE);
        _flipInverterControl = new FlipButton(X1, Y + H_LINE * 2, W_FLIP, H_LINE);

        _spinner = new Fl_Spinner(X2, Y + H_LINE * 1, W_SPINNER, H_LINE, nullptr);
        _spinner->type(FL_FLOAT_INPUT);
        _spinner->minimum(0.1);
        _spinner->maximum(6553.5);
        _spinner->step(0.1);
        _spinner->value(24.0);

        const auto createButton = [this, Y](Fl_Button*& button, const int line, Fl_Callback* cb)
        {
            button = new Fl_Button(X3, Y + H_LINE * line, W_SEND, H_LINE, "@+5>");
            button->callback(cb, this);
        };

        createButton(_sendMode, 1, sendModeCb);
        createButton(_sendInverterControl, 2, sendInverterControlCb);
        createButton(_sendReset, 3, sendResetCb);
        createButton(_sendWriteEeprom, 4, sendWriteEepromCb);

        _ledMode = new Led(X4, Y + H_LINE * 1, W_LED, H_LINE, "CV MODE");
        _ledInverterControl = new Led(X4, Y + H_LINE * 2, W_LED, H_LINE, "RELAY ON");

        this->end();
    }
    /**
     * Destructor.
     */
    ~Commands() {}
    /**
     * @param canTakeRequest Wether there is space to send request.
     * @param response The received response.
     * @param request The corresponding request to received response.
     * @return Returns the request to send.
     */
    catta::modbus::si::request::Request work(const bool canTakeRequest, const catta::modbus::si::response::Response& response,
                                             const catta::modbus::si::request::Request& request)
    {
        if (canTakeRequest && !_request.isEmpty())
        {
            const catta::modbus::si::request::Request result = _request;
            _request = {};
            _sendMode->show();
            _sendInverterControl->show();
            _sendReset->show();
            _sendWriteEeprom->show();
            return result;
        }
        (void)response;
        (void)request;
        return catta::modbus::si::request::Request::empty();
    }
    /**
     * @param[in] relayOn The relayOn.
     */
    void setRelayOn(const std::optional<bool> relayOn) { _ledInverterControl->set(relayOn); }
    /**
     * @param[in] cvMode The cvMode.
     */
    void setCvMode(const std::optional<bool> cvMode) { _ledMode->set(cvMode); }
    /**
     * Show locked stuff.
     */
    void lock()
    {
        _labelReset->hide();
        _labelWriteEeprom->hide();
        _sendReset->hide();
        _sendWriteEeprom->hide();
    }
    /**
     * Hide locked stuff.
     */
    void unlock()
    {
        _labelReset->show();
        _labelWriteEeprom->show();
        _sendReset->show();
        _sendWriteEeprom->show();
    }

  private:
    class FlipButton : public Fl_Group
    {
      public:
        FlipButton(const int X, const int Y, const int W, const int H) : Fl_Group(X, Y, W, H, nullptr)
        {
            const int w = W / 2;
            const int X0 = X;
            const int X1 = X0 + w;
            _on = new Fl_Button(X0, Y, w, H, "On");
            _on->type(102);
            _on->value(1);
            _on->compact(1);
            _on->callback(onCb, this);
            _off = new Fl_Button(X1, Y, w, H, nullptr);
            _off->type(102);
            _off->compact(1);
            _off->callback(offCb, this);

            this->end();
        }
        bool value() { return _on->value(); }

      private:
        Fl_Button* _on;
        Fl_Button* _off;
        static constexpr const char* ON = "On";
        static constexpr const char* OFF = "Off";
        static void onCb(Fl_Widget*, void* object)
        {
            FlipButton* fb = static_cast<FlipButton*>(object);
            if (fb)
            {
                fb->_on->label(ON);
                fb->_off->label(nullptr);
            }
        }
        static void offCb(Fl_Widget*, void* object)
        {
            FlipButton* fb = static_cast<FlipButton*>(object);
            if (fb)
            {
                fb->_on->label(nullptr);
                fb->_off->label(OFF);
            }
        }
    };
    Fl_Box* _labelMode;
    Fl_Box* _labelInverterControl;
    Fl_Box* _labelReset;
    Fl_Box* _labelWriteEeprom;
    FlipButton* _flipMode;
    FlipButton* _flipInverterControl;
    Fl_Spinner* _spinner;
    Fl_Button* _sendMode;
    Fl_Button* _sendInverterControl;
    Fl_Button* _sendReset;
    Fl_Button* _sendWriteEeprom;
    Led* _ledMode;
    Led* _ledInverterControl;
    catta::modbus::si::request::Request _request;

    void hideButtons()
    {
        _sendMode->hide();
        _sendInverterControl->hide();
        _sendReset->hide();
        _sendWriteEeprom->hide();
    }

    static void sendModeCb(Fl_Widget*, void* object)
    {
        Commands* commands = static_cast<Commands*>(object);
        if (commands)
        {
            const bool on = commands->_flipMode->value();
            const auto value = [commands]()
            {
                const catta::modbus::sunspec::ValueU16 uSet =
                    catta::modbus::sunspec::ValueU16::create(static_cast<std::uint16_t>(commands->_spinner->value() * 10.0));
                return catta::modbus::si::request::ConstantVoltage::create(uSet);
            };
            commands->_request =
                on ? catta::modbus::si::request::Request::startConstantVoltage(value()) : catta::modbus::si::request::Request::endConstantVoltage();
            commands->hideButtons();
        }
    }

    static void sendInverterControlCb(Fl_Widget*, void* object)
    {
        Commands* commands = static_cast<Commands*>(object);
        if (commands)
        {
            const bool on = commands->_flipInverterControl->value();
            commands->_request =
                on ? catta::modbus::si::request::Request::switchOnGridRelay() : catta::modbus::si::request::Request::switchOffGridRelay();
            commands->hideButtons();
        }
    }

    static void sendResetCb(Fl_Widget*, void* object)
    {
        Commands* commands = static_cast<Commands*>(object);
        if (commands)
        {
            commands->_request = REQUEST_RESET;
            commands->hideButtons();
        }
    }

    static void sendWriteEepromCb(Fl_Widget*, void* object)
    {
        Commands* commands = static_cast<Commands*>(object);
        if (commands)
        {
            commands->_request = REQUEST_WRITE_EEPROM;
            commands->hideButtons();
        }
    }

    using Request = catta::modbus::si::request::Request;
    using RegisterAddress = catta::modbus::si::RegisterAddress;
    using Value = catta::modbus::sunspec::ValueU16;
    using WriteRegister = catta::modbus::si::WriteRegister;

    constexpr static RegisterAddress REGISTER_RESET = RegisterAddress::siControlFunctionReset();
    constexpr static RegisterAddress REGISTER_WRITE_EEPROM = RegisterAddress::siControlFunctionWriteEeprom();
    constexpr static Value VALUE = Value::create(0x0001);
    constexpr static Request REQUEST_RESET = Request::writeRegister(WriteRegister::create(REGISTER_RESET, VALUE));
    constexpr static Request REQUEST_WRITE_EEPROM = Request::writeRegister(WriteRegister::create(REGISTER_WRITE_EEPROM, VALUE));
};
}  // namespace gui
}  // namespace catta
