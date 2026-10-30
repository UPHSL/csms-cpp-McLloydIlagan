#pragma once

#include <drogon/HttpController.h>

#include "database/Database.h"
#include "models/ResidentValidator.h"
#include "repositories/ResidentRepository.h"
#include "services/ResidentRegistrationService.h"
#include "services/ResidentQueryService.h"
#include "services/ResidentUpdateService.h"
#include "services/ResidentDeactivationService.h"

using namespace drogon;

class ResidentController : public HttpController<ResidentController>
{
public:
    ResidentController();

    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ResidentController::index,      "/residents",                 Get);
    ADD_METHOD_TO(ResidentController::showNew,    "/residents/new",             Get);
    ADD_METHOD_TO(ResidentController::create,     "/residents",                 Post);
    ADD_METHOD_TO(ResidentController::showEdit,   "/residents/{id}/edit",       Get);
    ADD_METHOD_TO(ResidentController::update,     "/residents/{id}",            Post);
    ADD_METHOD_TO(ResidentController::deactivate, "/residents/{id}/deactivate", Post);
    METHOD_LIST_END

    void index(const HttpRequestPtr& req,
               std::function<void(const HttpResponsePtr&)>&& callback);

    void showNew(const HttpRequestPtr& req,
                 std::function<void(const HttpResponsePtr&)>&& callback);

    void create(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback);

    void showEdit(const HttpRequestPtr& req,
                  std::function<void(const HttpResponsePtr&)>&& callback,
                  int id);

    void update(const HttpRequestPtr& req,
                std::function<void(const HttpResponsePtr&)>&& callback,
                int id);

    void deactivate(const HttpRequestPtr& req,
                    std::function<void(const HttpResponsePtr&)>&& callback,
                    int id);

private:
    csms::Database                      db_;
    csms::ResidentRepository            repo_;
    csms::ResidentValidator             validator_;
    csms::ResidentRegistrationService   registrationService_;
    csms::ResidentQueryService          queryService_;
    csms::ResidentUpdateService         updateService_;
    csms::ResidentDeactivationService   deactivationService_;

    static std::string htmlEscape(const std::string& s);
    static std::string renderLayout(const std::string& title,
                                    const std::string& body);
    static std::string friendlyError(const std::string& key);
};
