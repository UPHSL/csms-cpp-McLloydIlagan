#include "DashboardService.h"

namespace csms {

DashboardService::DashboardService(
    ResidentRepository& residentRepo,
    ServiceRequestRepository& serviceRequestRepo)
    : residentRepo_(residentRepo),
      serviceRequestRepo_(serviceRequestRepo)
{}

DashboardData DashboardService::getDashboardData()
{
    DashboardData data;

    data.totalResidents    = residentRepo_.countAll();
    data.activeResidents   = residentRepo_.countByStatus("Active");
    data.inactiveResidents = residentRepo_.countByStatus("Inactive");

    data.totalServiceRequests = serviceRequestRepo_.countAll();
    data.pendingRequests      = serviceRequestRepo_.countByStatus("Pending");
    data.inProgressRequests   = serviceRequestRepo_.countByStatus("In Progress");
    data.completedRequests    = serviceRequestRepo_.countByStatus("Completed");
    data.cancelledRequests    = serviceRequestRepo_.countByStatus("Cancelled");
    data.openRequests         = data.pendingRequests + data.inProgressRequests;

    data.recentRequests = serviceRequestRepo_.findRecent(5);

    return data;
}

} // namespace csms
