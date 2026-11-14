# Final Development Checkpoint

## 1. Developer Information
- Name: Lloyd Ilagan
- GitHub Username: McLloydIlagan
- Technology Stack: C++ with Drogon
- T16 Branch: feature/t16-final-release
- Final Release Version: 1.0.0

## 2. My Final Release Implementation

The application release version is established in a single header file, models/AppInfo.h, which defines three constexpr constants: APP_NAME, APP_SHORT_NAME, and APP_VERSION. This location fits the existing architecture because the project already uses a models/ directory for domain definitions and all build targets include that path in their include directories. The HealthController includes AppInfo.h and uses csms::APP_VERSION to set the JSON response field, so /health obtains the version from the same constant without any runtime configuration parsing. The browser interface obtains the version in each controller's navigation area, where a version span is appended using the same csms::APP_VERSION constant via ostringstream concatenation. The HomeController also references the constant for its version display. Inconsistent version values are avoided because there is exactly one definition of APP_VERSION — every consumer includes the same header and references the same constexpr, so changing the version requires editing only AppInfo.h. This approach follows C++ conventions by using constexpr for compile-time constants in a namespace, which is type-safe and has zero runtime overhead.

## 3. Files I Changed

1. **models/AppInfo.h** (new) — Single source of truth for application name, short name, and version as constexpr constants in the csms namespace. Included by all controllers and the test suite.
2. **controllers/HealthController.cc** — Updated /health JSON response to use csms::APP_NAME and csms::APP_VERSION instead of hardcoded strings, reporting version 1.0.0.
3. **controllers/HomeController.cc** — Updated the home page version display to use csms::APP_VERSION via string concatenation instead of the hardcoded 0.1.0.
4. **controllers/ResidentController.cc** — Added AppInfo.h include and a version span ("v1.0.0") to the navigation area on all pages.
5. **controllers/ServiceRequestController.cc** — Added AppInfo.h include and a version span to the navigation area on all pages.
6. **controllers/DashboardController.cc** — Added AppInfo.h include and a version span to the navigation area.
7. **test/test_main.cc** — Added the Final Examination integration test (testResidentLifecycleAffectsServiceRequestEligibilityAndDashboardCounts) crossing four modules with real persistence, plus version constant assertions.
8. **test/CMakeLists.txt** — Added FinalExamIntegrationTest to set_tests_properties with TIMEOUT 10.

## 4. My Final Examination Test

- **Test Name:** testResidentLifecycleAffectsServiceRequestEligibilityAndDashboardCounts
- **Modules/Responsibilities Covered:** Resident Registration (T04), Service Request Submission (T09), Resident Deactivation (T07), Dashboard Aggregation (T14), Service Request Query (T11)
- **Starting State:** Empty isolated SQLite database via makeTempDbPath()
- **Operations Performed:**
  1. Register an Active Resident and verify success
  2. Verify dashboard shows totalResidents=1, activeResidents=1
  3. Submit a valid Service Request for that Resident and verify status=Pending
  4. Verify dashboard shows totalServiceRequests=1, pending=1, open=1
  5. Deactivate the Resident and verify status becomes Inactive
  6. Verify dashboard shows activeResidents=0, inactiveResidents=1, total unchanged
  7. Attempt another Service Request submission for the Inactive Resident
  8. Verify submission fails with residentInactive=true and no record persisted
  9. Verify the original Service Request remains queryable with correct fields
  10. Assert APP_VERSION == "1.0.0", APP_NAME == "Community Services Management System"
- **Expected Final State:** 1 Resident (Inactive), 1 Service Request (Pending), dashboard counts accurate, rejected submission did not persist, version constants verified
- **Why This Test Is Meaningful:** It crosses five established CSMS modules in a single real-persistence scenario, proving that registration, submission eligibility, deactivation, dashboard aggregation, and query all interoperate correctly. A unit test for any single module would not catch integration failures between them.

## 5. My Final Technical Decision

- **Decision:** Place the release version in models/AppInfo.h as constexpr constants rather than in config.json or hardcoded in each controller.
- **Alternative Considered:** Defining the version in config.json and reading it at runtime, or hardcoding the version string in each controller's HTML output.
- **Reason for My Decision:** A constexpr constant in a header file is resolved at compile time with zero runtime overhead, is type-safe, and requires editing only one file to change the version across the entire application. config.json would require runtime parsing and is already used for framework configuration, not application metadata. Hardcoding in each controller would violate the single-source-of-truth requirement and risk inconsistent values. The constexpr approach is idiomatic C++ and ensures the version cannot drift between /health and the UI.

## 6. Problem or Verification Issue Encountered

- **Problem:** The version span in the navigation needed to appear on every page across three controllers, each of which generates inline HTML via ostringstream. The version constant is a C++ expression (csms::APP_VERSION) that cannot be embedded inside a raw string literal.
- **Cause/Investigation:** Raw string literals (R"(...)") cannot contain C++ expressions — they are pure string text. The nav HTML is built as a raw string, so the version had to be injected by breaking the raw string and using ostringstream concatenation.
- **Resolution:** Appended a separate << expression after each nav raw string: `<< " <span class=\"version\">v" << csms::APP_VERSION << "</span>"`. This keeps the version as a compile-time constant while allowing it to appear in the HTML output. The same pattern was applied to all three controllers using replace_all.

## 7. Final Regression Results
- **Test Command:** `ctest --test-dir build -C Debug --output-on-failure`
- **Final Regression Run 1:** 20/20 passed (100%)
- **Final Regression Run 2:** 20/20 passed (100%)
- **Total Passing Tests Reported:** 20

## 8. Tools and References Used
- Drogon framework documentation (controllers, HttpSimpleController, HttpController)
- C++ standard library documentation (ostringstream, constexpr, optional)
- SQLite3 C API documentation
- Git and GitHub documentation (branching, PR workflow, tagging, releases)
- AI coding assistant (Deepseek) used for implementation assistance throughout the course
- Existing CSMS repository documentation (docs/architecture.md, docs/developer-handbook.md)

## 9. Final Release Declaration

CSMS v1.0.0 - READY FOR RELEASE
