#pragma once

#include <string>

namespace csms {


enum class ResidentStatus
{
    Active
};


// Resident domain model.

class Resident
{
public:
    Resident() = default;

    Resident(int id,
             const std::string& firstName,
             const std::string& lastName,
             const std::string& address,
             const std::string& contactNumber,
             const std::string& email,
             ResidentStatus status = ResidentStatus::Active)
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
    ResidentStatus getStatus() const { return status_; }

    // Setters
    void setId(int id) { id_ = id; }
    void setFirstName(const std::string& firstName) { firstName_ = firstName; }
    void setLastName(const std::string& lastName) { lastName_ = lastName; }
    void setAddress(const std::string& address) { address_ = address; }
    void setContactNumber(const std::string& contactNumber) { contactNumber_ = contactNumber; }
    void setEmail(const std::string& email) { email_ = email; }
    void setStatus(ResidentStatus status) { status_ = status; }

private:
    int id_{0};
    std::string firstName_;
    std::string lastName_;
    std::string address_;
    std::string contactNumber_;
    std::string email_;
    ResidentStatus status_{ResidentStatus::Active};
};

} // namespace csms
