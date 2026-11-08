# Midterm Checkpoint — T10: Manage Service Request Status

---

## Section 1 — Developer Information

| Field | Value |
|---|---|
| **Name** | Lloyd Ilagan |
| **GitHub Username** | McLloydIlagan |
| **Primary Technology Stack** | C++ / Drogon / SQLite |
| **T10 Branch** | feature/t10-service-request-status |

---

## Section 2 — My T10 Implementation

For T10, I created a new `ServiceRequestStatusService` class that lives in the service layer, separate from the repository and any controller. The main method is `manageStatus(requestId, targetStatus)`, which runs through a series of checks before touching the database.

The first thing it does is call `ServiceRequestRepository::findById()` to look up the existing request. If nothing comes back, it stops right there and returns a result with `notFound = true` — nothing gets written. Once I have the record, I check whether the requested target status is actually one of the four values the system supports (`Pending`, `In Progress`, `Completed`, `Cancelled`). If it isn't, the method returns `unsupportedStatus = true` and again leaves persistence untouched.

After that, I read the current status from the retrieved `ServiceRequest` and pass both the current and target status into a private `isAllowedTransition()` helper. That helper is where the actual transition rules live — it rejects same-status requests immediately, then checks whether the move makes sense given the current state. If the transition isn't allowed, the result comes back with `invalidTransition = true` and the database is never modified.

Only when all three checks pass does the service call `ServiceRequestRepository::updateStatus()`, which runs a parameterised `UPDATE service_requests SET status = ? WHERE id = ?` and re-fetches the row with `findById()` to hand back the authoritative updated record. The final result carries `success = true` and the updated `ServiceRequest` so the caller always gets the real persisted state.

---

## Section 3 — My Transition Rules

### Allowed transitions

| Current Status | Allowed Next Statuses |
|---|---|
| Pending | In Progress, Cancelled |
| In Progress | Completed, Cancelled |
| Completed | *(none — terminal)* |
| Cancelled | *(none — terminal)* |

### Why Pending cannot move directly to Completed

A `Pending` request hasn't been picked up yet. Jumping straight to `Completed` would let someone mark a request as done without anyone actually working on it. The intent is that the request has to pass through `In Progress` first — that way there's a clear acknowledgment that processing actually happened before the request gets closed out.

### Why Completed is terminal

Once a request reaches `Completed`, the work is done. There's no meaningful reason to move it anywhere else — reopening it would muddy the processing history. If something needs to be done again, a new Service Request should be submitted.

### Why Cancelled is terminal

`Cancelled` means someone made a deliberate decision to stop. Allowing a cancelled request to come back to life would undercut that decision and make it hard to tell whether a request is actually active or not. Again, a fresh submission is the right path if the resident needs the service.

### How same-status requests are handled

`isAllowedTransition()` checks `currentStatus == targetStatus` as its very first condition and returns `false` immediately if they match. The result comes back as `invalidTransition = true` and nothing in the database changes. This makes sure T10 only records real state changes, not accidental no-op writes.

---

## Section 4 — Files I Changed

| File | Purpose |
|---|---|
| `services/ServiceRequestStatusResult.h` | New result struct for T10 — holds the four outcome flags (`success`, `notFound`, `unsupportedStatus`, `invalidTransition`) and the optional updated `ServiceRequest`. |
| `services/ServiceRequestStatusService.h` | Header for the new service class that owns the status workflow — look up, validate, enforce transition rules, delegate to the repository. |
| `services/ServiceRequestStatusService.cc` | Full implementation of `manageStatus()` and the two private helpers `isSupportedStatus()` and `isAllowedTransition()`. This is where the transition logic actually lives. |
| `repositories/ServiceRequestRepository.h` | Added the `updateStatus(int requestId, const std::string& newStatus)` declaration so the repository has a dedicated method for persisting a status change without touching anything else. |
| `repositories/ServiceRequestRepository.cc` | Implementation of `updateStatus()` — runs the parameterised `UPDATE`, then calls `findById()` to return the updated record directly from persistence. |
| `CMakeLists.txt` | Added `services/ServiceRequestStatusService.cc` to the main executable. |
| `test/CMakeLists.txt` | Added the same `.cc` to the test executable and registered `ServiceRequestStatusTest` with a `TIMEOUT 10` property. |
| `test/test_main.cc` | Added T10 includes, two shared helpers (`submitNewRequest`, `advanceToInProgress`), all 13 required tests, the student-designed test, and the `DROGON_TEST(ServiceRequestStatusTest)` wrapper. |

---

## Section 5 — Problem I Encountered

While writing the tests, `testCompletedIsTerminal` kept failing on the `toInProgress` check — the result was showing `success = true` instead of `invalidTransition = true`, which made no sense since `Completed` should be a terminal state.

I started tracing through the test manually and realised the problem wasn't actually in the transition logic itself. The test was calling `advanceToInProgress()` and then immediately attempting the `Completed` transition, but I hadn't added an assertion in between to confirm the request was actually in `Completed` before testing the outgoing transitions. The `advanceToInProgress` helper sets the status to `In Progress`, and the next `manageStatus(requestId, "Completed")` call was succeeding fine — but then the variable I was checking for the `toInProgress` rejection was actually the result of the `Completed` transition, not the rejection attempt.

Once I added an intermediate `assert(stored->getStatus() == "Completed")` after the first transition, the sequence became clear and all three terminal-state rejection checks passed correctly. It was a test-ordering bug rather than a logic bug in the service itself.

---

## Section 6 — My Student-Designed Test

**Test Name:** `testMultipleSequentialTransitionsFollowCorrectWorkflow`

**What the Test Verifies:**
This test takes a single Service Request all the way through the intended forward path: `Pending` → `In Progress` → `Completed`. After each step it reads the status back from the repository to confirm it actually persisted. Then it tries one more transition (`Completed` → `Cancelled`) to verify the terminal state holds. At the end it checks that all non-status fields — `id`, `serviceType`, `description`, `dateRequested` — are exactly the same as when the request was first submitted.

**Why I Added This Test:**
The required tests each test one hop in isolation on a fresh database. That's useful, but it doesn't catch bugs that only show up when multiple transitions happen to the same row in sequence. For example, `updateStatus()` could theoretically clear a field on the second call, or the `WHERE id = ?` binding could somehow target the wrong row after a prior update. By walking the same request through three consecutive `manageStatus()` calls and checking field preservation at the end, this test covers a real-world scenario that the individual hop tests don't collectively address.

---

## Section 7 — Tools and References Used

| Tool / Reference | How it was used |
|---|---|
| **SQLite documentation** (sqlite.org) | Looked up `sqlite3_prepare_v2`, `sqlite3_bind_*`, `sqlite3_step`, and `sqlite3_last_insert_rowid` while writing `updateStatus()`. |
| **Drogon framework documentation** | Checked `DROGON_TEST` macro usage and the `ParseAndAddDrogonTests` CMake helper when registering the new test suite. |
| **C++ reference** (cppreference.com) | Referenced `std::optional` and `std::string` comparison behaviour while working on the service and validator code. |
| **Visual Studio** | Used for editing files, navigating the project, and reading compiler error output. The Markdown Preview Enhanced extension was also used to preview this checkpoint document. |
| **AI assistant (DeepSeek)** | Used DeepSeek (via browser at deepseek.com) to help with code generation, test scaffolding, and drafting this document. All submitted code was reviewed before committing and I can explain every part of the implementation. |
| **Markdown Preview Extension** | Used Markdown Preview Extension  to help with markdown files design.|