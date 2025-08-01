#pragma once

// std
#include <cstdint>

namespace catta
{
namespace modbus
{
namespace si
{
/**
 * @brief Class for representing the power config.
 *
 * @author CattaTech - Maik Urbannek
 */
class PowerConfig
{
  public:
    /**
     * Default constructor. Creates invalid power config.
     */
    constexpr PowerConfig() noexcept : PowerConfig(EMPTY, false) {}
    /**
     * @return Returns invalid power config.
     */
    constexpr static PowerConfig empty() noexcept { return PowerConfig{}; }
    /**
     *  @return Returns @b true if power config is empty, otherwise @b false.
     */
    constexpr bool isEmpty() const noexcept { return _power >= EMPTY; }
    /**
     * @param[in] power The power value. Has to be 3200 or below, otherwise empty is returned.
     * @param[in] charging True represents charging.
     * @return Returns power config if input is valid, otherwise empty.
     */
    static constexpr PowerConfig create(const std::uint16_t power, bool charging) { return power >= EMPTY ? empty() : PowerConfig(power, charging); }
    /**
     * @param[in] raw The raw value. Has to be valid, otherwise empty is returned.
     * @return Returns power config if input is valid, otherwise empty.
     */
    static constexpr PowerConfig fromRaw(const std::uint16_t raw) { return PowerConfig::create(raw & 0x7fff, (raw & 0x8000) != 0); }
    /**
     * @return Returns th power. Is only valid if not empty.
     */
    constexpr std::uint16_t power() const noexcept { return _power; }
    /**
     * @return Returns the charging. Is only valid if not empty.
     */
    constexpr bool charging() const noexcept { return _charging; }
    /**
     * @param[in] other The other PowerConfig.
     * @return Returns @b true if the two PowerConfig objects are the same, otherwise @b false.
     */
    constexpr bool operator==(const PowerConfig& other) const noexcept = default;

  private:
    constexpr PowerConfig(const std::uint16_t power, bool charging) : _power(power), _charging(charging) {}
    std::uint16_t _power;
    bool _charging;
    static constexpr std::uint16_t EMPTY = 3201;
};
}  // namespace si
}  // namespace modbus
}  // namespace catta
