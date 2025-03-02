/**
 * @file types.hpp
 * @brief Definition of common MQTT client types.
 *
 * This file contains declarations for exception tracking, disconnect data,
 * and a variant type to store callback data of various types.
 *
 * @author duyld15
 */
#ifndef __CORE_MQTT_TYPES__
#define __CORE_MQTT_TYPES__
#include "mqtt/exception.h"
#include <memory>
#include <string>
#include <type_traits>

namespace mqttcpp {
/**
 * @brief Enum representing different types of exceptions.
 */
enum class ExceptionType {
  MQTT,     ///< MQTT exception type.
  STANDARD, ///< Standard exception type.
  UNKNOWN,  ///< Unknown exception type.
  NONE      ///< No exception.
};

/**
 * @brief Wrapper class to store one of several exception types.
 *
 * The ExceptionTrace class encapsulates exceptions of type mqtt::exception,
 * std::exception, or an unknown exception represented by a std::string. It uses
 * a union to hold the actual exception data, and an ExceptionType enum to track
 * which type is currently active.
 *
 * This class provides proper copy semantics and ensures that the stored
 * exception is correctly constructed and destroyed.
 */
class ExceptionTrace {
public:
  /**
   * @brief Default constructor that initializes an empty exception trace.
   */
  ExceptionTrace() : type_(ExceptionType::NONE) {}

  /**
   * @brief Constructs an ExceptionTrace from a mqtt::exception.
   * @param ex The mqtt::exception object to store.
   */
  ExceptionTrace(const mqtt::exception &ex) : type_(ExceptionType::MQTT) {
    new (&data_.mqttException) mqtt::exception(ex);
  }

  /**
   * @brief Constructs an ExceptionTrace from a std::exception.
   * @param ex The std::exception object to store.
   */
  ExceptionTrace(const std::exception &ex) : type_(ExceptionType::STANDARD) {
    new (&data_.standardException) std::exception(ex);
  }

  /**
   * @brief Constructs an ExceptionTrace from a std::string representing an
   * unknown exception.
   * @param message The message describing the unknown exception.
   */
  ExceptionTrace(const std::string &message) : type_(ExceptionType::UNKNOWN) {
    new (&data_.unknownException) std::string(message);
  }

  /**
   * @brief Destructor that properly destroys the stored exception.
   */
  ~ExceptionTrace() { destroy(); }

  /**
   * @brief Copy constructor.
   * @param other The ExceptionTrace object to copy from.
   */
  ExceptionTrace(const ExceptionTrace &other) : type_(ExceptionType::NONE) {
    copyFrom(other);
  }

  /**
   * @brief Assignment operator.
   * @param other The ExceptionTrace object to assign from.
   * @return A reference to this ExceptionTrace after assignment.
   */
  ExceptionTrace &operator=(const ExceptionTrace &other) {
    if (this != &other) {
      destroy();
      copyFrom(other);
    }
    return *this;
  }

  /**
   * @brief Retrieves the type of the stored exception.
   * @return The ExceptionType representing the active exception type.
   */
  ExceptionType getVariant() const { return type_; }

  /**
   * @brief Retrieves the stored mqtt::exception.
   * @return Pointer to the mqtt::exception if the active type is MQTT;
   * otherwise, nullptr.
   */
  const mqtt::exception *getMqttException() const {
    return (type_ == ExceptionType::MQTT) ? &data_.mqttException : nullptr;
  }

  /**
   * @brief Retrieves the stored std::exception.
   * @return Pointer to the std::exception if the active type is STANDARD;
   * otherwise, nullptr.
   */
  const std::exception *getStandardException() const {
    return (type_ == ExceptionType::STANDARD) ? &data_.standardException
                                              : nullptr;
  }

  /**
   * @brief Retrieves the stored unknown exception message.
   * @return Pointer to the std::string if the active type is UNKNOWN;
   * otherwise, nullptr.
   */
  const std::string *getUnknownException() const {
    return (type_ == ExceptionType::UNKNOWN) ? &data_.unknownException
                                             : nullptr;
  }

private:
  /**
   * @brief Destroys the currently stored exception object.
   *
   * This method invokes the appropriate destructor based on the current
   * exception type and resets the type to NONE.
   */
  void destroy() {
    switch (type_) {
    case ExceptionType::MQTT:
      data_.mqttException.~exception();
      break;
    case ExceptionType::STANDARD:
      data_.standardException.~exception();
      break;
    case ExceptionType::UNKNOWN:
      data_.unknownException.~basic_string();
      break;
    case ExceptionType::NONE:
      break;
    }
    type_ = ExceptionType::NONE;
  }

  /**
   * @brief Copies the contents from another ExceptionTrace.
   *
   * This method creates a copy of the exception stored in the other
   * ExceptionTrace instance using placement new.
   *
   * @param other The ExceptionTrace object to copy from.
   */
  void copyFrom(const ExceptionTrace &other) {
    type_ = other.type_;
    switch (other.type_) {
    case ExceptionType::MQTT:
      new (&data_.mqttException) mqtt::exception(other.data_.mqttException);
      break;
    case ExceptionType::STANDARD:
      new (&data_.standardException)
          std::exception(other.data_.standardException);
      break;
    case ExceptionType::UNKNOWN:
      new (&data_.unknownException) std::string(other.data_.unknownException);
      break;
    case ExceptionType::NONE:
      break;
    }
  }

  ExceptionType type_; ///< Indicates the type of exception currently stored.

  /**
   * @brief Union that holds one of the possible exception types.
   *
   * This union can store one of the following:
   * - mqtt::exception for MQTT exceptions.
   * - std::exception for standard exceptions.
   * - std::string for unknown exception messages.
   *
   * The default constructor and destructor are empty as the management of the
   * contained object is handled manually.
   */
  union ExceptionData {
    mqtt::exception mqttException;    ///< Storage for mqtt::exception.
    std::exception standardException; ///< Storage for std::exception.
    std::string unknownException; ///< Storage for unknown exception message.

    /**
     * @brief Default constructor for the union. Does nothing.
     */
    ExceptionData() {}

    /**
     * @brief Destructor for the union. Actual destruction is managed manually.
     */
    ~ExceptionData() {}
  } data_;
};

/**
 * @brief Shared pointer type for ExceptionTrace.
 */
using exception_trace_ptr = std::shared_ptr<ExceptionTrace>;

/**
 * @brief Data associated with a disconnect event.
 *
 * Contains MQTT properties and the reason code for the disconnection.
 */
struct disconnect_data {
  mqtt::properties props;
  mqtt::ReasonCode reason;
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

    Data() {} ///< Default constructor does nothing.

    ~Data() {} ///< Destruction is managed manually.
  } data_;
};
} // namespace mqttcpp

#endif // __CORE_MQTT_TYPES__
