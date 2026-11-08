#pragma once
#include <vector>
#include "../models/ServiceRequest.h"
#include "../repositories/ResidentRepository.h"
#include "../repositories/ServiceRequestRepository.h"

namespace csms {

struct DashboardData {
    // Resident metrics
    int totalResidents{0};
    int activeResidents{0};
    int inactiveResidents{0};

    // Service Request metrics
    int totalServiceRequests{0};
    int pendingRequests{0};
    int inProgressRequests{0};
    int completedRequests{0};
    int cancelledRequests{0};
    int openRequests{0};  // derived: pending + inProgress

    // Recent Service Requests (max 5)
    std::vector<ServiceRequest> recentRequests;
};

class DashboardService {
public:
    DashboardService(ResidentRepository& residentRepo,
                     ServiceRequestRepository& serviceRequestRepo);

    DashboardData getDashboardData();

private:
    ResidentRepository& residentRepo_;
    ServiceRequestRepository& serviceRequestRepo_;
};

} // namespace csms
