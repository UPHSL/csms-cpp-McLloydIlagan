#include "ResidentController.h"

#include <sstream>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// Constructor — opens the shared database; member services hold references.
// Member declaration order: db_, repo_, validator_, registrationService_,
// queryService_, updateService_, deactivationService_
// ---------------------------------------------------------------------------

ResidentController::ResidentController()
    : db_("csms.db"),
      repo_(db_),
      validator_(),
      registrationService_(validator_, repo_),
      queryService_(repo_),
      updateService_(validator_, repo_),
      deactivationService_(repo_)
{
}

// ---------------------------------------------------------------------------
// HTML helpers
// ---------------------------------------------------------------------------

std::string ResidentController::htmlEscape(const std::string& s)
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

std::string ResidentController::friendlyError(const std::string& key)
{
    if (key == "firstName")     return "First name is required.";
    if (key == "lastName")      return "Last name is required.";
    if (key == "address")       return "Address is required.";
    if (key == "contactNumber") return "Contact number must be 11 digits starting with 09.";
    if (key == "email")         return "A valid email address is required.";
    if (key == "status")        return "Status must be Active or Inactive.";
    return key; // fallback: return the raw key if unknown
}

std::string ResidentController::renderLayout(const std::string& title,
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
.btn{display:inline-block;padding:7px 16px;border-radius:6px;border:none;cursor:pointer;font-size:0.85rem;text-decoration:none;font-weight:600}
.btn-primary{background:#1a6cbb;color:#fff}.btn-primary:hover{background:#155a9a}
.btn-secondary{background:#6c757d;color:#fff}.btn-secondary:hover{background:#5a6268}
.btn-danger{background:#dc3545;color:#fff}.btn-danger:hover{background:#c82333}
.btn-sm{padding:4px 10px;font-size:0.8rem}
form.inline{display:inline}
.form-group{margin-bottom:1rem}
label{display:block;font-weight:600;margin-bottom:4px;font-size:0.9rem}
input[type=text],input[type=email]{width:100%;padding:8px 10px;border:1px solid #ced4da;border-radius:4px;font-size:0.95rem}
input.error-field{border-color:#dc3545}
.error-list{background:#f8d7da;border:1px solid #f5c2c7;border-radius:6px;padding:10px 16px;margin-bottom:1rem;color:#721c24;font-size:0.9rem}
.error-list li{margin:2px 0}
.search-form{display:flex;gap:8px;margin-bottom:1.25rem;align-items:center}
.search-form input{flex:1;max-width:320px;padding:7px 10px;border:1px solid #ced4da;border-radius:4px}
.toolbar{display:flex;justify-content:space-between;align-items:center;margin-bottom:1rem;flex-wrap:wrap;gap:8px}
.card{background:#fff;border:1px solid #d9dde3;border-radius:10px;padding:28px 32px;max-width:560px}
.not-found{text-align:center;padding:48px 0}
.not-found h2{font-size:1.5rem;color:#6c757d}
</style>
</head>
<body>
<div class="page-wrap">
)" + body + R"(
</div>
</body>
</html>)";
}

// ---------------------------------------------------------------------------
// GET /residents — list or search residents
// ---------------------------------------------------------------------------

void ResidentController::index(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string search = req->getParameter("search");

    std::vector<csms::Resident> residents;
    if (search.empty())
        residents = queryService_.listResidents();
    else
        residents = queryService_.searchResidents(search);

    std::ostringstream body;

    body << R"(<nav><a href="/">&#8592; Home</a></nav>)"
         << R"(<h1>Resident Management</h1>)";

    // Toolbar: search form + register button
    body << R"(<div class="toolbar">)"
         << R"(<form class="search-form" method="get" action="/residents">)"
         << R"(<input type="text" name="search" placeholder="Search by name&hellip;" value=")"
         << htmlEscape(search)
         << R"(">)"
         << R"(<button type="submit" class="btn btn-secondary btn-sm">Search</button>)"
         << R"(<a href="/residents" class="btn btn-secondary btn-sm">Clear</a>)"
         << R"(</form>)"
         << R"(<a href="/residents/new" class="btn btn-primary">+ Register Resident</a>)"
         << R"(</div>)";

    if (!search.empty())
    {
        body << "<p>Search results for <strong>&ldquo;"
             << htmlEscape(search)
             << "&rdquo;</strong>: " << residents.size() << " found.</p>";
    }

    body << R"(<table>)"
         << R"(<thead><tr>)"
         << R"(<th>ID</th>)"
         << R"(<th>Name</th>)"
         << R"(<th>Address</th>)"
         << R"(<th>Contact Number</th>)"
         << R"(<th>Email</th>)"
         << R"(<th>Status</th>)"
         << R"(<th>Actions</th>)"
         << R"(</tr></thead>)"
         << R"(<tbody>)";

    if (residents.empty())
    {
        body << R"(<tr><td colspan="7" style="text-align:center;color:#6c757d;padding:24px">)"
             << "No residents found."
             << R"(</td></tr>)";
    }
    else
    {
        for (const auto& r : residents)
        {
            const bool active = (r.getStatus() == "Active");
            body << "<tr>"
                 << "<td>" << r.getId() << "</td>"
                 << "<td>" << htmlEscape(r.getFirstName()) << " " << htmlEscape(r.getLastName()) << "</td>"
                 << "<td>" << htmlEscape(r.getAddress()) << "</td>"
                 << "<td>" << htmlEscape(r.getContactNumber()) << "</td>"
                 << "<td>" << htmlEscape(r.getEmail()) << "</td>"
                 << "<td><span class=\"badge " << (active ? "badge-active" : "badge-inactive") << "\">"
                 << htmlEscape(r.getStatus()) << "</span></td>"
                 << "<td>";

            // Edit — always available
            body << "<a href=\"/residents/" << r.getId() << "/edit\" "
                 << "class=\"btn btn-secondary btn-sm\">Edit</a> ";

            // Deactivate — only for Active residents
            if (active)
            {
                body << "<form class=\"inline\" method=\"post\" "
                     << "action=\"/residents/" << r.getId() << "/deactivate\" "
                     << "onsubmit=\"return confirm('Deactivate this resident?')\">"
                     << "<button type=\"submit\" class=\"btn btn-danger btn-sm\">Deactivate</button>"
                     << "</form>";
            }

            body << "</td></tr>";
        }
    }

    body << R"(</tbody></table>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Resident Management | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// GET /residents/new — blank registration form
// ---------------------------------------------------------------------------

void ResidentController::showNew(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    std::ostringstream body;

    body << R"(<nav><a href="/residents">&#8592; Resident Management</a></nav>)"
         << R"(<h1>Register Resident</h1>)"
         << R"(<div class="card">)"
         << R"(<form method="post" action="/residents">)"
         << R"(<div class="form-group"><label for="firstName">First Name</label>)"
         << R"(<input type="text" id="firstName" name="firstName" value=""></div>)"
         << R"(<div class="form-group"><label for="lastName">Last Name</label>)"
         << R"(<input type="text" id="lastName" name="lastName" value=""></div>)"
         << R"(<div class="form-group"><label for="address">Address</label>)"
         << R"(<input type="text" id="address" name="address" value=""></div>)"
         << R"(<div class="form-group"><label for="contactNumber">Contact Number</label>)"
         << R"(<input type="text" id="contactNumber" name="contactNumber" value=""></div>)"
         << R"(<div class="form-group"><label for="email">Email</label>)"
         << R"(<input type="email" id="email" name="email" value=""></div>)"
         << R"(<button type="submit" class="btn btn-primary">Register</button> )"
         << R"(<a href="/residents" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Register Resident | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /residents — process registration
// ---------------------------------------------------------------------------

void ResidentController::create(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback)
{
    const std::string firstName     = req->getParameter("firstName");
    const std::string lastName      = req->getParameter("lastName");
    const std::string address       = req->getParameter("address");
    const std::string contactNumber = req->getParameter("contactNumber");
    const std::string email         = req->getParameter("email");

    csms::Resident candidate(firstName, lastName, address, contactNumber, email);
    const csms::ResidentRegistrationResult result =
        registrationService_.registerResident(candidate);

    if (result.success)
    {
        // PRG: redirect to the listing page
        auto resp = HttpResponse::newRedirectionResponse("/residents");
        callback(resp);
        return;
    }

    // Validation failed — re-render form with errors and preserved input
    const std::vector<std::string>& errors = result.errors;

    auto hasError = [&](const std::string& field) -> bool {
        for (const auto& e : errors)
            if (e == field) return true;
        return false;
    };

    std::ostringstream body;

    body << R"(<nav><a href="/residents">&#8592; Resident Management</a></nav>)"
         << R"(<h1>Register Resident</h1>)";

    body << R"(<div class="error-list"><strong>Please correct the following:</strong><ul>)";
    for (const auto& e : errors)
        body << "<li>" << htmlEscape(friendlyError(e)) << "</li>";
    body << R"(</ul></div>)";

    body << R"(<div class="card">)"
         << R"(<form method="post" action="/residents">)";

    auto renderField = [&](const std::string& fid, const std::string& lbl,
                           const std::string& type, const std::string& value) {
        body << R"(<div class="form-group"><label for=")" << fid << "\">" << lbl << "</label>"
             << R"(<input type=")" << type << R"(" id=")" << fid << R"(" name=")" << fid
             << "\" value=\"" << htmlEscape(value) << "\""
             << (hasError(fid) ? R"( class="error-field")" : "")
             << "></div>";
    };

    renderField("firstName",     "First Name",     "text",  firstName);
    renderField("lastName",      "Last Name",      "text",  lastName);
    renderField("address",       "Address",        "text",  address);
    renderField("contactNumber", "Contact Number", "text",  contactNumber);
    renderField("email",         "Email",          "email", email);

    body << R"(<button type="submit" class="btn btn-primary">Register</button> )"
         << R"(<a href="/residents" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(HttpStatusCode::k422UnprocessableEntity);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Register Resident | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// GET /residents/{id}/edit — edit form pre-filled with existing data
// ---------------------------------------------------------------------------

void ResidentController::showEdit(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const auto found = repo_.findById(id);

    if (!found.has_value())
    {
        std::ostringstream body;
        body << R"(<div class="not-found">)"
             << "<h2>Resident Not Found</h2>"
             << "<p>No resident with ID " << id << " exists.</p>"
             << R"(<a href="/residents" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Resident Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    const csms::Resident& r = found.value();

    std::ostringstream body;
    body << R"(<nav><a href="/residents">&#8592; Resident Management</a></nav>)"
         << "<h1>Edit Resident</h1>"
         << R"(<div class="card">)"
         << R"(<form method="post" action="/residents/)" << id << R"(">)";

    auto renderField = [&](const std::string& fid, const std::string& lbl,
                           const std::string& type, const std::string& value) {
        body << R"(<div class="form-group"><label for=")" << fid << "\">" << lbl << "</label>"
             << R"(<input type=")" << type << R"(" id=")" << fid << R"(" name=")" << fid
             << "\" value=\"" << htmlEscape(value) << "\"></div>";
    };

    renderField("firstName",     "First Name",     "text",  r.getFirstName());
    renderField("lastName",      "Last Name",      "text",  r.getLastName());
    renderField("address",       "Address",        "text",  r.getAddress());
    renderField("contactNumber", "Contact Number", "text",  r.getContactNumber());
    renderField("email",         "Email",          "email", r.getEmail());

    // ID and Status are read-only
    body << R"(<div class="form-group"><label>ID</label>)"
         << "<p>" << r.getId() << "</p></div>";

    body << R"(<div class="form-group"><label>Status</label>)"
         << "<p><span class=\"badge "
         << (r.getStatus() == "Active" ? "badge-active" : "badge-inactive")
         << "\">" << htmlEscape(r.getStatus()) << "</span></p></div>";

    body << R"(<button type="submit" class="btn btn-primary">Update</button> )"
         << R"(<a href="/residents" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Edit Resident | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /residents/{id} — process update
// ---------------------------------------------------------------------------

void ResidentController::update(
    const HttpRequestPtr& req,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const std::string firstName     = req->getParameter("firstName");
    const std::string lastName      = req->getParameter("lastName");
    const std::string address       = req->getParameter("address");
    const std::string contactNumber = req->getParameter("contactNumber");
    const std::string email         = req->getParameter("email");

    csms::Resident proposed(firstName, lastName, address, contactNumber, email);
    const csms::ResidentUpdateResult result =
        updateService_.updateResident(id, proposed);

    if (result.success)
    {
        auto resp = HttpResponse::newRedirectionResponse("/residents");
        callback(resp);
        return;
    }

    if (result.notFound)
    {
        std::ostringstream body;
        body << R"(<div class="not-found">)"
             << "<h2>Resident Not Found</h2>"
             << "<p>No resident with ID " << id << " exists.</p>"
             << R"(<a href="/residents" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Resident Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    // Validation failure — re-render edit form with errors
    const std::vector<std::string>& errors = result.errors;

    auto hasError = [&](const std::string& field) -> bool {
        for (const auto& e : errors)
            if (e == field) return true;
        return false;
    };

    std::ostringstream body;
    body << R"(<nav><a href="/residents">&#8592; Resident Management</a></nav>)"
         << "<h1>Edit Resident</h1>";

    body << R"(<div class="error-list"><strong>Please correct the following:</strong><ul>)";
    for (const auto& e : errors)
        body << "<li>" << htmlEscape(friendlyError(e)) << "</li>";
    body << R"(</ul></div>)";

    body << R"(<div class="card">)"
         << R"(<form method="post" action="/residents/)" << id << R"(">)";

    auto renderField = [&](const std::string& fid, const std::string& lbl,
                           const std::string& type, const std::string& value) {
        body << R"(<div class="form-group"><label for=")" << fid << "\">" << lbl << "</label>"
             << R"(<input type=")" << type << R"(" id=")" << fid << R"(" name=")" << fid
             << "\" value=\"" << htmlEscape(value) << "\""
             << (hasError(fid) ? R"( class="error-field")" : "")
             << "></div>";
    };

    renderField("firstName",     "First Name",     "text",  firstName);
    renderField("lastName",      "Last Name",      "text",  lastName);
    renderField("address",       "Address",        "text",  address);
    renderField("contactNumber", "Contact Number", "text",  contactNumber);
    renderField("email",         "Email",          "email", email);

    body << R"(<button type="submit" class="btn btn-primary">Update</button> )"
         << R"(<a href="/residents" class="btn btn-secondary">Cancel</a>)"
         << R"(</form></div>)";

    auto resp = HttpResponse::newHttpResponse();
    resp->setStatusCode(HttpStatusCode::k422UnprocessableEntity);
    resp->setContentTypeCode(CT_TEXT_HTML);
    resp->setBody(renderLayout("Edit Resident | CSMS", body.str()));
    callback(resp);
}

// ---------------------------------------------------------------------------
// POST /residents/{id}/deactivate — deactivate a resident
// ---------------------------------------------------------------------------

void ResidentController::deactivate(
    const HttpRequestPtr& /*req*/,
    std::function<void(const HttpResponsePtr&)>&& callback,
    int id)
{
    const csms::ResidentDeactivationResult result =
        deactivationService_.deactivateResident(id);

    if (result.notFound)
    {
        std::ostringstream body;
        body << R"(<div class="not-found">)"
             << "<h2>Resident Not Found</h2>"
             << "<p>No resident with ID " << id << " exists.</p>"
             << R"(<a href="/residents" class="btn btn-secondary">Back to List</a>)"
             << R"(</div>)";

        auto resp = HttpResponse::newHttpResponse();
        resp->setStatusCode(HttpStatusCode::k404NotFound);
        resp->setContentTypeCode(CT_TEXT_HTML);
        resp->setBody(renderLayout("Resident Not Found | CSMS", body.str()));
        callback(resp);
        return;
    }

    // Newly deactivated or already inactive — redirect to listing
    auto resp = HttpResponse::newRedirectionResponse("/residents");
    callback(resp);
}
