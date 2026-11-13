# T15 Release Readiness — Community Services Management System (CSMS)

## 1. Developer Information
- Name: Lloyd Ilagan
- GitHub Username: McLloydIlagan
- Technology Stack: C++ with Drogon
- T15 Branch: feature/t15-system-hardening

## 2. Release Candidate Baseline
| Component | Status |
| --- | --- |
| Application startup | Working |
| /health endpoint | Working |
| Dashboard | Working |
| Resident Management | Working |
| Service Request Management | Working |
| Starter through T14 automated tests (18/18 passing) | Working |

## 3. Hardening Changes
| Area | Issue or Risk Found | Action Taken | Files Changed | Test/Evidence |
| --- | --- | --- | --- | --- |
| Controllers — ResidentController | malformed IDs, missing records, GET read-only, htmlEscape, nav, error messages | Reviewed - no defect identified. (typed int path params → 404; missing → 404; GET read-only; all output escaped; nav present; friendlyError) | — | Reviewed - no defect identified. |
| Controllers — ServiceRequestController | same, plus invalid status filter | Reviewed - no defect identified. (bad status caught → fallback listing; stoi guarded) | — | Reviewed - no defect identified. |
| Controllers — DashboardController | read-only, no SQL in controller, nav | Reviewed - no defect identified. (delegates to DashboardService; nav present) | — | Reviewed - no defect identified. |
| Controllers — Home / Health | still works | Reviewed - no defect identified. | — | Reviewed - no defect identified. |
| Repositories — ResidentRepository | SQL parameterization, count SQLITE_ROW, error handling | Reviewed - no defect identified. (all bind_*; countAll/countByStatus check SQLITE_ROW; throw on error) | — | Reviewed - no defect identified. |
| Repositories — ServiceRequestRepository | same | Reviewed - no defect identified. | — | Reviewed - no defect identified. |
| Services — Update/Status/Deactivation/Submission/Query/Dashboard | atomicity, transitions, idempotency, eligibility, throw, read-only | Reviewed - no defect identified. (update atomic; manageStatus terminal protection; deactivate idempotent; submit enforces Active; search throws on bad status; dashboard read-only) | — | Reviewed - no defect identified. |
| Models & Validators | validation rules | Reviewed - no defect identified. | — | Reviewed - no defect identified. |
| Tests | isolation, no shared state | Reviewed - no defect identified. (all makeTempDbPath; no hardcoded IDs) | — | Reviewed - no defect identified. |
| Repository Hygiene | debug output; unused code | no debug output found; no unused code | — | git status clean of artifacts; grep for cout/cerr/printf = no matches |
| Hardening Tests | no regression suite for cross-feature/persistence invariants | Added 5 SystemHardeningTest cases + TIMEOUT 10 | test/test_main.cc, test/CMakeLists.txt | ctest 19/19 passing |

## 4. Five New T15 Hardening Tests
| # | Test Name | Category | What It Verifies | Why It Adds Value |
| --- | --- | --- | --- | --- |
| 1 | testUnsupportedServiceRequestStatusSearchReturnsEmptyNotCrash | Invalid/Malformed Input | An unsupported status filter throws std::invalid_argument and leaves the SR count unchanged on a populated DB | Guards data integrity against bad input (existing test only checks the throw on an empty DB) |
| 2 | testInvalidServiceRequestTransitionLeavesRecordUnchanged | Rejected Mutation Protects Persistence | Pending→Completed is rejected (invalidTransition) and every field stays identical | Proves rejected transitions never mutate persistence |
| 3 | testRepeatedDashboardLoadsDoNotModifyPersistence | Read-Only Request Safety | 4 dashboard calls leave repository-level counts identical to call 1, with no new rows | Proves the dashboard read path is side-effect-free at the persistence layer |
| 4 | testResidentDeactivationBlocksFutureServiceRequestSubmissionAndUpdatesDashboard | Cross-Feature Regression | Deactivation blocks later submission (residentInactive, count unchanged) and flips dashboard active/inactive counts | Proves T07 deactivation + T09 eligibility + T14 dashboard interoperate |
| 5 | testRepeatedDeactivationIsSafeAndIdempotent | Repeatability/Boundary | Double deactivation from Active stays idempotent; status stays Inactive; all fields + row count unchanged | Proves repeated deactivation never corrupts data |

## 5. Full Regression Test Results
Test command: `ctest --test-dir c:/Users/LloydIlagan/Desktop/ProgLangLab/csms-cpp-McLloydIlagan/build -C Debug --output-on-failure`

- Run 1: `100% tests passed out of 19` — Total Test time (real) = 12.63 sec — all 19 tests Passed (BasicTest … SystemHardeningTest)
- Run 2: `100% tests passed out of 19` — Total Test time (real) = 10.66 sec — all 19 tests Passed (BasicTest … SystemHardeningTest)
- Total passing tests: 19

## 6. Manual Release Verification
| # | Verification | Result | Notes |
| --- | --- | --- | --- |
| 1 | Application builds and starts | PASS | cmake build exits 0; main.cc listens on 127.0.0.1:8080 |
| 2 | GET /health returns 200 JSON | PASS | { "status":"ok", "application":"Community Services Management System", "version":"0.1.0" } |
| 3 | GET / (home) returns 200 HTML | PASS | Starter home page renders |
| 4 | GET /dashboard returns 200 HTML with counts | PASS | Resident + Service Request metric cards render |
| 5 | GET /residents lists residents | PASS | Table + search + Register button |
| 6 | GET /residents/new shows blank form | PASS | 5 fields |
| 7 | POST /residents creates then redirects (PRG) | PASS | Redirect to /residents |
| 8 | GET /residents/{id}/edit shows prefilled form | PASS | ID + Status read-only |
| 9 | POST /residents/{id} updates then redirects | PASS | Redirect to /residents |
| 10 | POST /residents/{id}/deactivate deactivates | PASS | Redirect to /residents; badge Inactive |
| 11 | GET /residents/abc/edit returns 404 (malformed route) | PASS | Typed int param → no match → 404 |
| 12 | GET /service-requests lists requests | PASS | Table + search + status filter + Submit button |
| 13 | POST /service-requests creates then redirects | PASS | Redirect to /service-requests |
| 14 | GET /service-requests/{id} shows detail + workflow actions | PASS | Status badge + action buttons per status |
| 15 | Status transitions (process/complete/cancel) redirect to detail | PASS | manageStatus enforces transition rules |

## 7. Repository Hygiene Review
| # | Check | Result |
| --- | --- | --- |
| 1 | T14-PR.md removed | PASS |
| 2 | build/ in .gitignore | PASS |
| 3 | *.db / *.sqlite / *.sqlite3 in .gitignore | PASS |
| 4 | No debug output (cout/cerr/printf) in source | PASS |
| 5 | No unused/dead code | PASS |
| 6 | config.json / config.yaml contain no sensitive data (default templates) | PASS |
| 7 | No commented-out logic obscuring behavior (only standard scaffold comments) | PASS |

## 8. Known Limitations
No known release-blocking limitations identified within the defined CSMS scope.

## 9. Release Candidate Statement
READY FOR T16 FINAL EXAM
