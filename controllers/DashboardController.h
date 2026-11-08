#pragma once
#include <drogon/HttpSimpleController.h>
#include "database/Database.h"
#include "repositories/ResidentRepository.h"
#include "repositories/ServiceRequestRepository.h"
#include "services/DashboardService.h"

using namespace drogon;

class DashboardController : public HttpSimpleController<DashboardController>
{
public:
    DashboardController();

    void asyncHandleHttpRequest(
        const HttpRequestPtr& req,
        std::function<void(const HttpResponsePtr&)>&& callback) override;

    PATH_LIST_BEGIN
    PATH_ADD("/dashboard", Get);
    PATH_LIST_END

private:
    csms::Database db_;
    csms::ResidentRepository residentRepo_;
    csms::ServiceRequestRepository serviceRequestRepo_;
    csms::DashboardService dashboardService_;

    std::string htmlEscape(const std::string& s) const;
    std::string statusBadge(const std::string& status) const;
    std::string renderLayout(const std::string& title,
                             const std::string& body) const;
};
