#pragma once

// fltk
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input.H>

// hardware
#include <catta/hardware/uart/Baudrate.hpp>
#include <catta/hardware/uart/DataBits.hpp>
#include <catta/hardware/uart/Parity.hpp>
#include <catta/hardware/uart/StopBits.hpp>

// std
#include <optional>

namespace catta
{
namespace gui
{
/**
 * @tparam UART Uart class.
 * @brief Handles Uart config and connection.
 * @author CattaTech - Maik Urbannek
 */
template <class UART>
class Uart : public Fl_Group
{
  private:
    static constexpr int W_GAP = 10;
    static constexpr int W_INPUT = 100;
    static constexpr int W_BUTTON = 100;
    static constexpr int W_BOX = 300;
    static constexpr int defaultWidth = W_INPUT + W_GAP + W_BUTTON + W_GAP + W_BUTTON + W_GAP + W_BOX + W_GAP + W_INPUT;
    static constexpr int defaultHeight = 50;

  public:
    /**
     * @param[in] X The x coordinate of the widget.
     * @param[in] Y The y coordinate of the widget.
     * @param[in] W The width of the widget.
     * @param[in] H The height of the widget.
     * @param[in] defaultName The default device address of the uart.
     * @param[in] L The label name.
     */
    Uart(int X, int Y, int W, int H, const char* defaultName, const char* L = nullptr)
        : Fl_Group(0, 0, W, H, L), _uart(UART::empty()), _error(ERROR::empty())
    {
        const int x = (W - defaultWidth) / 2;
        const int y = (H - defaultHeight) / 2;
        this->_input = new Fl_Input(x, y, W_INPUT, defaultHeight);
        this->_input->value(defaultName);
        this->_button0 = new Fl_Button(x + W_INPUT + W_GAP, y, W_BUTTON, defaultHeight, "Connect");
        this->_button0->callback(opencb, this);
        this->_button1 = new Fl_Button(x + W_INPUT + W_GAP + W_BUTTON + W_GAP, y, W_BUTTON, defaultHeight, "Disconnect");
        this->_button1->callback(closecb, this);
        this->_button1->deactivate();
        this->_box = new Fl_Box(x + W_INPUT + W_GAP + W_BUTTON + W_GAP + W_BUTTON + W_GAP, y, W_BOX, defaultHeight, getErrorString());
        this->_box->box(FL_DOWN_BOX);
        auto menu = [](const char* text)
        {
            return Fl_Menu_Item{.text = text,
                                .shortcut_ = 0,
                                .callback_ = nullptr,
                                .user_data_ = nullptr,
                                .flags = 0,
                                .labeltype_ = 0,
                                .labelfont_ = 0,
                                .labelsize_ = 0,
                                .labelcolor_ = 0};
        };
        static const Fl_Menu_Item id[] = {menu("00"), menu("01"), menu("02"), menu("03"), menu("04"),   menu("05"),
                                          menu("06"), menu("07"), menu("08"), menu("09"), menu("0a"),   menu("0b"),
                                          menu("0c"), menu("0d"), menu("0e"), menu("0f"), menu(nullptr)};
        this->_choice = new Fl_Choice(x + W_INPUT + W_GAP + W_BUTTON + W_GAP + W_BUTTON + W_GAP + W_BOX + W_GAP, y, W_INPUT, defaultHeight);
        this->_choice->menu(id);
        this->_choice->value(0x0c);
        this->resize(X, Y, W, H);
        this->end();
    }
    /**
     * @param[in] c The byte to send.
     * @return Returns @b true if the byte was sent, otherwise @b false.
     */
    [[nodiscard]] bool write(const char c) noexcept
    {
        if (this->_uart.isEmpty()) return false;
        const auto result = this->_uart.write(c);
        if (this->_button0 && (this->_uart.isEmpty() || !this->_uart.error().isEmpty())) this->setDisconnect();
        return result;
    }
    /**
     * @return Returns the recevied character if received, otherwise empty.
     */
    [[nodiscard]] std::optional<char> read() noexcept
    {
        if (this->_uart.isEmpty()) return std::optional<char>{};
        const auto result = this->_uart.read();
        if (this->_button0 && (this->_uart.isEmpty() || !this->_uart.error().isEmpty())) this->setDisconnect();
        return result;
    }
    /**
     * @return Returns choosen id.
     */
    [[nodiscard]] std::uint8_t id() const noexcept { return static_cast<std::uint8_t>(this->_choice->value()); }
    /**
     * Destructor.
     */
    ~Uart()
    {
        if (_input) delete _input;
        if (_button0) delete _button0;
        if (_button1) delete _button1;
        if (_box) delete _box;
        if (_choice) delete _choice;
    }

  private:
    UART _uart;
    Fl_Input* _input;
    Fl_Button* _button0;
    Fl_Button* _button1;
    Fl_Box* _box;
    Fl_Choice* _choice;
    using ERROR = decltype(UART::empty().error());
    ERROR _error;
    const char* getErrorString()
    {
        return this->_uart.isEmpty() ? "NOT_CONNECTED" : this->_error.isEmpty() ? "CONNECTED" : ERROR::enumNames[this->_error].data();
    }
    static void opencb(Fl_Widget*, void* object)
    {
        Uart* uart = static_cast<Uart*>(object);
        static constexpr auto baudrate = catta::hardware::uart::Baudrate::baudrate57600();
        static constexpr auto parity = catta::hardware::uart::Parity::empty();
        static constexpr auto dataBits = catta::hardware::uart::DataBits::eight();
        static constexpr auto stopBits = catta::hardware::uart::StopBits::one();
        if (uart && uart->_input && uart->_box && uart->_button0)
        {
            const std::string_view device = uart->_input->value();
            uart->_uart = UART::create(device, baudrate, parity, dataBits, stopBits);
            uart->_error = uart->_uart.error();
            uart->_box->label(uart->getErrorString());
            if (!(uart->_uart.isEmpty()) && uart->_uart.error().isEmpty())
            {
                uart->_button0->deactivate();
                uart->_button1->activate();
            }
            else
                uart->_uart = UART::empty();
        }
    }
    static void closecb(Fl_Widget*, void* object)
    {
        Uart* uart = static_cast<Uart*>(object);
        if (uart && uart->_box && uart->_button0)
        {
            uart->_uart = UART::empty();
            uart->setDisconnect();
        }
    }
    void setDisconnect()
    {
        this->_button0->activate();
        this->_button1->deactivate();
        this->_error = _uart.error();
        this->_box->label(getErrorString());
    }
};
}  // namespace gui
}  // namespace catta
