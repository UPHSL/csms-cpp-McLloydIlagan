#pragma once

#include <string>

namespace csms {

/**
 * Resident domain model.
 *
 * Represents a person registered within the community. This model is the
 * foundation for resident-related features in the CSMS.
 *
 * Status is represented as a std::string to support open-ended values
 * and validation by ResidentValidator.
 */
class Resident
{
public:
    Resident() = default;

    // Constructor without id — id is unassigned before persistence
    Resident(const std::string& firstName,
             const std::string& lastName,
             const std::string& address,
             const std::string& contactNumber,
             const std::string& email,
             const std::string& status = "Active")
        : firstName_(firstName),
          lastName_(lastName),
          address_(address),
          contactNumber_(contactNumber),
          email_(email),
          status_(status)
    {
    }

    // Constructor with id — used after persistence assigns an id
    Resident(int id,
             const std::string& firstName,
             const std::string& lastName,
             const std::string& address,
             const std::string& contactNumber,
             const std::string& email,
             const std::string& status = "Active")
        : id_(id),
          firstName_(firstName),
          lastName_(lastName),
          address_(address),
          contactNumber_(contactNumber),
          email_(email),
          status_(status)
    {
    }

    // Getters
    int getId() const { return id_; }
    const std::string& getFirstName() const { return firstName_; }
    const std::string& getLastName() const { return lastName_; }
    const std::string& getAddress() const { return address_; }
    const std::string& getContactNumber() const { return contactNumber_; }
    const std::string& getEmail() const { return email_; }
    const std::string& getStatus() const { return status_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setFirstName(const std::string& firstName) { firstName_ = firstName; }
    void setLastName(const std::string& lastName) { lastName_ = lastName; }
    void setAddress(const std::string& address) { address_ = address; }
    void setContactNumber(const std::string& contactNumber) { contactNumber_ = contactNumber; }
    void setEmail(const std::string& email) { email_ = email; }
    void setStatus(const std::string& status) { status_ = status; }

private:
    int id_{0};
    std::string firstName_;
    std::string lastName_;
    std::string address_;
    std::string contactNumber_;
    std::string email_;
    std::string status_{"Active"};
};

} // namespace csms
