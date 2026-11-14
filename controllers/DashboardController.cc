#include "DashboardController.h"
#include "models/AppInfo.h"

#include <sstream>
#include <string>

DashboardController::DashboardController()
    : db_("csms.db"),
      residentRepo_(db_),
      serviceRequestRepo_(db_),
      dashboardService_(residentRepo_, serviceRequestRepo_)
{}

std::string DashboardController::htmlEscape(const std::string& s) const
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

std::string DashboardController::statusBadge(const std::string& status) const
{
    std::string cls = "badge";
    if (status == "Pending")          cls += " badge-pending";
    else if (status == "In Progress") cls += " badge-inprogress";
    else if (status == "Completed")   cls += " badge-completed";
    else if (status == "Cancelled")   cls += " badge-cancelled";
    return "<span class=\"" + cls + "\">" + htmlEscape(status) + "</span>";
}

std::string DashboardController::renderLayout(const std::string& title,
                                               const std::string& body) const
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
.btn-sm{padding:4px 10px;font-size:0.8rem}
.metric-grid{display:flex;flex-wrap:wrap;gap:16px;margin-bottom:2rem}
.metric-card{background:#fff;border:1px solid #d9dde3;border-radius:10px;padding:20px 28px;min-width:140px;text-align:center;flex:1}
.metric-value{font-size:2.2rem;font-weight:700;color:#1a6cbb;line-height:1}
.metric-label{font-size:0.85rem;color:#555;margin-top:6px}
h2{font-size:1.3rem;margin-bottom:0.6em;margin-top:1.5rem}
.empty-state{text-align:center;padding:32px 0;color:#6c757d}
</style>
</head>
<body>
<div class="page-wrap">
)" + body + R"(
</div>
</body>
</html>)";
}

void DashboardController::asyncHandleHttpRequest(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const csms::DashboardData data = dashboardService_.getDashboardData();

    std::ostringstream body;

    body << R"(<nav><a href="/dashboard">Dashboard</a> &nbsp;|&nbsp; )"
         << R"(<a href="/residents">Residents</a> &nbsp;|&nbsp; )"
         << R"(<a href="/service-requests">Service Requests</a></nav>)"
         << " <span class=\"version\">v" << csms::APP_VERSION << "</span>"
         << R"(<h1>CSMS Dashboard</h1>)";

    // Resident Summary
    body << "<h2>Resident Summary</h2>"
         << R"(<div class="metric-grid">)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.totalResidents
         << R"(</div><div class="metric-label">Total Residents</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.activeResidents
         << R"(</div><div class="metric-label">Active Residents</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.inactiveResidents
         << R"(</div><div class="metric-label">Inactive Residents</div></div>)"
         << "</div>";

    // Service Request Summary
    body << "<h2>Service Request Summary</h2>"
         << R"(<div class="metric-grid">)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.totalServiceRequests
         << R"(</div><div class="metric-label">Total Requests</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.pendingRequests
         << R"(</div><div class="metric-label">Pending</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.inProgressRequests
         << R"(</div><div class="metric-label">In Progress</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.completedRequests
         << R"(</div><div class="metric-label">Completed</div></div>)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.cancelledRequests
         << R"(</div><div class="metric-label">Cancelled</div></div>)"
         << "</div>";

    // Open Service Request Summary
    body << "<h2>Open Service Requests</h2>"
         << R"(<div class="metric-grid">)"
         << R"(<div class="metric-card"><div class="metric-value">)" << data.openRequests
         << R"(</div><div class="metric-label">Open Requests (Pending + In Progress)</div></div>)"
         << "</div>";

    // Recent Service Requests (max 5)
    body << "<h2>Recent Service Requests</h2>";
    if (data.recentRequests.empty())
    {
        body << R"(<div class="empty-state">No service requests yet.</div>)";
    }
    else
    {
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

        for (const auto& sr : data.recentRequests)
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

        body << R"(</tbody></table>)";
    }

    // Navigation to major modules
    body << R"(<div style="margin-top:2rem">)"
         << R"(<a href="/residents" class="btn btn-primary" style="margin-right:8px">Manage Residents</a>)"
         << R"(<a href="/service-requests" class="btn btn-secondary">Manage Service Requests</a>)"
         << "</div>";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("CSMS Dashboard", body.str()));
    callback(resp);
}
