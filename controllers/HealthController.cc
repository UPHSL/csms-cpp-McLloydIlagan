#include "HealthController.h"
#include "models/AppInfo.h"

void HealthController::asyncHandleHttpRequest(
    const HttpRequestPtr&,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    Json::Value result;

    result["status"] = "ok";
    result["application"] = csms::APP_NAME;
    result["version"] = csms::APP_VERSION;

    auto response = HttpResponse::newHttpJsonResponse(result);

    callback(response);
}