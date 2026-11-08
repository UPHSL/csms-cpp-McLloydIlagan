#pragma once

#include <optional>
#include <string>

namespace csms {

/**
 * ServiceRequest domain model.
 *
 * Represents a community service request associated with a Resident.
 * This model is the structural foundation for Service Request features.
 *
 * Responsibilities:
 * - Hold the six required Service Request fields.
 * - Default status to "Pending" when not explicitly supplied.
 * - Keep the id unassigned (std::nullopt) before persistence.
 *
 * This class does not:
 * - Validate Service Request information.
 * - Persist Service Request records.
 * - Verify that residentId refers to an existing Resident.
 * - Manage status transitions.
 */
class ServiceRequest
{
public:
    ServiceRequest() = default;

    // Constructor without id — id is unassigned before persistence.
    ServiceRequest(int residentId,
                   const std::string& serviceType,
                   const std::string& description,
                   const std::string& dateRequested,
                   const std::string& status = "Pending")
        : residentId_(residentId),
          serviceType_(serviceType),
          description_(description),
          dateRequested_(dateRequested),
          status_(status)
    {
    }

    // Constructor with id — used after persistence assigns an id.
    ServiceRequest(int id,
                   int residentId,
                   const std::string& serviceType,
                   const std::string& description,
                   const std::string& dateRequested,
                   const std::string& status = "Pending")
        : id_(id),
          residentId_(residentId),
          serviceType_(serviceType),
          description_(description),
          dateRequested_(dateRequested),
          status_(status)
    {
    }

    // Getters
    std::optional<int> getId()            const { return id_; }
    int                getResidentId()    const { return residentId_; }
    const std::string& getServiceType()   const { return serviceType_; }
    const std::string& getDescription()  const { return description_; }
    const std::string& getDateRequested() const { return dateRequested_; }
    const std::string& getStatus()        const { return status_; }

    // Setters
    void setId(int id)                                { id_ = id; }
    void setResidentId(int residentId)                { residentId_ = residentId; }
    void setServiceType(const std::string& t)         { serviceType_ = t; }
    void setDescription(const std::string& d)         { description_ = d; }
    void setDateRequested(const std::string& date)    { dateRequested_ = date; }
    void setStatus(const std::string& status)         { status_ = status; }

private:
    std::optional<int> id_{std::nullopt};  // unassigned until persisted
    int                residentId_{0};
    std::string        serviceType_;
    std::string        description_;
    std::string        dateRequested_;
    std::string        status_{"Pending"}; // T08 default
};

} // namespace csms
