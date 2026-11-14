# CSMS v1.0.0 Release Notes

## Release
- Application: Community Services Management System
- Release: v1.0.0
- Release Type: Final Course Release

## Release Summary

CSMS v1.0.0 is a browser-based community services management system built with C++ and the Drogon framework. It provides full lifecycle management for Residents and Service Requests, backed by SQLite persistence, with a dashboard summarizing the operational state of the system. The application enforces domain validation, controlled workflow transitions, and Resident eligibility rules through a layered architecture that separates presentation, application services, validation, and persistence.

## Resident Management Capabilities

- Register new Residents with validation (first name, last name, address, contact number, email)
- Search Residents by partial, case-insensitive first or last name
- List all Residents with deterministic ordering (lastName ASC, firstName ASC, id ASC)
- Update editable Resident information (preserves ID and status)
- Deactivate Active Residents (soft deactivation — record remains persisted and searchable)
- Inactive Residents remain visible in listing and search
- All operations available through the browser interface at /residents

## Service Request Management Capabilities

- Submit Service Requests for eligible Active Residents
- Validation of service type, description, and date requested
- Resident existence and Active status verified before persistence
- Search by partial, case-insensitive serviceType or description
- Filter by status (Pending, In Progress, Completed, Cancelled)
- Combined text search and status filtering
- View Service Request details by ID
- Controlled status workflow:
  - Pending to In Progress
  - Pending to Cancelled
  - In Progress to Completed
  - In Progress to Cancelled
- Completed and Cancelled are terminal states
- All operations available through the browser interface at /service-requests

## Dashboard Capabilities

- Total, Active, and Inactive Resident counts (SQL COUNT aggregate queries)
- Total, Pending, In Progress, Completed, Cancelled Service Request counts
- Open Service Requests derived metric (Pending + In Progress)
- Five most recent Service Requests with dateRequested DESC, id DESC ordering
- Navigation to Resident Management and Service Request Management
- Read-only — no mutations from dashboard access

## Quality and Hardening

T15 system hardening reviewed all nine required areas:
- Full regression behavior across T01-T14
- Invalid and missing input handling
- Persistence integrity after rejected operations
- HTTP and mutation safety (GET read-only, POST for mutations)
- Query and database safety (parameterized SQL)
- Workflow and lifecycle protection
- Interface and navigation consistency
- Test reliability (isolated temp DBs, no shared state)
- Repository and release hygiene

Five T15 hardening tests were added covering: unsupported status input, invalid transition persistence protection, dashboard read-only safety, cross-feature regression, and repeated deactivation idempotency.

T16 final release added the version identity (1.0.0) from a single source of truth (models/AppInfo.h), updated /health and all UI pages to report it consistently, and added a Final Examination integration test crossing four modules with real persistence.

## Final Verification
- Final Regression Run 1: 20/20 passed (100%)
- Final Regression Run 2: 20/20 passed (100%)
- Application Startup: PASS
- /health: PASS (reports CSMS v1.0.0, status ok)
- Dashboard: PASS
- Resident Management: PASS
- Service Request Management: PASS
- Final Release Version: 1.0.0

## Known Limitations
No known release-blocking limitations remain within the defined CSMS v1.0.0 scope.
