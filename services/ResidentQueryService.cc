#include "ResidentQueryService.h"

#include <algorithm>
#include <cctype>

namespace csms {

ResidentQueryService::ResidentQueryService(ResidentRepository& repository)
    : repository_(repository)
{
}

std::vector<Resident> ResidentQueryService::listResidents()
{
    return repository_.findAll();
}

std::vector<Resident> ResidentQueryService::searchResidents(
    const std::string& searchTerm)
{
    const std::string trimmed = trim(searchTerm);

    // A blank search term means "list all Residents".
    if (trimmed.empty())
    {
        return repository_.findAll();
    }

    // Delegate the actual database-level search to the repository.
    // No in-memory filtering is performed here.
    return repository_.searchByName(trimmed);
}

std::string ResidentQueryService::trim(const std::string& value) const
{
    const auto start = std::find_if_not(
        value.begin(),
        value.end(),
        [](unsigned char c) { return std::isspace(c); }
    );

    const auto end = std::find_if_not(
        value.rbegin(),
        value.rend(),
        [](unsigned char c) { return std::isspace(c); }
    ).base();

    return (start < end) ? std::string(start, end) : std::string{};
}

} // namespace csms
