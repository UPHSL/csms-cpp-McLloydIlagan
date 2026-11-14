#include "ServiceRequestController.h"
#include "models/AppInfo.h"

#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Constructor
// ---------------------------------------------------------------------------

ServiceRequestController::ServiceRequestController()
    : db_("csms.db"),
      residentRepo_(db_),
      repo_(db_),
      validator_(),
      submissionService_(validator_, residentRepo_, repo_),
      statusService_(repo_),
      queryService_(repo_)
{
}

// ---------------------------------------------------------------------------
// HTML helpers
// ---------------------------------------------------------------------------

std::string ServiceRequestController::htmlEscape(const std::string& s)
{
    std::string out;
    out.reserve(s.size());
    for (char c : s)
    {
        switch (c)
        {
            case '&':  out += "&amp;";  break;
            case '<':  out += "&lt;";   break;
            case '>':  out += "&gt;";   break;
            case '"':  out += "&quot;"; break;
            case '\'': out += "&#39;";  break;
            default:   out += c;        break;
        }
    }
    return out;
}

std::string ServiceRequestController::friendlyError(const std::string& key)
{
    if (key == "residentId")    return "Resident ID is required.";
    if (key == "serviceType")   return "Service type is required.";
    if (key == "description")   return "Description is required.";
    if (key == "dateRequested") return "Date requested is required.";
    return key;
}

std::string ServiceRequestController::renderLayout(const std::string& title,
                                                    const std::string& body)
{
    return R"(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>)" + htmlEscape(title) + R"(</title>
<style>
*{box-sizing:border-box}
body{margin:0;font-family:Arial,Helvetica,sans-serif;background:#f4f5f7;color:#111}
.page-wrap{max-width:1100px;margin:0 auto;padding:24px 16px}
h1{font-size:1.8rem;margin-bottom:0.25em}
nav{margin-bottom:1.5rem}
nav a{color:#1a6cbb;text-decoration:none;font-weight:600}
nav a:hover{text-decoration:underline}
table{width:100%;border-collapse:collapse;background:#fff;border:1px solid #d9dde3;border-radius:8px;overflow:hidden}
th{background:#f0f2f5;text-align:left;padding:10px 12px;font-size:0.85rem;text-transform:uppercase;letter-spacing:0.04em}
td{padding:10px 12px;border-top:1px solid #e8eaed;font-size:0.9rem}
tr:hover td{background:#f9f9fb}
.badge{display:inline-block;padding:2px 10px;border-radius:12px;font-size:0.8rem;font-weight:700}
.badge-active{background:#d4edda;color:#155724}
.badge-inactive{background:#f8d7da;color:#721c24}
.badge-pending{background:#fff3cd;color:#856404}
.badge-inprogress{background:#cce5ff;color:#004085}
.badge-completed{background:#d4edda;color:#155724}
.badge-cancelled{background:#f8d7da;color:#721c24}
.btn{display:inline-block;padding:7px 16px;border-radius:6px;border:none;cursor:pointer;font-size:0.85rem;text-decoration:none;font-weight:600}
.btn-primary{background:#1a6cbb;color:#fff}.btn-primary:hover{background:#155a9a}
.btn-secondary{background:#6c757d;color:#fff}.btn-secondary:hover{background:#5a6268}
.btn-danger{background:#dc3545;color:#fff}.btn-danger:hover{background:#c82333}
.btn-sm{padding:4px 10px;font-size:0.8rem}
form.inline{display:inline}
.form-group{margin-bottom:1rem}
label{display:block;font-weight:600;margin-bottom:4px;font-size:0.9rem}
input[type=text],input[type=email],input[type=date],select,textarea{width:100%;padding:8px 10px;border:1px solid #ced4da;border-radius:4px;font-size:0.95rem}
textarea{min-height:80px;resize:vertical}
input.error-field,select.error-field,textarea.error-field{border-color:#dc3545}
.error-list{background:#f8d7da;border:1px solid #f5c2c7;border-radius:6px;padding:10px 16px;margin-bottom:1rem;color:#721c24;font-size:0.9rem}
.error-list li{margin:2px 0}
.error-banner{background:#f8d7da;border:1px solid #f5c2c7;border-radius:6px;padding:10px 16px;margin-bottom:1rem;color:#721c24;font-size:0.9rem}
.search-form{display:flex;gap:8px;margin-bottom:1.25rem;align-items:center;flex-wrap:wrap}
.search-form input{flex:1;max-width:280px;padding:7px 10px;border:1px solid #ced4da;border-radius:4px}
.search-form select{padding:7px 10px;border:1px solid #ced4da;border-radius:4px}
.toolbar{display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem;flex-wrap:wrap;gap:8px}
.card{background:#fff;border:1px solid #d9dde3;border-radius:10px;padding:28px 32px;max-width:560px}
.not-found{text-align:center;padding:48px 0}
.not-found h2{font-size:1.5rem;color:#6c757d}
.detail-card{background:#fff;border:1px solid #d9dde3;border-radius:10px;padding:28px 32px;max-width:720px}
.detail-row{display:flex;margin-bottom:0.75rem;gap:12px}
.detail-label{font-weight:600;min-width:160px;color:#555;font-size:0.9rem}
.detail-value{font-size:0.9rem}
.action-row{display:flex;gap:8px;margin-top:1.25rem;flex-wrap:wrap}
</style>
</head>
<body>
<div class="page-wrap">
)" + body + R"(
</div>
</body>
</html>)";
}

std::string ServiceRequestController::statusBadge(const std::string& status)
{
    std::string cls = "badge";
    if (status == "Pending")          cls += " badge-pending";
    else if (status == "In Progress") cls += " badge-inprogress";
    else if (status == "Completed")   cls += " badge-completed";
    else if (status == "Cancelled")   cls += " badge-cancelled";
    return "<span class=\"" + cls + "\">" + htmlEscape(status) + "</span>";
}

// ---------------------------------------------------------------------------
// GET /service-requests — list or search service requests
// ---------------------------------------------------------------------------

void ServiceRequestController::index(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string search = req->getParameter("search");
    const std::string status = req->getParameter("status");

    std::vector<csms::ServiceRequest> requests;
    std::string errorMsg;

    if (search.empty() && status.empty())
    {
        requests = queryService_.listServiceRequests();
    }
    else
    {
        try
        {
            requests = queryService_.searchServiceRequests(search, status);
        }
        catch (const std::invalid_argument& e)
        {
            errorMsg = e.what();
            requests = queryService_.listServiceRequests();
        }
    }

    std::ostringstream body;

    body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
         << R"(<h1>Service Request Management</h1>)";

    if (!errorMsg.empty())
    {
        body << R"(<div class="error-banner">)" << htmlEscape(errorMsg) << R"(</div>)";
    }

    // Toolbar: search form + submit button
    body << R"(<div class="toolbar">)"
         << R"(<form class="search-form" method="get" action="/service-requests">)"
         << R"(<input type="text" name="search" placeholder="Search service requests&hellip;" value=")"
         << htmlEscape(search) << R"(">)"
         << R"(<select name="status">)"
         << R"(<option value="")"   << (status.empty()              ? " selected" : "") << R"(>All Statuses</option>)"
         << R"(<option value="Pending")"      << (status == "Pending"      ? " selected" : "") << R"(>Pending</option>)"
         << R"(<option value="In Progress")"  << (status == "In Progress"  ? " selected" : "") << R"(>In Progress</option>)"
         << R"(<option value="Completed")"    << (status == "Completed"    ? " selected" : "") << R"(>Completed</option>)"
         << R"(<option value="Cancelled")"    << (status == "Cancelled"    ? " selected" : "") << R"(>Cancelled</option>)"
         << R"(</select>)"
         << R"(<button type="submit" class="btn btn-secondary btn-sm">Search</button>)"
         << R"(<a href="/service-requests" class="btn btn-secondary btn-sm">Clear</a>)"
         << R"(</form>)"
         << R"(<a href="/service-requests/new" class="btn btn-primary">+ Submit Service Request</a>)"
         << R"(</div>)";

    if (!search.empty() || !status.empty())
    {
        body << "<p>" << requests.size() << " result(s) found.</p>";
    }

    body << R"(<table>)"
         << R"(<thead><tr>)"
         << R"(<th>ID</th>)"
         << R"(<th>Resident ID</th>)"
         << R"(<th>Service Type</th>)"
         << R"(<th>Date Requested</th>)"
         << R"(<th>Status</th>)"
         << R"(<th>Actions</th>)"
         << R"(</tr></thead>)"
         << R"(<tbody>)";

    if (requests.empty())
    {
        body << R"(<tr><td colspan="6" style="text-align:center;color:#6c757d;padding:24px">)"
             << "No Service Requests found."
             << R"(</td></tr>)";
    }
    else
    {
        for (const auto& sr : requests)
        {
            const int srId = sr.getId().has_value() ? sr.getId().value() : 0;
            body << "<tr>"
                 << "<td>" << srId << "</td>"
                 << "<td>" << sr.getResidentId() << "</td>"
                 << "<td>" << htmlEscape(sr.getServiceType()) << "</td>"
                 << "<td>" << htmlEscape(sr.getDateRequested()) << "</td>"
                 << "<td>" << statusBadge(sr.getStatus()) << "</td>"
                 << "<td>"
                 << "<a href=\"/service-requests/" << srId << "\" "
                 << "class=\"btn btn-secondary btn-sm\">View</a>"
                 << "</td>"
                 << "</tr>";
        }
    }

    body << R"(</tbody></table>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Service Request Management | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// GET /service-requests/new — blank submission form
// ---------------------------------------------------------------------------

void ServiceRequestController::showNew(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::ostringstream body;

    body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
         << R"(<h1>Submit Service Request</h1>)"
         << R"(<div class="card">)"
         << R"(<form method="post" action="/service-requests">)"
         << R"(<div class="form-group"><label for="residentId">Resident ID</label>)"
         << R"(<input type="text" id="residentId" name="residentId" value=""></div>)"
         << R"(<div class="form-group"><label for="serviceType">Service Type</label>)"
         << R"(<input type="text" id="serviceType" name="serviceType" value=""></div>)"
         << R"(<div class="form-group"><label for="description">Description</label>)"
         << R"(<textarea id="description" name="description"></textarea></div>)"
         << R"(<div class="form-group"><label for="dateRequested">Date Requested</label>)"
         << R"(<input type="date" id="dateRequested" name="dateRequested" value=""></div>)"
         << R"(<button type="submit" class="btn btn-primary">Submit</button> )"
         << R"(<a href="/service-requests" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Submit Service Request | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /service-requests — process submission
// ---------------------------------------------------------------------------

void ServiceRequestController::create(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string residentIdStr = req->getParameter("residentId");
    const std::string serviceType   = req->getParameter("serviceType");
    const std::string description   = req->getParameter("description");
    const std::string dateRequested = req->getParameter("dateRequested");

    int residentId = 0;
    if (!residentIdStr.empty())
    {
        try { residentId = std::stoi(residentIdStr); }
        catch (const std::exception&) { residentId = 0; }
    }

    csms::ServiceRequest request(residentId, serviceType, description, dateRequested);
    const csms::ServiceRequestSubmissionResult result =
        submissionService_.submitRequest(request);

    if (result.success)
    {
        auto resp = HttpResponse::newRedirectionResponse("/service-requests");
        callback(resp);
        return;
    }

    // Build error re-render
    std::ostringstream body;
    body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
         << R"(<h1>Submit Service Request</h1>)";

    if (result.residentNotFound)
    {
        body << R"(<div class="error-banner">Resident not found.</div>)";
    }
    else if (result.residentInactive)
    {
        body << R"(<div class="error-banner">Inactive Residents cannot submit new Service Requests.</div>)";
    }
    else if (!result.errors.empty())
    {
        body << R"(<div class="error-list"><strong>Please correct the following:</strong><ul>)";
        for (const auto& e : result.errors)
            body << "<li>" << htmlEscape(friendlyError(e)) << "</li>";
        body << R"(</ul></div>)";
    }

    const auto& errors = result.errors;
    auto hasError = [&](const std::string& field) -> bool {
        for (const auto& e : errors)
            if (e == field) return true;
        return false;
    };

    body << R"(<div class="card">)"
         << R"(<form method="post" action="/service-requests">)"
         << R"(<div class="form-group"><label for="residentId">Resident ID</label>)"
         << R"(<input type="text" id="residentId" name="residentId" value=")"
         << htmlEscape(residentIdStr) << "\""
         << (hasError("residentId") ? R"( class="error-field")" : "")
         << R"(></div>)"
         << R"(<div class="form-group"><label for="serviceType">Service Type</label>)"
         << R"(<input type="text" id="serviceType" name="serviceType" value=")"
         << htmlEscape(serviceType) << "\""
         << (hasError("serviceType") ? R"( class="error-field")" : "")
         << R"(></div>)"
         << R"(<div class="form-group"><label for="description">Description</label>)"
         << R"(<textarea id="description" name="description")"
         << (hasError("description") ? R"( class="error-field")" : "")
         << ">" << htmlEscape(description) << R"(</textarea></div>)"
         << R"(<div class="form-group"><label for="dateRequested">Date Requested</label>)"
         << R"(<input type="date" id="dateRequested" name="dateRequested" value=")"
         << htmlEscape(dateRequested) << "\""
         << (hasError("dateRequested") ? R"( class="error-field")" : "")
         << R"(></div>)"
         << R"(<button type="submit" class="btn btn-primary">Submit</button> )"
         << R"(<a href="/service-requests" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(HttpStatusCode::k422UnprocessableEntity);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Submit Service Request | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// GET /service-requests/{id} — show detail
// ---------------------------------------------------------------------------

void ServiceRequestController::show(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const auto found = queryService_.viewServiceRequest(id);

    if (!found.has_value())
    {
        std::ostringstream body;
        body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
             << R"(<div class="not-found">)"
             << "<h2>Service Request Not Found</h2>"
             << "<p>No service request with ID " << id << " exists.</p>"
             << R"(<a href="/service-requests" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Service Request Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    const csms::ServiceRequest& sr = found.value();
    const std::string& currentStatus = sr.getStatus();

    std::ostringstream body;
    body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
         << "<h1>Service Request #" << id << "</h1>"
         << R"(<div class="detail-card">)"
         << R"(<div class="detail-row"><span class="detail-label">ID</span><span class="detail-value">)"
         << id << R"(</span></div>)"
         << R"(<div class="detail-row"><span class="detail-label">Resident ID</span><span class="detail-value">)"
         << sr.getResidentId() << R"(</span></div>)"
         << R"(<div class="detail-row"><span class="detail-label">Service Type</span><span class="detail-value">)"
         << htmlEscape(sr.getServiceType()) << R"(</span></div>)"
         << R"(<div class="detail-row"><span class="detail-label">Description</span><span class="detail-value">)"
         << htmlEscape(sr.getDescription()) << R"(</span></div>)"
         << R"(<div class="detail-row"><span class="detail-label">Date Requested</span><span class="detail-value">)"
         << htmlEscape(sr.getDateRequested()) << R"(</span></div>)"
         << R"(<div class="detail-row"><span class="detail-label">Status</span><span class="detail-value">)"
         << statusBadge(currentStatus) << R"(</span></div>)";

    // Workflow action buttons
    if (currentStatus == "Pending")
    {
        body << "<div class=\"action-row\">"
             << "<form method=\"post\" action=\"/service-requests/" << id << "/process\">"
             << "<button type=\"submit\" class=\"btn btn-primary\">Start Processing</button>"
             << "</form>"
             << "<form method=\"post\" action=\"/service-requests/" << id << "/cancel\">"
             << "<button type=\"submit\" class=\"btn btn-danger\" "
             << "onclick=\"return confirm('Cancel this service request?')\">Cancel</button>"
             << "</form>"
             << "</div>";
    }
    else if (currentStatus == "In Progress")
    {
        body << "<div class=\"action-row\">"
             << "<form method=\"post\" action=\"/service-requests/" << id << "/complete\">"
             << "<button type=\"submit\" class=\"btn btn-primary\">Complete</button>"
             << "</form>"
             << "<form method=\"post\" action=\"/service-requests/" << id << "/cancel\">"
             << "<button type=\"submit\" class=\"btn btn-danger\" "
             << "onclick=\"return confirm('Cancel this service request?')\">Cancel</button>"
             << "</form>"
             << "</div>";
    }
    else if (currentStatus == "Completed")
    {
        body << "<p>This request has been completed.</p>";
    }
    else if (currentStatus == "Cancelled")
    {
        body << "<p>This request has been cancelled.</p>";
    }

    body << R"(</div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Service Request #" + std::to_string(id) + " | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /service-requests/{id}/process
// ---------------------------------------------------------------------------

void ServiceRequestController::process(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const csms::ServiceRequestStatusResult result =
        statusService_.manageStatus(id, "In Progress");

    if (result.notFound)
    {
        std::ostringstream body;
        body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
             << R"(<div class="not-found">)"
             << "<h2>Service Request Not Found</h2>"
             << "<p>No service request with ID " << id << " exists.</p>"
             << R"(<a href="/service-requests" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Service Request Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    // success, invalidTransition, unsupportedStatus all redirect to show
    auto resp = HttpResponse::newRedirectionResponse(
        "/service-requests/" + std::to_string(id));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /service-requests/{id}/complete
// ---------------------------------------------------------------------------

void ServiceRequestController::complete(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const csms::ServiceRequestStatusResult result =
        statusService_.manageStatus(id, "Completed");

    if (result.notFound)
    {
        std::ostringstream body;
        body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
             << R"(<div class="not-found">)"
             << "<h2>Service Request Not Found</h2>"
             << "<p>No service request with ID " << id << " exists.</p>"
             << R"(<a href="/service-requests" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Service Request Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    auto resp = HttpResponse::newRedirectionResponse(
        "/service-requests/" + std::to_string(id));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /service-requests/{id}/cancel
// ---------------------------------------------------------------------------

void ServiceRequestController::cancel(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const csms::ServiceRequestStatusResult result =
        statusService_.manageStatus(id, "Cancelled");

    if (result.notFound)
    {
        std::ostringstream body;
        body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; <a href="/residents">Residents</a> &nbsp;|&nbsp; <a href="/service-requests">Service Requests</a></nav>)"
     << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
             << R"(<div class="not-found">)"
             << "<h2>Service Request Not Found</h2>"
             << "<p>No service request with ID " << id << " exists.</p>"
             << R"(<a href="/service-requests" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Service Request Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    auto resp = HttpResponse::newRedirectionResponse(
        "/service-requests/" + std::to_string(id));
    callback(resp);
}
