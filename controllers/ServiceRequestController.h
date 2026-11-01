#pragma once

#include <drogon/HttpController.h>

#include "database/Database.h"
#include "models/ServiceRequestValidator.h"
#include "repositories/ResidentRepository.h"
#include "repositories/ServiceRequestRepository.h"
#include "services/ServiceRequestSubmissionService.h"
#include "services/ServiceRequestStatusService.h"
#include "services/ServiceRequestQueryService.h"

using namespace drogon;

class ServiceRequestController : public HttpController<ServiceRequestController>
{
public:
    ServiceRequestController();

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ServiceRequestController::index,    "/service-requests",               Get);
    ADD_METHOD_TO(ServiceRequestController::showNew,  "/service-requests/new",           Get);
    ADD_METHOD_TO(ServiceRequestController::create,   "/service-requests",               Post);
    ADD_METHOD_TO(ServiceRequestController::show,     "/service-requests/{id}",          Get);
    ADD_METHOD_TO(ServiceRequestController::process,  "/service-requests/{id}/process",  Post);
    ADD_METHOD_TO(ServiceRequestController::complete, "/service-requests/{id}/complete", Post);
    ADD_METHOD_TO(ServiceRequestController::cancel,   "/service-requests/{id}/cancel",   Post);
    METHOD_LIST_END

    void index(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

    void showNew(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);

    void create(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void show(const HttpRequestPtr& req,
              std::function<void(const HttpResponsePtr&)>&& callback,
              int id);

    void process(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback,
                 int id);

    void complete(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback,
                  int id);

    void cancel(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                int id);

private:
    csms::Database                         db_;
    csms::ResidentRepository               residentRepo_;
    csms::ServiceRequestRepository         repo_;
    csms::ServiceRequestValidator          validator_;
    csms::ServiceRequestSubmissionService  submissionService_;
    csms::ServiceRequestStatusService      statusService_;
    csms::ServiceRequestQueryService       queryService_;

    static std::string htmlEscape(const std::string& s);
    static std::string renderLayout(const std::string& title,
                                    const std::string& body);
    static std::string friendlyError(const std::string& key);
    static std::string statusBadge(const std::string& status);
};
