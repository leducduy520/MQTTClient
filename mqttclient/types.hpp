/**
 * @file types.hpp
 * @brief Definition of common MQTT client types.
 *
 * This file contains declarations for exception tracking, disconnect data,
 * and a variant type to store callback data of various types.
 *
 * @author duyld15
 * @date February 23, 2025
 */

#ifndef __CORE_MQTT_TYPES__
#define __CORE_MQTT_TYPES__

#include "mqtt/exception.h"
#include <memory>
#include <string>

namespace mqttcpp {
/**
 * @brief Tracks exceptions in the MQTT client.
 *
 * This structure stores an MQTT exception along with flags indicating
 * whether the exception is from an unknown or external source.
 */
struct ExceptionTrace {
  mqtt::exception exc; ///< The stored MQTT exception.
  bool unknown;        ///< Flag indicating if the exception is unknown.
  bool external;       ///< Flag indicating if the exception is external.

  /**
   * @brief Push an exception into the trace.
   *
   * If @p is_unknown is false, the provided exception @p e is stored.
   * The @p external flag is set to false.
   *
   * @param is_unknown Boolean flag indicating if the exception is unknown.
   * @param e The exception to record (default is mqtt::exception(0)).
   */
  void push(bool is_unknown, mqtt::exception e = mqtt::exception(0)) {
    unknown = is_unknown;
    if (!is_unknown) {
      exc = e;
    }
    external = false;
  }

  /**
   * @brief Push an exception with a message.
   *
   * If @p is_unknown is false, the exception is constructed using the
   * provided message. The @p external flag is set to true.
   *
   * @param is_unknown Boolean flag indicating if the exception is unknown.
   * @param msg The error message for the exception.
   */
  void push(bool is_unknown, const char *msg) {
    unknown = is_unknown;
    if (!is_unknown) {
      exc = mqtt::exception(0, msg);
    }
    external = true;
  }

  /**
   * @brief Check if the exception state is okay.
   *
   * This method returns true if no unknown or external exception was set
   * and the reason code within the exception is 0.
   *
   * @return True if exception state is okay, false otherwise.
   */
  bool ok() { return !unknown && !external && (exc.get_reason_code() == 0); }

  /**
   * @brief Default constructor.
   *
   * Initializes the exception with a default value and clears flags.
   */
  ExceptionTrace() : exc(mqtt::exception(0)), unknown(false), external(false) {}
};

/// Shared pointer type for ExceptionTrace.
using exception_trace_ptr = std::shared_ptr<ExceptionTrace>;

/**
 * @brief Data associated with a disconnect event.
 *
 * Contains MQTT properties and the reason code for the disconnection.
 */
struct disconnect_data {
  mqtt::properties props;  ///< MQTT properties for disconnect.
  mqtt::ReasonCode reason; ///< Disconnect reason code.
};

/**
 * @brief Supported types for CallbackVariant.
 */
enum class VariantType {
  String,               ///< Holds a std::string.
  TokenPointer,         ///< Holds a mqtt::token_ptr.
  MessagePointer,       ///< Holds a mqtt::const_message_ptr.
  DeliveryTokenPointer, ///< Holds a mqtt::delivery_token_ptr.
  DisconnectData,       ///< Holds a disconnect_data.
  None                  ///< No value stored.
};

/**
 * @brief A type-safe variant container for callback data.
 *
 * The CallbackVariant class can hold values of various types (token pointer,
 * message pointer, delivery token pointer, string, or disconnect data)
 * and provides accessor functions for type-safe retrieval.
 */
class CallbackVariant {
public:
  /**
   * @brief Default constructor.
   *
   * Constructs a CallbackVariant with no value.
   */
  CallbackVariant() : type_(VariantType::None) {}

  /**
   * @brief Constructor for token pointer.
   *
   * @param tok_ptr A mqtt::token_ptr value.
   */
  CallbackVariant(mqtt::token_ptr tok_ptr) : type_(VariantType::TokenPointer) {
    new (&data_.tok_ptr) mqtt::token_ptr(tok_ptr);
  }

  /**
   * @brief Constructor for message pointer.
   *
   * @param mes_ptr A mqtt::const_message_ptr value.
   */
  CallbackVariant(mqtt::const_message_ptr mes_ptr)
      : type_(VariantType::MessagePointer) {
    new (&data_.mes_ptr) mqtt::const_message_ptr(mes_ptr);
  }

  /**
   * @brief Constructor for delivery token pointer.
   *
   * @param del_tok_ptr A mqtt::delivery_token_ptr value.
   */
  CallbackVariant(mqtt::delivery_token_ptr del_tok_ptr)
      : type_(VariantType::DeliveryTokenPointer) {
    new (&data_.del_tok_ptr) mqtt::delivery_token_ptr(del_tok_ptr);
  }

  /**
   * @brief Constructor for std::string.
   *
   * @param str A string value.
   */
  CallbackVariant(const std::string &str) : type_(VariantType::String) {
    new (&data_.str) std::string(str);
  }

  /**
   * @brief Constructor for disconnect data.
   *
   * @param disconn_data A disconnect_data value.
   */
  CallbackVariant(const disconnect_data &disconn_data)
      : type_(VariantType::DisconnectData) {
    new (&data_.disconn_data) disconnect_data(disconn_data);
  }

  /**
   * @brief Copy constructor.
   *
   * Performs a deep copy of the stored value.
   *
   * @param other The instance to copy from.
   */
  CallbackVariant(const CallbackVariant &other) : type_(VariantType::None) {
    copyFrom(other);
  }

  /**
   * @brief Copy assignment operator.
   *
   * Cleans up the current value and then copies from @p other.
   *
   * @param other The instance to assign from.
   * @return A reference to this instance.
   */
  CallbackVariant &operator=(const CallbackVariant &other) {
    if (this != &other) {
      destroy();
      copyFrom(other);
    }
    return *this;
  }

  /**
   * @brief Destructor.
   *
   * Calls destroy() to cleanup the stored value.
   */
  ~CallbackVariant() { destroy(); }

  /**
   * @brief Returns the active type.
   *
   * @return The VariantType indicating which data is currently stored.
   */
  VariantType type() const { return type_; }

  /**
   * @brief Retrieve stored token pointer.
   *
   * @exception std::runtime_error if the stored type is not TokenPointer.
   * @return The mqtt::token_ptr stored in the variant.
   */
  mqtt::token_ptr asToken() const {
    if (type_ != VariantType::TokenPointer)
      throw std::runtime_error(
          "CallbackVariant is not holding a token pointer");
    return data_.tok_ptr;
  }

  /**
   * @brief Retrieve stored message pointer.
   *
   * @exception std::runtime_error if the stored type is not MessagePointer.
   * @return The mqtt::const_message_ptr stored in the variant.
   */
  mqtt::const_message_ptr asMessage() const {
    if (type_ != VariantType::MessagePointer)
      throw std::runtime_error(
          "CallbackVariant is not holding a mqtt::const_message_ptr");
    return data_.mes_ptr;
  }

  /**
   * @brief Retrieve stored delivery token pointer.
   *
   * @exception std::runtime_error if the stored type is not
   * DeliveryTokenPointer.
   * @return The mqtt::delivery_token_ptr stored in the variant.
   */
  mqtt::delivery_token_ptr asDeliveryToken() const {
    if (type_ != VariantType::DeliveryTokenPointer)
      throw std::runtime_error(
          "CallbackVariant is not holding a mqtt::delivery_token_ptr");
    return data_.del_tok_ptr;
  }

  /**
   * @brief Retrieve stored string.
   *
   * @exception std::runtime_error if the stored type is not String.
   * @return A const reference to the stored std::string.
   */
  const std::string &asString() const {
    if (type_ != VariantType::String)
      throw std::runtime_error("CallbackVariant is not holding a std::string");
    return data_.str;
  }

  /**
   * @brief Retrieve stored disconnect data.
   *
   * @exception std::runtime_error if the stored type is not DisconnectData.
   * @return A const reference to the stored disconnect_data.
   */
  const disconnect_data &asDisconnectData() const {
    if (type_ != VariantType::DisconnectData)
      throw std::runtime_error(
          "CallbackVariant is not holding a disconnect_data");
    return data_.disconn_data;
  }

private:
  /**
   * @brief Destroys the currently stored value.
   *
   * Calls the appropriate destructor or resets the stored smart pointer,
   * and then sets the type to None.
   */
  void destroy() {
    if (type_ == VariantType::String) {
      data_.str.~basic_string();
    }
    if (type_ == VariantType::DisconnectData) {
      data_.disconn_data.~disconnect_data();
    }
    if (type_ == VariantType::MessagePointer) {
      data_.mes_ptr.reset();
    }
    if (type_ == VariantType::TokenPointer) {
      data_.tok_ptr.reset();
    }
    if (type_ == VariantType::DeliveryTokenPointer) {
      data_.del_tok_ptr.reset();
    }
    type_ = VariantType::None;
  }

  /**
   * @brief Copies the stored value from another CallbackVariant.
   *
   * Uses placement new to initialize the stored data of this instance
   * according to the type in @p other.
   *
   * @param other The source instance to copy from.
   */
  void copyFrom(const CallbackVariant &other) {
    type_ = other.type_;
    switch (other.type_) {
    case VariantType::TokenPointer:
      new (&data_.tok_ptr) mqtt::token_ptr(other.data_.tok_ptr);
      break;
    case VariantType::MessagePointer:
      new (&data_.mes_ptr) mqtt::const_message_ptr(other.data_.mes_ptr);
      break;
    case VariantType::DeliveryTokenPointer:
      new (&data_.del_tok_ptr)
          mqtt::delivery_token_ptr(other.data_.del_tok_ptr);
      break;
    case VariantType::String:
      new (&data_.str) std::string(other.data_.str);
      break;
    case VariantType::DisconnectData:
      new (&data_.disconn_data) disconnect_data(other.data_.disconn_data);
      break;
    case VariantType::None:
    default:
      break;
    }
  }

  VariantType type_; ///< Active type stored in the variant.

  /**
   * @brief Union to hold one of several data types.
   *
   * The union is used to store different types without memory overhead.
   */
  union Data {
    std::string str;                      ///< Holds a string.
    mqtt::token_ptr tok_ptr;              ///< Holds a token pointer.
    mqtt::const_message_ptr mes_ptr;      ///< Holds a message pointer.
    mqtt::delivery_token_ptr del_tok_ptr; ///< Holds a delivery token pointer.
    mqtt::connect_data conn_data;         ///< (Unused in CallbackVariant)
    disconnect_data disconn_data;         ///< Holds disconnect data.

    Data() {}  ///< Default constructor does nothing.
    ~Data() {} ///< Destruction is managed manually.
  } data_;
};
} // namespace mqttcpp

#endif // __CORE_MQTT_TYPES__